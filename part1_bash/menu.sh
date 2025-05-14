#!/bin/bash
# SO_HIDE_DEBUG=1                   ## Uncomment this line to hide all @DEBUG statements
# SO_HIDE_COLOURS=1                 ## Uncomment this line to disable all escape colouring
. so_utils.sh                       ## This is required to activate the macros so_success, so_error, and so_debug

#####################################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2024/2025, Enunciado Versão 1
##
## Aluno: Nº:       Nome:
## Nome do Módulo: S4. Script: menu.sh
## Descrição/Explicação do Módulo:
##    Este script centraliza a execução dos outros scripts, permitindo ao usuário 
##    escolher interativamente as operações desejadas.
##
#####################################################################################

## Este script invoca os scripts restantes, não recebendo argumentos.
## Atenção: Não é suposto que volte a fazer nenhuma das funcionalidades dos scripts anteriores. O propósito aqui é simplesmente termos uma forma centralizada de invocar os restantes scripts.
## S4.1. Apresentação:
## S4.1.1. O script apresenta (pode usar echo, cat ou outro, sem “limpar” o ecrã) um menu com as opções abaixo indicadas.


## S4.2. Validações:
## S4.2.1. Aceita como input do utilizador um número. Valida que a opção introduzida corresponde a uma opção válida. Se não for, dá so_error <opção> (com a opção errada escolhida), e volta ao passo S4.1 (ou seja, mostra novamente o menu). Caso contrário, dá so_success <opção>.
## S4.2.2. Analisa a opção escolhida, e mediante cada uma delas, deverá invocar o sub-script correspondente descrito nos pontos S1 a S3 acima. No caso das opções 1 e 4, este script deverá pedir interactivamente ao utilizador as informações necessárias para execução do sub-script correspondente, injetando as mesmas como argumentos desse sub-script:
## S4.2.2.1. Assim sendo, no caso da opção 1, o script deverá pedir ao utilizador sucessivamente e interactivamente os dados a inserir:


## Este script não deverá fazer qualquer validação dos dados inseridos, já que essa validação é feita no script S1. Após receber os dados, este script invoca o Sub-Script: regista_passagem.sh com os argumentos recolhidos do utilizador. Após a execução do sub-script, dá so_success e volta ao passo S4.1.
## S4.2.2.2. No caso da opção 2, o script deverá pedir ao utilizador sucessivamente e interactivamente os dados a inserir:
##  Este script não deverá fazer qualquer validação dos dados inseridos, já que essa validação é feita no script S1. Após receber os dados, este script invoca o Sub-Script: regista_passagem.sh com os argumentos recolhidos do utilizador. Após a execução do sub-script, dá so_success e volta ao passo S4.1.
## S4.2.2.3. No caso da opção 3, o script invoca o Sub-Script: manutencao.sh. Após a execução do sub-script, dá so_success e volta para o passo S4.1.
## S4.2.2.4. No caso da opção 4, o script deverá pedir ao utilizador as opções de estatísticas a pedir, antes de invocar o Sub-Script: stats.sh. Se uma das opções escolhidas for a 8, o menu deverá invocar o Sub-Script: stats.sh sem argumentos, para que possa executar TODAS as estatísticas, caso contrário deve respeitar a ordem.
## Após a execução do Sub-Script: stats.sh, dá so_success e volta para o passo S4.1.


## Apenas a opção 0 (zero) permite sair deste Script: menu.sh. Até escolher esta opção, o menu deverá ficar em ciclo, permitindo realizar múltiplas operações iterativamente (e não recursivamente, ou seja, não deverá chamar o Script: menu.sh novamente). 

#!/bin/bash

## ------------------- S4.1: Apresentação ------------------- ##
## - Exibe o menu de opções para o usuário
## - Não limpa a tela, apenas imprime as opções disponíveis

# Localiza o arquivo de utilitários "so_utils.sh"
SO_UTILS=$(find /home -type f -name "so_utils.sh" 2>/dev/null | head -n 1)
if [[ -z "$SO_UTILS" ]]; then
    echo "@ERROR {S4.0} [Arquivo so_utils.sh não encontrado]" >&2
    exit 1
fi
source "$SO_UTILS"

## ------------------- S4.2: Validações ------------------- ##
## - Verifica se os scripts necessários estão presentes e executáveis antes de chamá-los
## - Caso um script esteja ausente ou sem permissões, retorna um erro

check_script() {
    if [[ ! -f "$1" || ! -x "$1" ]]; then
        so_error "S4.0" "Script $1 não encontrado ou sem permissão de execução"
        return 1
    fi
}

## ------------------- S4.3: Menu de Navegação ------------------- ##
## - Permanece em loop até que o usuário escolha a opção de sair (0)
## - Chama os scripts apropriados com os argumentos necessários
## - Retorna mensagens de erro e sucesso conforme apropriado

while true; do
    echo -e "\nMENU:"
    echo "1: Regista passagem – Entrada estacionamento"
    echo "2: Regista passagem – Saída estacionamento"
    echo "3: Manutenção"
    echo "4: Estatísticas"
    echo "0: Sair"
    echo -n "Opção: "
    read opcao

    case "$opcao" in
        ("1")  # Registrar entrada de veículo no estacionamento
            so_success "S4.2.1" "$opcao"
            check_script "./regista_passagem.sh" || continue
            
            # Solicita dados ao usuário
            echo -n "Indique a matrícula da viatura: "
            read matricula
            matricula=$(echo "$matricula" | tr -s ' ')
            
            echo -n "Indique o código do país de origem da viatura: "
            read pais
            
            echo -n "Indique a categoria da viatura [L(igeiro)|P(esado)|M(otociclo)]: "
            read categoria
            if [[ ! "$categoria" =~ ^[LPM]$ ]]; then
                so_error "S4.2.1" "$opcao"
                continue
            fi
            
            echo -n "Indique o nome do condutor da viatura: "
            read condutor
            
            # Chama o script de registo de passagem
            ./regista_passagem.sh "$matricula" "$pais" "$categoria" "$condutor"
            so_success "S4.3"
            ;;
            
        ("2")  # Registrar saída de veículo do estacionamento
            so_success "S4.2.1" "$opcao"
            check_script "./regista_passagem.sh" || continue
            
            # Solicita dados ao usuário
            echo -n "Indique a matrícula da viatura: "
            read matricula
            matricula=$(echo "$matricula" | tr -s ' ')
            
            echo -n "Indique o código do país de origem da viatura: "
            read pais
            
            # Chama o script de registo de saída
            ./regista_passagem.sh "$pais/${matricula// / }"
            so_success "S4.4"
            ;;
            
        ("3")  # Executar manutenção do sistema
            so_success "S4.2.1" "$opcao"
            check_script "./manutencao.sh" || continue
            
            # Chama o script de manutenção
            ./manutencao.sh
            so_success "S4.5"
            ;;
            
        ("4")  # Executar estatísticas do sistema
            so_success "S4.2.1" "$opcao"
            check_script "./stats.sh" || continue
            
            echo -n "Indique quais as estatísticas a incluir: "
            read stats
            
            # Verifica se o usuário escolheu alguma estatística
            if [[ -z "$stats" ]]; then
                so_error "S4.6"
                continue
            fi
            
            # Se a opção 8 for escolhida, executa todas as estatísticas
            if [[ " $stats " =~ " 8 " ]]; then
                ./stats.sh
            else
                ./stats.sh $stats
            fi
            
            so_success "S4.6"
            ;;
            
        ("0")  # Sair do menu
            so_success "S4.2.1" "$opcao"
            echo "Saindo..."
            exit 0
            ;;
            
        (*)  # Opção inválida
            so_error "S4.2.1" "$opcao"
            ;;
    esac
done
