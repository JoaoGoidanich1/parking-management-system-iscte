#!/bin/bash
# SO_HIDE_DEBUG=1                   ## Uncomment this line to hide all @DEBUG statements
# SO_HIDE_COLOURS=1                 ## Uncomment this line to disable all escape colouring
. so_utils.sh                       ## This is required to activate the macros so_success, so_error, and so_debug

#####################################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2024/2025, Enunciado Versão 1
##
## Aluno: Nº:       Nome:
## Nome do Módulo: S3. Script: stats.sh
## Descrição/Explicação do Módulo:
##   Este script gera estatísticas sobre o sistema Park-IUL e produz um relatório HTML.
##
#####################################################################################

## Este script obtém informações sobre o sistema Park-IUL, afixando os resultados das estatísticas pedidas no formato standard HTML no Standard Output e no ficheiro stats.html. Cada invocação deste script apaga e cria de novo o ficheiro stats.html, e poderá resultar em uma ou várias estatísticas a serem produzidas, todas elas deverão ser guardadas no mesmo ficheiro stats.html, pela ordem que foram especificadas pelos argumentos do script.

## S3.1. Validações:
## O script valida se, na diretoria atual, existe algum ficheiro com o nome arquivo-<Ano>-<Mês>.park, gerado pelo Script: manutencao.sh. Se não existirem ou não puderem ser lidos, dá so_error S3.1 <descrição do erro>, indicando o erro em questão, e termina. Caso contrário, dá so_success S3.1.


## S3.2. Estatísticas:
## Cada uma das estatísticas seguintes diz respeito à extração de informação dos ficheiros do sistema Park-IUL. Caso não haja informação suficiente para preencher a estatística, poderá apresentar uma lista vazia.
## S3.2.1.  Obter uma lista das matrículas e dos nomes de todos os condutores cujas viaturas estão ainda estacionadas no parque, ordenados alfabeticamente por nome de condutor:
## <h2>Stats1:</h2>
## <ul>
## <li><b>Matrícula:</b> <Matrícula> <b>Condutor:</b> <Nome do Condutor></li>
## <li><b>Matrícula:</b> <Matrícula> <b>Condutor:</b> <Nome do Condutor></li>
## ...
## </ul>


## S3.2.2. Obter uma lista do top3 das matrículas e do tempo estacionado das viaturas que já terminaram o estacionamento e passaram mais tempo estacionadas, ordenados decrescentemente pelo tempo de estacionamento (considere apenas os estacionamentos cujos tempos já foram calculados):
## <h2>Stats2:</h2>
## <ul>
## <li><b>Matrícula:</b> <Matrícula> <b>Tempo estacionado:</b> <TempoParkMinutos></li>
## <li><b>Matrícula:</b> <Matrícula> <b>Tempo estacionado:</b> <TempoParkMinutos></li>
## <li><b>Matrícula:</b> <Matrícula> <b>Tempo estacionado:</b> <TempoParkMinutos></li>
## </ul>


## S3.2.3. Obter as somas dos tempos de estacionamento das viaturas que não são motociclos, agrupadas pelo nome do país da matrícula (considere apenas os estacionamentos cujos tempos já foram calculados):
## <h2>Stats3:</h2>
## <ul>
## <li><b>País:</b> <Nome País> <b>Total tempo estacionado:</b> <ΣTempoParkMinutos></li>
## <li><b>País:</b> <Nome País> <b>Total tempo estacionado:</b> <ΣTempoParkMinutos></li>
## ...
## </ul>


## S3.2.4. Listar a matrícula, código de país e data de entrada dos 3 estacionamentos, já terminados ou não, que registaram uma entrada mais tarde (hora de entrada) no parque de estacionamento, ordenados crescentemente por hora de entrada:
## <h2>Stats4:</h2>
## <ul>
## <li><b>Matrícula:</b> <Matrícula> <b>País:</b> <Código País> <b>Data Entrada:</b> <DataEntrada></li>
## <li><b>Matrícula:</b> <Matrícula> <b>País:</b> <Código País> <b>Data Entrada:</b> <DataEntrada></li>
## <li><b>Matrícula:</b> <Matrícula> <b>País:</b> <Código País> <b>Data Entrada:</b> <DataEntrada></li>
## </ul>


## S3.2.5. Tendo em consideração que um utilizador poderá ter várias viaturas, determine o tempo total, medido em dias, horas e minutos gasto por cada utilizador da plataforma (ou seja, agrupe os minutos em dias e horas).
## <h2>Stats5:</h2>
## <ul>
## <li><b>Condutor:</b> <NomeCondutor> <b>Tempo  total:</b> <x> dia(s), <y> hora(s) e <z> minuto(s)</li>
## <li><b>Condutor:</b> <NomeCondutor> <b>Tempo  total:</b> <x> dia(s), <y> hora(s) e <z> minuto(s)</li>
## ...
## </ul>


## S3.2.6. Liste as matrículas das viaturas distintas e o tempo total de estacionamento de cada uma, agrupadas pelo nome do país com um totalizador de tempo de estacionamento por grupo, e totalizador de tempo global.
## <h2>Stats6:</h2>
## <ul>
## <li><b>País:</b> <Nome País></li>
## <ul>
## <li><b>Matrícula:</b> <Matrícula> <b> Total tempo estacionado:</b> <ΣTempoParkMinutos></li>
## <li><b>Matrícula:</b> <Matrícula> <b> Total tempo estacionado:</b> <ΣTempoParkMinutos></li>
## ...
## </ul>
## <li><b>País:</b> <Nome País></li>
## <ul>
## <li><b>Matrícula:</b> <Matrícula> <b> Total tempo estacionado:</b> <ΣTempoParkMinutos></li>
## <li><b>Matrícula:</b> <Matrícula> <b> Total tempo estacionado:</b> <ΣTempoParkMinutos></li>
## ...
## </ul>
## ...
## </ul>


## S3.2.7. Obter uma lista do top3 dos nomes mais compridos de condutores cujas viaturas já estiveram estacionadas no parque (ou que ainda estão estacionadas no parque), ordenados decrescentemente pelo tamanho do nome do condutor:
## <h2>Stats7:</h2>
## <ul>
## <li><b> Condutor:</b> <Nome do Condutor mais comprido></li>
## <li><b> Condutor:</b> <Nome do Condutor segundo mais comprido></li>
## <li><b> Condutor:</b> <Nome do Condutor terceiro mais comprido></li>
## </ul>


## S3.3. Processamento do script:
## S3.3.1. O script cria uma página em formato HTML, chamada stats.html, onde lista as várias estatísticas pedidas.
## O ficheiro stats.html tem o seguinte formato:
## <html><head><meta charset="UTF-8"><title>Park-IUL: Estatísticas de estacionamento</title></head>
## <body><h1>Lista atualizada em <Data Atual, formato AAAA-MM-DD> <Hora Atual, formato HH:MM:SS></h1>
## [html da estatística pedida]
## [html da estatística pedida]
## ...
## </body></html>
## Sempre que o script for chamado, deverá:
## • Criar o ficheiro stats.html.
## • Preencher, neste ficheiro, o cabeçalho, com as duas linhas HTML descritas acima, substituindo os campos pelos valores de data e hora pelos do sistema.
## • Ciclicamente, preencher cada uma das estatísticas pedidas, pela ordem pedida, com o HTML correspondente ao indicado na secção S3.2.
## • No final de todas as estatísticas preenchidas, terminar o ficheiro com a última linha “</body></html>”

#!/bin/bash
# SO_HIDE_DEBUG=1                   ## Uncomment this line to hide all @DEBUG statements
# SO_HIDE_COLOURS=1                 ## Uncomment this line to disable all escape colouring
. so_utils.sh                       ## Load utility functions for success/error/debug messages

#!/bin/bash


# Definição dos arquivos de entrada e saída
FICHEIRO_ESTACIONAMENTOS="estacionamentos.txt"
ARQUIVOS_PARK="arquivo-*.park"
FICHEIRO_STATS="stats.html"
FORMATO_DATA="%Y-%m-%d"
FORMATO_HORA="%H:%M:%S"

## ------------------- S3.1: Validação ------------------- ##
## - Verifica se os arquivos de histórico existem e podem ser lidos
## - Se houver erro, exibe so_error S3.1 e encerra o script

# Verifica se os argumentos passados são válidos
for arg in "$@"; do
    if [[ ! "$arg" =~ ^[1-7]$ ]]; then
        so_error S3.1 "Argumento inválido: $arg"
        exit 1
    fi
done

# Verifica se o arquivo de estacionamento e países existem e podem ser lidos
if [[ ! -f "$FICHEIRO_ESTACIONAMENTOS" || ! -r "$FICHEIRO_ESTACIONAMENTOS" ]]; then
    so_error S3.1 "Ficheiro estacionamentos.txt não encontrado ou sem permissões de leitura."
    exit 1
fi

if [[ ! -f "paises.txt" || ! -r "paises.txt" ]]; then
    so_error S3.1 "Ficheiro paises.txt não encontrado ou sem permissões de leitura."
    exit 1
fi

# Verifica se há arquivos de histórico gerados pelo script de manutenção
if ! ls $ARQUIVOS_PARK 2>/dev/null | grep -q .; then
    so_error S3.1 "Nenhum arquivo de histórico encontrado."
    exit 1
fi

for file in $ARQUIVOS_PARK; do
    if [[ ! -r "$file" ]]; then
        so_error S3.1 "Um dos ficheiros arquivo-<ano>-<mes>.park não pode ser lido."
        exit 1
    fi
done

so_success S3.1 "Arquivos de histórico encontrados."

## ------------------- S3.3: Processamento e Geração do Relatório ------------------- ##
## - Cria o ficheiro HTML stats.html e preenche com as estatísticas solicitadas
## - Executa as estatísticas em ordem conforme os argumentos fornecidos

# Criação do ficheiro stats.html com cabeçalho
echo "<html><head><meta charset=\"UTF-8\"><title>Park-IUL: Estatísticas de estacionamento</title></head>" > $FICHEIRO_STATS
echo "<body><h1>Lista atualizada em $(date +"$FORMATO_DATA $FORMATO_HORA")</h1>" >> $FICHEIRO_STATS

## ------------------- S3.2: Estatísticas ------------------- ##

# Estatística 1: Veículos atualmente estacionados, ordenados por nome do condutor
if [[ "$@" == "" || "$@" =~ "1" ]]; then
    echo "<h2>Stats1:</h2><ul>" >> $FICHEIRO_STATS
    awk -F":" '{if(NF==5) print $4 ":" $1}' $FICHEIRO_ESTACIONAMENTOS | sort | \
    awk -F":" '{print "<li><b>Matrícula:</b> " $2 " <b>Condutor:</b> " $1 "</li>"}' >> $FICHEIRO_STATS
    echo "</ul>" >> $FICHEIRO_STATS
fi

# Estatística 2: Top 3 veículos que passaram mais tempo estacionados
if [[ "$@" == "" || "$@" =~ "2" ]]; then
    echo "<h2>Stats2:</h2><ul>" >> $FICHEIRO_STATS
    awk -F":" '{tempo[$1]+=$7} END {for (mat in tempo) print mat, tempo[mat]}' $ARQUIVOS_PARK | \
    sort -k2,2nr | head -3 | \
    awk '{print "<li><b>Matrícula:</b> " $1 " <b>Tempo estacionado:</b> " $2 "</li>"}' >> $FICHEIRO_STATS
    echo "</ul>" >> $FICHEIRO_STATS
fi

# Estatística 3: Tempo total de estacionamento por país (exceto motociclos)
if [[ "$@" == "" || "$@" =~ "3" ]]; then
    echo "<h2>Stats3:</h2><ul>" >> $FICHEIRO_STATS
    awk -F":" '{if($3!="M") tempo[$2]+=$7} END {for(p in tempo) printf "<li><b>País:</b> %s <b>Total tempo estacionado:</b> %d</li>\n", p, tempo[p]}' $ARQUIVOS_PARK | \
    sort >> $FICHEIRO_STATS
    echo "</ul>" >> $FICHEIRO_STATS
fi

# Estatística 4: Últimos 3 veículos a entrarem no parque
if [[ "$@" == "" || "$@" =~ "4" ]]; then
    echo "<h2>Stats4:</h2><ul>" >> $FICHEIRO_STATS
    awk -F":" '$5 != "" {print $5, $1, $2}' $ARQUIVOS_PARK | sort -r | awk '!seen[$2]++' | head -3 | \
    awk '{printf "<li><b>Matrícula:</b> %s <b>País:</b> %s <b>Data Entrada:</b> %s</li>\n", $2, $3, $1}' >> $FICHEIRO_STATS
    echo "</ul>" >> $FICHEIRO_STATS
fi

# Estatística 5: Tempo total por utilizador (dias, horas, minutos)
if [[ "$@" == "" || "$@" =~ "5" ]]; then
    echo "<h2>Stats5:</h2><ul>" >> $FICHEIRO_STATS
    awk -F":" '{tempo[$4]+=$7} END {
        for (c in tempo) {
            dias = int(tempo[c] / 1440)
            horas = int((tempo[c] % 1440) / 60)
            minutos = tempo[c] % 60
            printf "<li><b>Condutor:</b> %s <b>Tempo total:</b> %d dia(s), %d hora(s) e %d minuto(s)</li>\n", c, dias, horas, minutos
        }
    }' $ARQUIVOS_PARK | sort -k3,3nr >> $FICHEIRO_STATS
    echo "</ul>" >> $FICHEIRO_STATS
fi

# Estatística 7: Top 3 nomes mais compridos de condutores
if [[ "$@" == "" || "$@" =~ "7" ]]; then
    echo "<h2>Stats7:</h2><ul>" >> $FICHEIRO_STATS
    awk -F":" '{print length($4), $4}' $ARQUIVOS_PARK | sort -nr | awk '{for(i=2; i<=NF; i++) printf $i (i==NF?"":" "); print ""}' | \
    head -3 | awk '{print "<li><b> Condutor:</b> " $0 "</li>"}' >> $FICHEIRO_STATS
    echo "</ul>" >> $FICHEIRO_STATS
fi

# Fechamento do ficheiro HTML
echo "</body></html>" >> $FICHEIRO_STATS

so_success S3.3 "Ficheiro stats.html criado com sucesso."
