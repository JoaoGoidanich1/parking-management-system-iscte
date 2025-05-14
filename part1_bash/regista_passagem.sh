#!/bin/bash
# SO_HIDE_DEBUG=1                   ## Uncomment this line to hide all @DEBUG statements
# SO_HIDE_COLOURS=1                 ## Uncomment this line to disable all escape colouring
. so_utils.sh                       ## This is required to activate the macros so_success, so_error, and so_debug

#####################################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2024/2025, Enunciado Versão 1
##
## Aluno: Nº:       Nome:
## Nome do Módulo: S1. Script: regista_passagem.sh
## Descrição/Explicação do Módulo:
##
##
#####################################################################################

## Este script é invocado quando uma viatura entra/sai do estacionamento Park-IUL. Este script recebe todos os dados por argumento, na chamada da linha de comandos, incluindo os <Matrícula:string>, <Código País:string>, <Categoria:char> e <Nome do Condutor:string>.

## S1.1. Valida os argumentos passados e os seus formatos:
## • Valida se os argumentos passados são em número suficiente (para os dois casos exemplificados), assim como se a formatação de cada argumento corresponde à especificação indicada. O argumento <Categoria> pode ter valores: L (correspondente a Ligeiros), P (correspondente a Pesados) ou M (correspondente a Motociclos);
## • A partir da indicação do argumento <Código País>, valida se o argumento <Matrícula> passada cumpre a especificação da correspondente <Regra Validação Matrícula>;
## • Valida se o argumento <Nome do Condutor> é o “primeiro + último” nomes de um utilizador atual do Tigre;
## • Em caso de qualquer erro das condições anteriores, dá so_error S1.1 <descrição do erro>, indicando o erro em questão, e termina. Caso contrário, dá so_success S1.1.


## S1.2. Valida os dados passados por argumento para o script com o estado da base de dados de estacionamentos especificada no ficheiro estacionamentos.txt:
## • Valida se, no caso de a invocação do script corresponder a uma entrada no parque de estacionamento, se ainda não existe nenhum registo desta viatura na base de dados;
## • Valida se, no caso de a invocação do script corresponder a uma saída do parque de estacionamento, se existe um registo desta viatura na base de dados;
## • Em caso de qualquer erro das condições anteriores, dá so_error S1.2 <descrição do erro>, indicando o erro em questão, e termina. Caso contrário, dá so_success S1.2.


## S1.3. Atualiza a base de dados de estacionamentos especificada no ficheiro estacionamentos.txt:
## • Remova do argumento <Matrícula> passado todos os separadores (todos os caracteres que não sejam letras ou números) eventualmente especificados;
## • Especifique como data registada (de entrada ou de saída, conforme o caso) a data e hora do sistema Tigre;
## • No caso de um registo de entrada, crie um novo registo desta viatura na base de dados;
## • No caso de um registo de saída, atualize o registo desta viatura na base de dados, registando a data de saída;
## • Em caso de qualquer erro das condições anteriores, dá so_error S1.3 <descrição do erro>, indicando o erro em questão, e termina. Caso contrário, dá so_success S1.3.


## S1.4. Lista todos os estacionamentos registados, mas ordenados por saldo:
## • O script deve criar um ficheiro chamado estacionamentos-ordenados-hora.txt igual ao que está no ficheiro estacionamentos.txt, com a mesma formatação, mas com os registos ordenados por ordem crescente da hora (e não da data) de entrada das viaturas.
## • Em caso de qualquer erro das condições anteriores, dá so_error S1.4 <descrição do erro>, indicando o erro em questão, e termina. Caso contrário, dá so_success S1.4.



#!/bin/bash
#!/bin/bash

# Definir diretório base do script
#!/bin/bash
# SO_HIDE_DEBUG=1                   ## Uncomment this line to hide all @DEBUG statements
# SO_HIDE_COLOURS=1                 ## Uncomment this line to disable all escape colouring
. so_utils.sh                       ## This is required to activate the macros so_success, so_error, and so_debug

#####################################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2024/2025, Enunciado Versão 1
##
## Aluno: Nº:       Nome:
## Nome do Módulo: S1. Script: regista_passagem.sh
## Descrição/Explicação do Módulo:
##     Este script gerencia a entrada e saída de veículos no estacionamento do Park-IUL.
##
#####################################################################################

## ------------------- S1.1: Validação dos Argumentos ------------------- ##
## - Garante que o número de argumentos passados é correto
## - Valida a formatação da matrícula com base no código do país
## - Verifica se a categoria do veículo é válida (L, P ou M)
## - Confirma que o nome do condutor está no formato "Primeiro Último"
## - Se houver erro, exibe so_error S1.1 e encerra o script

[[ -f "paises.txt" ]] || { so_error S1.1 "Arquivo paises.txt não encontrado"; exit 1; }

# Definição das variáveis de acordo com os argumentos recebidos
case $# in
    4)  # Entrada de veículo no estacionamento
        MATRICULA_RAW="$1"
        CODIGO_PAIS="$2"
        CATEGORIA="$3"
        NOME_CONDUTOR="$4"
        OPERACAO="ENTRADA"
        ;;
    1)  # Saída de veículo do estacionamento
        IFS='/' read -r CODIGO_PAIS MATRICULA_RAW <<< "$1"
        OPERACAO="SAIDA"
        ;;
    *)  # Número de argumentos inválido
        so_error S1.1 "Número inválido de argumentos"
        exit 1
        ;;
esac

# Verifica se a categoria é válida (apenas para entrada)
if [[ "$OPERACAO" == "ENTRADA" && ! "$CATEGORIA" =~ ^(L|P|M)$ ]]; then
    so_error S1.1 "Categoria incorreta"
    exit 1
fi

# Obtém o formato correto da matrícula com base no país
FORMATO_MATRICULA=$(awk -F '###' -v cod="$CODIGO_PAIS" '$1 == cod {print $3}' paises.txt)
[[ -n "$FORMATO_MATRICULA" ]] || { so_error S1.1 "Código do país inválido"; exit 1; }

# Valida a matrícula com o formato esperado
[[ "$MATRICULA_RAW" =~ $FORMATO_MATRICULA ]] || { so_error S1.1 "Formato inválido da matrícula"; exit 1; }

# Verifica se o nome do condutor está no formato "Primeiro Último" (apenas para entrada)
if [[ "$OPERACAO" == "ENTRADA" && ! "$NOME_CONDUTOR" =~ ^[A-Za-z]+\ [A-Za-z]+$ ]]; then
    so_error S1.1 "Nome inválido. Deve estar no formato 'Primeiro Último'."
    exit 1
fi

# Confirma se o condutor é um usuário válido do sistema Tigre
if [[ "$OPERACAO" == "ENTRADA" ]]; then
    PRIMEIRO_NOME=$(echo "$NOME_CONDUTOR" | awk '{print $1}')
    ULTIMO_NOME=$(echo "$NOME_CONDUTOR" | awk '{print $NF}')
    getent passwd | awk -F: '{print $5}' | grep -iE "$PRIMEIRO_NOME.*$ULTIMO_NOME" > /dev/null || {
        so_error S1.1 "Usuário não encontrado no servidor Tigre"; exit 1;
    }
fi

so_success S1.1 "Dados validados com sucesso."

## ------------------- S1.2: Validação no Arquivo de Estacionamento ------------------- ##
## - Verifica se a matrícula já está registrada (entrada duplicada não permitida)
## - Confirma que há registro para saída (não permite saída sem entrada)
## - Se houver erro, exibe so_error S1.2 e encerra o script

MATRICULA=$(echo "$MATRICULA_RAW" | tr -d ' -')  # Remove caracteres especiais da matrícula
[[ -f "estacionamentos.txt" ]] && REGISTRO_ANTERIOR=$(grep "^$MATRICULA:" estacionamentos.txt | tail -n 1) || REGISTRO_ANTERIOR=""

if [[ "$OPERACAO" == "ENTRADA" && -n "$REGISTRO_ANTERIOR" && $(echo "$REGISTRO_ANTERIOR" | awk -F':' '{print NF}') -eq 5 ]]; then
    so_error S1.2 "Veículo já estacionado"
    exit 1
elif [[ "$OPERACAO" == "SAIDA" ]]; then
    [[ -z "$REGISTRO_ANTERIOR" ]] && { so_error S1.2 "Veículo não encontrado no estacionamento"; exit 1; }
    [[ $(echo "$REGISTRO_ANTERIOR" | awk -F':' '{print NF}') -eq 6 ]] && { so_error S1.2 "Veículo já saiu"; exit 1; }
fi

so_success S1.2 "Verificação no banco de dados concluída."

## ------------------- S1.3: Atualização do Banco de Dados ------------------- ##
## - Adiciona uma nova entrada ou atualiza um registro de saída
## - Se houver erro, exibe so_error S1.3 e encerra o script

if [[ ! -f "estacionamentos.txt" ]]; then
    if ! touch "estacionamentos.txt"; then
        so_error S1.3 "Falha ao criar estacionamentos.txt"
        exit 1
    fi
fi
[[ -w "estacionamentos.txt" ]] || { so_error S1.3 "Arquivo sem permissão de escrita"; exit 1; }

DATA_REGISTRO=$(date "+%Y-%m-%dT%Hh%M")  # Obtém a data e hora atual

if [[ "$OPERACAO" == "ENTRADA" ]]; then
    # Registra uma nova entrada
    printf "%s:%s:%s:%s:%s\n" "$MATRICULA" "$CODIGO_PAIS" "$CATEGORIA" "$NOME_CONDUTOR" "$DATA_REGISTRO" >> estacionamentos.txt

elif [[ "$OPERACAO" == "SAIDA" ]]; then
    # Atualiza o registro com a data de saída
    awk -v mat="$MATRICULA" -v data="$DATA_REGISTRO" -F':' 'BEGIN {OFS=FS} {if ($1 == mat && NF == 5) { $6 = data } print}' estacionamentos.txt > temp.txt && mv temp.txt estacionamentos.txt
fi

so_success S1.3 "Registro atualizado com sucesso."

## ------------------- S1.4: Ordenação por Hora de Entrada ------------------- ##
## - Cria um novo arquivo "estacionamentos-ordenados-hora.txt"
## - Ordena os registros pelo horário de entrada
## - Se houver erro, exibe so_error S1.4 e encerra o script

sort -t':' -k5.12,5.16 estacionamentos.txt > estacionamentos-ordenados-hora.txt

so_success S1.4 "Arquivo ordenado gerado com sucesso."
