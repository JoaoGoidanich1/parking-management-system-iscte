/****************************************************************************************
 ** ISCTE-IUL: Trabalho prático 2 de Sistemas Operativos 2024/2025, Enunciado Versão 4+
 **
 ** Aluno: Nº:  130209     Nome: Joao Pedro Magliano Goidanich
 ** Nome do Módulo: cliente.c
 ** Descrição/Explicação do Módulo:
 **  Este módulo implementa o Cliente do sistema Park-IUL.
** O Cliente realiza o check-in de uma viatura, comunica com o Servidor,
** aguarda pelo Servidor Dedicado, e posteriormente realiza o checkout
** quando o utilizador desejar terminar o estacionamento.
** A comunicação é feita através de FIFO e sinais (SIGUSR1, SIGHUP, SIGINT, SIGALRM).
 ***************************************************************************************/

// #define SO_HIDE_DEBUG                // Uncomment this line to hide all @DEBUG statements
#include "common.h"

/*** Variáveis Globais ***/
Estacionamento clientRequest;           // Pedido enviado do Cliente para o Servidor
volatile sig_atomic_t checkinConcluido = FALSE;
int recebeuRespostaServidor = FALSE;    // Variável que determina se o Cliente já recebeu uma resposta do Servidor


/**
 * @brief  Processamento do processo Cliente.
 *         OS ALUNOS NÃO DEVERÃO ALTERAR ESTA FUNÇÃO.
 */
int main () {
    so_debug("<");

    // c1_IniciaCliente:
    c1_1_ValidaFifoServidor(FILE_REQUESTS);
    c1_2_ArmaSinaisCliente();


    // c2_CheckinCliente:
    c2_1_InputEstacionamento(&clientRequest);
    FILE *fFifoServidor;
    c2_2_AbreFifoServidor(FILE_REQUESTS, &fFifoServidor);
    c2_3_EscrevePedido(fFifoServidor, clientRequest);

    c3_ProgramaAlarme(MAX_ESPERA);

    // c4_AguardaRespostaServidor:
    c4_1_EsperaRespostaServidor();
    c4_2_DesligaAlarme();
    c4_3_InputEsperaCheckout();

    c5_EncerraCliente();

    so_error("Cliente", "O programa nunca deveria ter chegado a este ponto!");
    so_debug(">");
    return 0;

}

/**
 * @brief  c1_1_ValidaFifoServidor Ler a descrição da tarefa C1.1 no enunciado
 * @param  filenameFifoServidor (I) O nome do FIFO do servidor (i.e., FILE_REQUESTS)
 */
 void c1_1_ValidaFifoServidor(char *filenameFifoServidor) {
    so_debug("< [@param filenameFifoServidor:%s]", filenameFifoServidor);

    struct stat st;

    if (stat(filenameFifoServidor, &st) != 0 || !S_ISFIFO(st.st_mode)) {
        so_error("C1.1", "FIFO %s não existe ou não é válido", filenameFifoServidor);
        exit(1);
    }

    so_success("C1.1", "");

    so_debug(">");
}


/**
 * @brief  c1_2_ArmaSinaisCliente Ler a descrição da tarefa C1.3 no enunciado
 */
 void c1_2_ArmaSinaisCliente() {
    so_debug("<");

    struct sigaction sa;
    sa.sa_sigaction = c6_TrataSigusr1;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        so_error("C1.2", "Erro ao armar SIGUSR1");
        exit(EXIT_FAILURE);
    }

    if (signal(SIGHUP, c7_TrataSighup) == SIG_ERR ||
        signal(SIGINT, c8_TrataCtrlC) == SIG_ERR ||
        signal(SIGALRM, c9_TrataAlarme) == SIG_ERR) {
        so_error("C1.2", "Erro ao armar SIGHUP, SIGINT ou SIGALRM");
        exit(EXIT_FAILURE);
    }

    so_success("C1.2", "Sinais armados com sucesso!");
    so_debug(">");
}




/**
 * @brief  c2_1_InputEstacionamento Ler a descrição da tarefa C2.1 no enunciado
 * @param  pclientRequest (O) pedido a ser enviado por este Cliente ao Servidor
 */
 void c2_1_InputEstacionamento(Estacionamento *pclientRequest) {
    so_debug("<");

    printf("Park-IUL: Check-in Viatura\n----------------------------\n");

    printf("Introduza a matrícula da viatura: ");
    so_gets(pclientRequest->viatura.matricula, sizeof(pclientRequest->viatura.matricula));
    if (!*so_trim(pclientRequest->viatura.matricula)) {
        so_error("C2.1", "Matrícula vazia");
        exit(1);
    }

    printf("Introduza o país da viatura: ");
    so_gets(pclientRequest->viatura.pais, sizeof(pclientRequest->viatura.pais));
    if (!*so_trim(pclientRequest->viatura.pais)) {
        so_error("C2.1", "País vazio");
        exit(1);
    }

    printf("Introduza a categoria da viatura: ");
    pclientRequest->viatura.categoria = so_getc();
    if (isspace(pclientRequest->viatura.categoria)) {
        so_error("C2.1", "Categoria vazia");
        exit(1);
    }

    printf("Introduza o nome do condutor: ");
    so_gets(pclientRequest->viatura.nomeCondutor, sizeof(pclientRequest->viatura.nomeCondutor));
    if (!*so_trim(pclientRequest->viatura.nomeCondutor)) {
        so_error("C2.1", "Nome do condutor vazio");
        exit(1);
    }

    pclientRequest->pidCliente = getpid();
    pclientRequest->pidServidorDedicado = -1;

    so_success("C2.1", "%s %s %c %s %d %d",
               pclientRequest->viatura.matricula,
               pclientRequest->viatura.pais,
               pclientRequest->viatura.categoria,
               pclientRequest->viatura.nomeCondutor,
               pclientRequest->pidCliente,
               pclientRequest->pidServidorDedicado);

    so_debug("> [*pclientRequest:[%s:%s:%c:%s:%d:%d]]", pclientRequest->viatura.matricula, pclientRequest->viatura.pais, pclientRequest->viatura.categoria, pclientRequest->viatura.nomeCondutor, pclientRequest->pidCliente, pclientRequest->pidServidorDedicado);
}


/**
 * @brief  c2_2_AbreFifoServidor Ler a descrição da tarefa C2.2 no enunciado
 * @param  filenameFifoServidor (I) O nome do FIFO do servidor (i.e., FILE_REQUESTS)
 * @param  pfFifoServidor (O) descritor aberto do ficheiro do FIFO do servidor
 */
 void c2_2_AbreFifoServidor(char *filenameFifoServidor, FILE **pfFifoServidor) {
    so_debug("< [@param filenameFifoServidor:%s]", filenameFifoServidor);

    *pfFifoServidor = fopen(filenameFifoServidor, "wb");
    if (*pfFifoServidor == NULL) {
        so_error("C2.2", "Erro ao abrir FIFO %s", filenameFifoServidor);
        exit(1);
    }

    so_success("C2.2", "");
    so_debug("> [*pfFifoServidor:%p]", *pfFifoServidor);
}


/**
 * @brief  c2_3_EscrevePedido Ler a descrição da tarefa C2.3 no enunciado
 * @param  fFifoServidor (I) descritor aberto do ficheiro do FIFO do servidor
 * @param  clientRequest (I) pedido a ser enviado por este Cliente ao Servidor
 */
 void c2_3_EscrevePedido(FILE *fFifoServidor, Estacionamento clientRequest) {
    so_debug("< [@param fFifoServidor:%p, clientRequest:[%s:%s:%c:%s:%d:%d]]",
             fFifoServidor,
             clientRequest.viatura.matricula,
             clientRequest.viatura.pais,
             clientRequest.viatura.categoria,
             clientRequest.viatura.nomeCondutor,
             clientRequest.pidCliente,
             clientRequest.pidServidorDedicado);

    size_t escritos = fwrite(&clientRequest, sizeof(Estacionamento), 1, fFifoServidor);

    if (escritos != 1) {
        so_error("C2.3", "Erro ao escrever pedido no FIFO");
        exit(1);
    }
    fflush(fFifoServidor);
    so_success("C2.3", "");
    so_debug(">");
}




/**
 * @brief  c3_ProgramaAlarme Ler a descrição da tarefa C3 no enunciado
 * @param  segundos (I) número de segundos a programar no alarme
 */
 void c3_ProgramaAlarme(int segundos) {
    so_debug("< [@param segundos:%d]", segundos);

    alarm(segundos);
    so_success("C3", "Espera resposta em %d segundos", segundos);

    so_debug(">");
}


/**
 * @brief  c4_1_EsperaRespostaServidor Ler a descrição da tarefa C4 no enunciado
 */

 void c4_1_EsperaRespostaServidor() {
    so_debug("<");

    pause(); // Espera passiva até receber o SIGUSR1

    // Após o sinal SIGUSR1, verifica-se a variável global
    if (checkinConcluido) {
        so_success("C4.1", "Check-in realizado com sucesso");
    }

    so_debug(">");
}




/**
 * @brief  c4_2_DesligaAlarme Ler a descrição da tarefa C4.1 no enunciado
 */
 void c4_2_DesligaAlarme() {
    so_debug("<");

    alarm(0); // cancela alarme
    so_success("C4.2", "Desliguei alarme");

    so_debug(">");
}

/**
 * @brief  c4_3_InputEsperaCheckout Ler a descrição da tarefa C4.2 no enunciado
 */
 void c4_3_InputEsperaCheckout() {
    so_debug("<");

    char buffer[20];

    while (1) {
        printf("Digite 'sair' para terminar o estacionamento: ");
        so_gets(buffer, sizeof(buffer));

        if (strcmp(so_trim(buffer), "sair") == 0) {
            so_success("C4.3", "Utilizador pretende terminar estacionamento");
            c5_EncerraCliente();   // <---- chama aqui!
            break;
        }
    }

    so_debug(">");
}


/**
 * @brief  c5_EncerraCliente Ler a descrição da tarefa C5 no enunciado
 *         OS ALUNOS NÃO DEVERÃO ALTERAR ESTA FUNÇÃO.
 */
void c5_EncerraCliente() {
    so_debug("<");

    c5_1_EnviaSigusr1AoServidor(clientRequest);
    c5_2_EsperaRespostaServidorETermina();

    so_debug(">");
}

/**
 * @brief  c5_1_EnviaSigusr1AoServidor Ler a descrição da tarefa C5.1 no enunciado
 * @param  clientRequest (I) pedido a ser enviado por este Cliente ao Servidor
 */
 void c5_1_EnviaSigusr1AoServidor(Estacionamento clientRequest) {
    so_debug("< [@param clientRequest:[%s:%s:%c:%s:%d:%d]]",
             clientRequest.viatura.matricula, clientRequest.viatura.pais,
             clientRequest.viatura.categoria, clientRequest.viatura.nomeCondutor,
             clientRequest.pidCliente, clientRequest.pidServidorDedicado);

    if (kill(clientRequest.pidServidorDedicado, SIGUSR1) != 0) {
        so_error("C5.1", "Erro ao enviar SIGUSR1 ao servidor dedicado");
        exit(1);
    }

    so_success("C5.1", "");

    so_debug(">");
}

/**
 * @brief  c5_2_EsperaRespostaServidorETermina Ler a descrição da tarefa C5.2 no enunciado
 */
 void c5_2_EsperaRespostaServidorETermina() {
    pause();
    so_success("C5.2", "");  // ← isto é esperado!
    exit(0);
}


/**
 * @brief  c6_TrataSigusr1 Ler a descrição da tarefa C6 no enunciado
 * @param  sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 * @param  siginfo (I) informação sobre o sinal
 * @param  context (I) contexto em que o sinal foi chamado
 */
 void c6_TrataSigusr1(int sinalRecebido, siginfo_t *siginfo, void *context) {
    so_debug("< [@param sinalRecebido:%d, siginfo->si_pid:%d]", sinalRecebido, siginfo->si_pid);

    (void)context;

    clientRequest.pidServidorDedicado = siginfo->si_pid;
    checkinConcluido = TRUE;

    so_success("C6", "Check-in concluído com sucesso pelo Servidor Dedicado %d", siginfo->si_pid);

    so_debug(">");
}


/**
 * @brief  c7_TrataSighup Ler a descrição da tarefa C7 no enunciado
 * @param  sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 * @param  siginfo (I) informação sobre o sinal
 */
 void c7_TrataSighup(int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    so_success("C7", "Estacionamento terminado");
    exit(0);

    so_debug(">");
}

/**
 * @brief  c8_TrataCtrlC Ler a descrição da tarefa c8 no enunciado
 * @param  sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 */
  void c8_TrataCtrlC(int sinalRecebido) {
 
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    so_success("C8", "Cliente: Shutdown");
    c5_EncerraCliente();

    so_debug(">");
}

/**
 * @brief  c9_TrataAlarme Ler a descrição da tarefa c9 no enunciado
 * @param  sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 */
 void c9_TrataAlarme(int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    so_error("C9", "Cliente: Timeout");
    so_debug(">");
    exit(0);

    
}
