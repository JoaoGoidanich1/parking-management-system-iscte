#!/bin/bash
# SO_HIDE_DEBUG=1                   ## Uncomment this line to hide all @DEBUG statements
# SO_HIDE_COLOURS=1                 ## Uncomment this line to disable all escape colouring
. so_utils.sh                       ## This is required to activate the macros so_success, so_error, and so_debug

#####################################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2024/2025, Enunciado Versão 1
##
## Aluno: Nº:       Nome:
## Nome do Módulo: S2. Script: manutencao.sh
## Descrição/Explicação do Módulo:
##    Este script realiza a manutenção do estacionamento, validando registros,
##    arquivando veículos que já saíram e atualizando a base de dados.
##
#####################################################################################

## Este script não recebe nenhum argumento, e permite realizar a manutenção dos registos de estacionamento. 

## S2.1. Validações do script:
## O script valida se, no ficheiro estacionamentos.txt:
## • Todos os registos referem códigos de países existentes no ficheiro paises.txt;
## • Todas as matrículas registadas correspondem à especificação de formato dos países correspondentes;
## • Todos os registos têm uma data de saída superior à data de entrada;
## • Em caso de qualquer erro das condições anteriores, dá so_error S2.1 <descrição do erro>, indicando o erro em questão, e termina. Caso contrário, dá so_success S2.1.


## S2.2. Processamento:
## • O script move, do ficheiro estacionamentos.txt, todos os registos que estejam completos (com registo de entrada e registo de saída), mantendo o formato do ficheiro original, para ficheiros separados com o nome arquivo-<Ano>-<Mês>.park, com todos os registos agrupados pelo ano e mês indicados pelo nome do ficheiro. Ou seja, os registos são removidos do ficheiro estacionamentos.txt e acrescentados ao correspondente ficheiro arquivo-<Ano>-<Mês>.park, sendo que o ano e mês em questão são os do campo <DataSaída>. 
## • Quando acrescentar o registo ao ficheiro arquivo-<Ano>-<Mês>.park, este script acrescenta um campo <TempoParkMinutos> no final do registo, que corresponde ao tempo, em minutos, que durou esse registo de estacionamento (correspondente à diferença em minutos entre os dois campos anteriores).
## • Em caso de qualquer erro das condições anteriores, dá so_error S2.2 <descrição do erro>, indicando o erro em questão, e termina. Caso contrário, dá so_success S2.2.
## • O registo em cada ficheiro arquivo-<Ano>-<Mês>.park, tem então o formato:
## <Matrícula:string>:<Código País:string>:<Categoria:char>:<Nome do Condutor:string>: <DataEntrada:AAAA-MM-DDTHHhmm>:<DataSaída:AAAA-MM-DDTHHhmm>:<TempoParkMinutos:int>
## • Exemplo de um ficheiro arquivo-<Ano>-<Mês>.park, para janeiro de 2025:

#!/bin/bash

# Carregar funções auxiliares
source so_utils.sh

# Ficheiros de referência
FICHEIRO_PAISES="paises.txt"
FICHEIRO_ESTACIONAMENTOS="estacionamentos.txt"
FORMATO_DATA_HORA="+%Y-%m-%dT%H:%M"

# Verificar existência dos arquivos
if [[ ! -f "$FICHEIRO_PAISES" ]]; then
    so_error S2.1 "Arquivo paises.txt não encontrado."
    exit 1
fi

if [[ ! -f "$FICHEIRO_ESTACIONAMENTOS" ]]; then
    so_success S2.1 "Arquivo estacionamentos.txt não encontrado."
    so_success S2.2 "Nenhum registro para processar."
    exit 0  
fi


# Validação dos registos no ficheiro estacionamentos.txt
validar_registos() {
    local erro=0
    while IFS= read -r linha; do
        num_campos=$(awk -F':' '{print NF}' <<< "$linha")
        if [[ $num_campos -lt 5 || $num_campos -gt 6 ]]; then
            so_error S2.2 "Formato inválido no ficheiro estacionamentos.txt: $linha"
            erro=1
            continue
        fi
        
        if [[ $num_campos -eq 5 ]]; then
            IFS=: read -r matricula codigo_pais categoria nome data_entrada <<< "$linha"
            data_saida=""
        else
            IFS=: read -r matricula codigo_pais categoria nome data_entrada data_saida <<< "$linha"
        fi
        
        # Verificar se o código do país existe em paises.txt
        if ! grep -q "^$codigo_pais###" "$FICHEIRO_PAISES"; then
            so_error S2.1 "Código de país inválido: $codigo_pais"
            erro=1
        fi

        regex=$(awk -F'###' -v pais="$codigo_pais" '$1 == pais {print $3}' "$FICHEIRO_PAISES")
        if [[ -n "$regex" && ! "$matricula" =~ $regex ]]; then
            so_error S2.1 "Matrícula inválida para o país $codigo_pais: $matricula"
            erro=1
        fi

        if [[ -n "$data_saida" ]]; then
            data_entrada_formatada=$(echo "$data_entrada" | sed -E 's/([0-9]{2})h([0-9]{2})/\1:\2/' | sed 's/T/ /')
            data_saida_formatada=$(echo "$data_saida" | sed -E 's/([0-9]{2})h([0-9]{2})/\1:\2/' | sed 's/T/ /')
            
            data_entrada_timestamp=$(date -d "$data_entrada_formatada" +%s 2>/dev/null)
            data_saida_timestamp=$(date -d "$data_saida_formatada" +%s 2>/dev/null)
            
            if [[ -n "$data_saida_timestamp" && $data_saida_timestamp -le $data_entrada_timestamp ]]; then
           so_error S2.1 "Data de saída menor ou igual à data de entrada para $matricula"
            exit 1 
            fi

        fi
    done < "$FICHEIRO_ESTACIONAMENTOS"
    
    if [[ $erro -eq 1 ]]; then
        exit 1
    else
        so_success S2.1 "Validação concluída com sucesso."
    fi
}

# Executar validação antes de qualquer outro processo
validar_registos || exit 1


# Processar registros completos e movê-los para arquivos de arquivo
processar_registos() {
    local erro=0
    > temp.txt  # Criar ficheiro temporário para armazenar registros sem saída

    # Verificar se temos permissões de escrita antes de processar os registros
    if [[ ! -w "." ]]; then
        so_error S2.2 "Diretoria sem permissões para escrita."
        exit 1
    fi

    while IFS= read -r linha; do
        if [[ "$linha" =~ ^([^:]+):([^:]+):([^:]+):([^:]+):([^:]+)(:([^:]+))?$ ]]; then
            matricula="${BASH_REMATCH[1]}"
            codigo_pais="${BASH_REMATCH[2]}"
            categoria="${BASH_REMATCH[3]}"
            nome="${BASH_REMATCH[4]}"
            data_entrada="${BASH_REMATCH[5]}"
            data_saida="${BASH_REMATCH[7]}"
        else
            so_error S2.2 "Formato inválido no ficheiro estacionamentos.txt: $linha"
            erro=1
            continue
        fi
        
        if [[ -n "$data_saida" ]]; then
            ano_mes=$(echo "$data_saida" | cut -d'T' -f1 | cut -d'-' -f1,2)
            arquivo="arquivo-${ano_mes}.park"
            
            data_entrada_formatada=$(echo "$data_entrada" | sed -E 's/([0-9]{2})h([0-9]{2})/\1:\2/' | sed 's/T/ /')
            data_saida_formatada=$(echo "$data_saida" | sed -E 's/([0-9]{2})h([0-9]{2})/\1:\2/' | sed 's/T/ /')
            
            data_entrada_timestamp=$(date -d "$data_entrada_formatada" +%s 2>/dev/null)
            data_saida_timestamp=$(date -d "$data_saida_formatada" +%s 2>/dev/null)
            tempo_minutos=$(( (data_saida_timestamp - data_entrada_timestamp) / 60 ))

            # Tentar escrever no arquivo e verificar se falhou
            echo "$matricula:$codigo_pais:$categoria:$nome:$data_entrada:$data_saida:$tempo_minutos" >> "$arquivo"
            if [[ $? -ne 0 ]]; then
                so_error S2.2 "Falha ao escrever no arquivo $arquivo devido a permissões."
                exit 1
            fi
        else
            echo "$linha" >> temp.txt
        fi
    done < "$FICHEIRO_ESTACIONAMENTOS"

    mv temp.txt "$FICHEIRO_ESTACIONAMENTOS"
    so_success S2.2 "Registos arquivados com sucesso."
    exit 1
}


processar_registos


# Criar ou sobrescrever o ficheiro cron.def com os horários exigidos
cat <<EOL > cron.def
# Executa manutencao.sh às 05:59 e 13:01 de segunda a sexta-feira
59 5 * * 1-5 cd /home/a130209/trabalho/parte-1; /bin/bash /home/a130209/trabalho/parte-1/manutencao.sh >> /home/a130209/trabalho/parte-1/saida.log 2>&1
1 13 * * 1-5 cd /home/a130209/trabalho/parte-1; /bin/bash /home/a130209/trabalho/parte-1/manutencao.sh >> /home/a130209/trabalho/parte-1/saida.log 2>&1

EOL

so_success S2.3 "Arquivo cron.def atualizado corretamente."



# Ordenar estacionamentos por hora de entrada apenas após todos os processos
sort -t':' -k5 "$FICHEIRO_ESTACIONAMENTOS" > estacionamentos-ordenados-hora.txt
so_success S2.4 "Ficheiro estacionamentos-ordenados-hora.txt criado com sucesso."
