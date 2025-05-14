/****************************************************************************************
 ** ISCTE-IUL: Trabalho prático 2 de Sistemas Operativos 2024/2025, Enunciado Versão 4+
 **
 ** Aluno: Nº: 130209      Nome: Joao Pedro Magliano Goidanich
 ** Nome do Módulo: servidor.c
 ** Descrição/Explicação do Módulo:
 **  Este módulo implementa o Servidor e os Servidores Dedicados do sistema Park-IUL.
** O Servidor principal gere pedidos de Clientes através de FIFO, atribui lugares de estacionamento,
** e cria processos Servidores Dedicados para cada cliente validado.
** Os Servidores Dedicados gerem a permanência do Cliente no parque,
** registam logs de entrada/saída e libertam o lugar ao checkout.
** Utiliza sinais (SIGINT, SIGCHLD, SIGUSR1, SIGUSR2, SIGHUP) para comunicação e controlo.
 **
 ***************************************************************************************/

// #define SO_HIDE_DEBUG                // Uncomment this line to hide all @DEBUG statements
#include "common.h"

void s5_TrataTerminouServidorDedicado(int, siginfo_t *, void *);


/*** Variáveis Globais ***/
Estacionamento clientRequest;           // Pedido enviado do Cliente para o Servidor
Estacionamento *lugaresEstacionamento;  // Array de Lugares de Estacionamento do parque
int dimensaoMaximaParque;               // Dimensão Máxima do parque (BD), recebida por argumento do programa
int indexClienteBD;                     // Índice do cliente que fez o pedido ao servidor/servidor dedicado na BD
long posicaoLogfile;                    // Posição no ficheiro Logfile para escrever o log da entrada corrente
LogItem logItem;                        // Informação da entrada corrente a escrever no logfile
volatile sig_atomic_t clienteQuerSair = FALSE;
int ultimoIndexSDTerminou = -1;

/**
 * @brief  Processamento do processo Servidor e dos processos Servidor Dedicado
 *         OS ALUNOS NÃO DEVERÃO ALTERAR ESTA FUNÇÃO.
 * @param  argc (I) número de Strings do array argv
 * @param  argv (I) array de lugares de estacionamento que irá servir de BD
 * @return Success (0) or not (<> 0)
 */
int main(int argc, char *argv[]) {
    so_debug("<");

    s1_IniciaServidor(argc, argv);
    s2_MainServidor();

    so_error("Servidor", "O programa nunca deveria ter chegado a este ponto!");
    so_debug(">");
    return 0;
}

/**
 * @brief  s1_iniciaServidor Ler a descrição da tarefa S1 no enunciado.
 *         OS ALUNOS NÃO DEVERÃO ALTERAR ESTA FUNÇÃO.
 * @param  argc (I) número de Strings do array argv
 * @param  argv (I) array de lugares de estacionamento que irá servir de BD
 */
void s1_IniciaServidor(int argc, char *argv[]) {
    so_debug("<");

    s1_1_ObtemDimensaoParque(argc, argv, &dimensaoMaximaParque);
    s1_2_CriaBD(dimensaoMaximaParque, &lugaresEstacionamento);
    s1_3_ArmaSinaisServidor();
    s1_4_CriaFifoServidor(FILE_REQUESTS);

    so_debug(">");
}

/**
 * @brief  s1_1_ObtemDimensaoParque Ler a descrição da tarefa S1.1 no enunciado
 * @param  argc (I) número de Strings do array argv
 * @param  argv (I) array de lugares de estacionamento que irá servir de BD
 * @param  pdimensaoMaximaParque (O) número máximo de lugares do parque, especificado pelo utilizador
 */
 void s1_1_ObtemDimensaoParque(int argc, char *argv[], int *pdimensaoMaximaParque) {
    if (argc != 2) {
        so_error("S1.1", "Número inválido de argumentos: %d", argc);
        exit(1);
    }

    *pdimensaoMaximaParque = atoi(argv[1]);

    if (*pdimensaoMaximaParque <= 0) {
        so_error("S1.1", "Dimensão inválida: %d", *pdimensaoMaximaParque);
        exit(1);
    }

    so_success("S1.1", "%d", *pdimensaoMaximaParque);
}


/**
 * @brief  s1_2_CriaBD Ler a descrição da tarefa S1.2 no enunciado
 * @param  dimensaoMaximaParque (I) número máximo de lugares do parque, especificado pelo utilizador
 * @param  plugaresEstacionamento (O) array de lugares de estacionamento que irá servir de BD
 */
 void s1_2_CriaBD(int dimensaoMaximaParque, Estacionamento **plugaresEstacionamento) {
    *plugaresEstacionamento = (Estacionamento *)malloc(dimensaoMaximaParque * sizeof(Estacionamento));

    if (*plugaresEstacionamento == NULL) {
        so_error("S1.2", "Erro ao alocar memória para BD");
        exit(1);
    }

    for (int i = 0; i < dimensaoMaximaParque; i++) {
        (*plugaresEstacionamento)[i].pidCliente = DISPONIVEL;
        (*plugaresEstacionamento)[i].pidServidorDedicado = DISPONIVEL;
    }

    so_success("S1.2", "Base de dados criada com %d lugares", dimensaoMaximaParque);
}


/**
 * @brief  s1_3_ArmaSinaisServidor Ler a descrição da tarefa S1.3 no enunciado
 */
 void s1_3_ArmaSinaisServidor() {
    // Armar SIGINT
    struct sigaction act_int;
    memset(&act_int, 0, sizeof(act_int));
    act_int.sa_handler = s3_TrataCtrlC;
    sigemptyset(&act_int.sa_mask);
    act_int.sa_flags = 0;

    if (sigaction(SIGINT, &act_int, NULL) == -1) {
        so_error("S1.3", "Erro ao armar o sinal SIGINT");
        exit(1);
    }

    // Armar SIGCHLD
    struct sigaction act_chld;
    memset(&act_chld, 0, sizeof(act_chld));
    act_chld.sa_sigaction = s5_TrataTerminouServidorDedicado;
    sigemptyset(&act_chld.sa_mask);
    act_chld.sa_flags = SA_SIGINFO | SA_RESTART;

    if (sigaction(SIGCHLD, &act_chld, NULL) == -1) {
        so_error("S1.3", "Erro ao armar o sinal SIGCHLD");
        exit(1);
    }

    so_success("S1.3", "SIGCHLD armado");
}












/**
 * @brief  s1_4_CriaFifoServidor Ler a descrição da tarefa S1.4 no enunciado
 * @param  filenameFifoServidor (I) O nome do FIFO do servidor (i.e., FILE_REQUESTS)
 */
 void s1_4_CriaFifoServidor(char *filenameFifoServidor) {
    unlink(filenameFifoServidor); // Apaga se já existir

    if (mkfifo(filenameFifoServidor, 0666) != 0) {
        so_error("S1.4", "Erro ao criar FIFO");
        exit(1);
    }

    so_success("S1.4", "FIFO %s criado", filenameFifoServidor);
}


/**
 * @brief  s2_MainServidor Ler a descrição da tarefa S2 no enunciado.
 *         OS ALUNOS NÃO DEVERÃO ALTERAR ESTA FUNÇÃO, exceto depois de
 *         realizada a função s2_1_AbreFifoServidor(), altura em que podem
 *         comentar o statement sleep abaixo (que, neste momento está aqui
 *         para evitar que os alunos tenham uma espera ativa no seu código)
 */
void s2_MainServidor() {
    so_debug("<");

    FILE *fFifoServidor;
    while (TRUE) { 
        s2_1_AbreFifoServidor(FILE_REQUESTS, &fFifoServidor);
        s2_2_LePedidosFifoServidor(fFifoServidor);
        sleep(10);  // TEMPORÁRIO, os alunos deverão comentar este statement apenas
                    // depois de terem a certeza que não terão uma espera ativa
    }

    so_debug(">");
}

/**
 * @brief  s2_1_AbreFifoServidor Ler a descrição da tarefa S2.1 no enunciado
 * @param  filenameFifoServidor (I) O nome do FIFO do servidor (i.e., FILE_REQUESTS)
 * @param  pfFifoServidor (O) descritor aberto do ficheiro do FIFO do servidor
 */
 void s2_1_AbreFifoServidor(char *filenameFifoServidor, FILE **pfFifoServidor) {
    *pfFifoServidor = fopen(filenameFifoServidor, "rb");

    if (*pfFifoServidor == NULL) {
        so_error("S2.1", "Erro ao abrir FIFO para leitura");
        s4_EncerraServidor(filenameFifoServidor);
        exit(1);
    }

    so_success("S2.1", "");
}


/**
 * @brief  s2_2_LePedidosFifoServidor Ler a descrição da tarefa S2.2 no enunciado.
 *         OS ALUNOS NÃO DEVERÃO ALTERAR ESTA FUNÇÃO.
 * @param  fFifoServidor (I) descritor aberto do ficheiro do FIFO do servidor
 */
void s2_2_LePedidosFifoServidor(FILE *fFifoServidor) {
    so_debug("<");

    int terminaCiclo2 = FALSE;
    while (TRUE) {
        terminaCiclo2 = s2_2_1_LePedido(fFifoServidor, &clientRequest);
        if (terminaCiclo2)
            break;
        s2_2_2_ProcuraLugarDisponivelBD(clientRequest, lugaresEstacionamento, dimensaoMaximaParque, &indexClienteBD);
        s2_2_3_CriaServidorDedicado(lugaresEstacionamento, indexClienteBD);
    }

    so_debug(">");
}

/**
 * @brief  s2_2_1_LePedido Ler a descrição da tarefa S2.2.1 no enunciado
 * @param  fFifoServidor (I) descritor aberto do ficheiro do FIFO do servidor
 * @param  pclientRequest (O) pedido recebido, enviado por um Cliente
 * @return TRUE se não conseguiu ler um pedido porque o FIFO não tem mais pedidos.
 */
 int s2_2_1_LePedido(FILE *fFifoServidor, Estacionamento *pclientRequest) {
    size_t lidos = fread(pclientRequest, sizeof(Estacionamento), 1, fFifoServidor);

    if (lidos == 1) {
        so_success("S2.2.1", "Li Pedido do FIFO");
        return FALSE;
    }

    if (ferror(fFifoServidor)) {
        fclose(fFifoServidor);
        so_error("S2.2.1", "Erro ao ler do FIFO");
        s4_EncerraServidor(FILE_REQUESTS);
        exit(1);
    }

    fclose(fFifoServidor);
    so_success("S2.2.1", "Não há mais registos no FIFO");
    return TRUE;
}



/**
 * @brief  s2_2_2_ProcuraLugarDisponivelBD Ler a descrição da tarefa S2.2.2 no enunciado
 * @param  clientRequest (I) pedido recebido, enviado por um Cliente
 * @param  lugaresEstacionamento (I) array de lugares de estacionamento que irá servir de BD
 * @param  dimensaoMaximaParque (I) número máximo de lugares do parque, especificado pelo utilizador
 * @param  pindexClienteBD (O) índice do lugar correspondente a este pedido na BD (>= 0), ou -1 se não houve nenhum lugar disponível
 */
 void s2_2_2_ProcuraLugarDisponivelBD(Estacionamento clientRequest, Estacionamento *lugaresEstacionamento, int dimensaoMaximaParque, int *pindexClienteBD) {
    *pindexClienteBD = -1;

    for (int i = 0; i < dimensaoMaximaParque; i++) {
        if (lugaresEstacionamento[i].pidCliente == DISPONIVEL) {
            lugaresEstacionamento[i] = clientRequest;
            *pindexClienteBD = i;
            so_success("S2.2.2", "Reservei Lugar: %d", i);
            return;
        }
    }

    // Corrigido: era so_success → deve ser so_error (porque é uma situação de erro lógica)
    so_error("S2.2.2", "Não há lugares disponíveis");
}



/**
 * @brief  s2_2_3_CriaServidorDedicado    Ler a descrição da tarefa S2.2.3 no enunciado
 * @param  lugaresEstacionamento (I) array de lugares de estacionamento que irá servir de BD
 * @param  indexClienteBD (I) índice do lugar correspondente a este pedido na BD (>= 0), ou -1 se não houve nenhum lugar disponível
 */
 void s2_2_3_CriaServidorDedicado(Estacionamento *lugaresEstacionamento, int indexClienteBD) {
    pid_t pid = fork();

    if (pid < 0) {
        so_error("S2.2.3", "Erro ao criar Servidor Dedicado");
        s4_EncerraServidor(FILE_REQUESTS);
        exit(1);
    }
    

    if (pid == 0) {
        so_success("S2.2.3", "SD: Nasci com PID %d", getpid());
        sd7_MainServidorDedicado();
        exit(0);
    } else {
        if (indexClienteBD != -1) {
            lugaresEstacionamento[indexClienteBD].pidServidorDedicado = pid;
        }
        so_success("S2.2.3", "Servidor: Iniciei SD %d", pid);
    }
}


/**
 * @brief  s3_TrataCtrlC    Ler a descrição da tarefa S3 no enunciado
 * @param  sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 */
 void s3_TrataCtrlC(int sinalRecebido) {
    so_success("S3", "Servidor: Start Shutdown");


    for (int i = 0; i < dimensaoMaximaParque; i++) {
        if (lugaresEstacionamento[i].pidServidorDedicado > 0) {
            kill(lugaresEstacionamento[i].pidServidorDedicado, SIGUSR2);
        }
    }
    s4_EncerraServidor(FILE_REQUESTS);
    fflush(stdout);

 }


/**
 * @brief  s4_EncerraServidor Ler a descrição da tarefa S4 no enunciado
 * @param  filenameFifoServidor (I) O nome do FIFO do servidor (i.e., FILE_REQUESTS)
 */
 void s4_EncerraServidor(char *filenameFifoServidor) {
    if (unlink(filenameFifoServidor) != 0) {
        so_error("S4", "Erro ao remover FIFO");
    }

    so_success("S4", "Servidor: End Shutdown");
    exit(0);
}




/**
 * @brief  s5_TrataTerminouServidorDedicado    Ler a descrição da tarefa S5 no enunciado
 * @param  sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 */
 void s5_TrataTerminouServidorDedicado(int sinalRecebido, siginfo_t *info, void *context) {
    (void)sinalRecebido;
    (void)context;

    int status;
    pid_t pid_terminado;

    while ((pid_terminado = waitpid(-1, &status, 0)) > 0) {
        for (int i = 0; i < dimensaoMaximaParque; i++) {
            if (lugaresEstacionamento[i].pidServidorDedicado == pid_terminado) {
                so_success("S5", "Servidor: Confirmo que terminou o SD %d", pid_terminado);

                // Libertar lugar
                lugaresEstacionamento[i].pidServidorDedicado = DISPONIVEL;
                lugaresEstacionamento[i].pidCliente = DISPONIVEL;
                memset(&lugaresEstacionamento[i].viatura, 0, sizeof(Viatura));

                so_success("S6", "Servidor: Liberei completamente o lugar %d", i);
                break; // encontrou o SD terminado
            }
        }
    }
}

 

void s6_LibertaLugarNoServidorPrincipal() {
    if (ultimoIndexSDTerminou == -1) {
        so_error("S6", "Servidor: Não encontrei índice do SD terminado");
        return;
    }

    lugaresEstacionamento[ultimoIndexSDTerminou].pidServidorDedicado = DISPONIVEL;
    lugaresEstacionamento[ultimoIndexSDTerminou].pidCliente = DISPONIVEL;
    memset(&lugaresEstacionamento[ultimoIndexSDTerminou].viatura, 0, sizeof(Viatura));
    so_success("S6", "Servidor: Liberei completamente o lugar %d", ultimoIndexSDTerminou);
}




/**
 * @brief  sd7_ServidorDedicado Ler a descrição da tarefa SD7 no enunciado
 *         OS ALUNOS NÃO DEVERÃO ALTERAR ESTA FUNÇÃO.
 */
void sd7_MainServidorDedicado() {
    so_debug("<");

    // sd7_IniciaServidorDedicado:
    sd7_1_ArmaSinaisServidorDedicado();
    sd7_2_ValidaPidCliente(clientRequest);
    sd7_3_ValidaLugarDisponivelBD(indexClienteBD);

    // sd8_ValidaPedidoCliente:
    sd8_1_ValidaMatricula(clientRequest);
    sd8_2_ValidaPais(clientRequest);
    sd8_3_ValidaCategoria(clientRequest);
    sd8_4_ValidaNomeCondutor(clientRequest);

    // sd9_EntradaCliente:
    sd9_1_AdormeceTempoRandom();
    sd9_2_EnviaSigusr1AoCliente(clientRequest);
    sd9_3_EscreveLogEntradaViatura(FILE_LOGFILE, clientRequest, &posicaoLogfile, &logItem);

    // sd10_AcompanhaCliente:
    sd10_1_AguardaCheckout();
    sd10_2_EscreveLogSaidaViatura(FILE_LOGFILE, posicaoLogfile, logItem);

    sd11_EncerraServidorDedicado();

    so_error("Servidor Dedicado", "O programa nunca deveria ter chegado a este ponto!");

    so_debug(">");
}

/**
 * @brief  sd7_1_ArmaSinaisServidorDedicado    Ler a descrição da tarefa SD7.1 no enunciado
 */
 void sd7_1_ArmaSinaisServidorDedicado() {
    struct sigaction act_usr2, act_usr1, ign;

    // SIGUSR2 → Terminar o Servidor Dedicado (SD12)
    memset(&act_usr2, 0, sizeof(act_usr2));
    act_usr2.sa_handler = sd12_TrataSigusr2;
    sigemptyset(&act_usr2.sa_mask);
    act_usr2.sa_flags = 0;
    if (sigaction(SIGUSR2, &act_usr2, NULL) == -1) {
        so_error("SD7.1", "Erro ao armar SIGUSR2");
        exit(1);
    }

    // SIGUSR1 → Pedido de checkout do Cliente (SD13)
    memset(&act_usr1, 0, sizeof(act_usr1));
    act_usr1.sa_handler = sd13_TrataSigusr1;
    sigemptyset(&act_usr1.sa_mask);
    act_usr1.sa_flags = 0;
    if (sigaction(SIGUSR1, &act_usr1, NULL) == -1) {
        so_error("SD7.1", "Erro ao armar SIGUSR1");
        exit(1);
    }

    // Ignorar SIGINT
    memset(&ign, 0, sizeof(ign));
    ign.sa_handler = SIG_IGN;
    sigemptyset(&ign.sa_mask);
    ign.sa_flags = 0;
    if (sigaction(SIGINT, &ign, NULL) == -1) {
        so_error("SD7.1", "Erro ao ignorar SIGINT");
        exit(1);
    }

    so_success("SD7.1", "");
}




/**
 * @brief  sd7_2_ValidaPidCliente    Ler a descrição da tarefa SD7.2 no enunciado
 * @param  clientRequest (I) pedido recebido, enviado por um Cliente
 */
 void sd7_2_ValidaPidCliente(Estacionamento clientRequest) {
    if (clientRequest.pidCliente <= 0) {
        so_error("SD7.2", "PID inválido: %d", clientRequest.pidCliente);
        exit(1);
    }

    so_success("SD7.2", "");
}


/**
 * @brief  sd7_3_ValidaLugarDisponivelBD    Ler a descrição da tarefa SD7.3 no enunciado
 * @param  indexClienteBD (I) índice do lugar correspondente a este pedido na BD (>= 0), ou -1 se não houve nenhum lugar disponível
 */
 void sd7_3_ValidaLugarDisponivelBD(int indexClienteBD)
 {
    if (indexClienteBD < 0 || indexClienteBD >= dimensaoMaximaParque)
 {
        so_error("SD7.3", "Index fora dos limites");
        sd11_EncerraServidorDedicado();
        exit(1);
    }
    so_success("SD7.3", "");
 }    



/**
 * @brief  sd8_1_ValidaMatricula Ler a descrição da tarefa SD8.1 no enunciado
 * @param  clientRequest (I) pedido recebido, enviado por um Cliente
 */
 void sd8_1_ValidaMatricula(Estacionamento clientRequest) {
    char *m = clientRequest.viatura.matricula;

    for (int i = 0; m[i] != '\0'; i++) {
        if (!isdigit(m[i]) && !(m[i] >= 'A' && m[i] <= 'Z')) {
            so_error("SD8.1", "Matrícula inválida: %s", m);
            sd11_EncerraServidorDedicado();
        }
    }

    so_success("SD8.1", "");
}


/**
 * @brief  sd8_2_ValidaPais Ler a descrição da tarefa SD8.2 no enunciado
 * @param  clientRequest (I) pedido recebido, enviado por um Cliente
 */
 void sd8_2_ValidaPais(Estacionamento clientRequest) {
    char *p = clientRequest.viatura.pais;

    if (strlen(p) != 2 || !isupper(p[0]) || !isupper(p[1])) {
        so_error("SD8.2", "País inválido: %s", p);
        sd11_EncerraServidorDedicado();
    }

    so_success("SD8.2", "");
}


/**
 * @brief  sd8_3_ValidaCategoria Ler a descrição da tarefa SD8.3 no enunciado
 * @param  clientRequest (I) pedido recebido, enviado por um Cliente
 */
 void sd8_3_ValidaCategoria(Estacionamento clientRequest) {
    char c = clientRequest.viatura.categoria;

    if (c != 'P' && c != 'L' && c != 'M') {
        so_error("SD8.3", "Categoria inválida: %c", c);
        sd11_EncerraServidorDedicado();
    }

    so_success("SD8.3", "");
}


/**
 * @brief  sd8_4_ValidaNomeCondutor Ler a descrição da tarefa SD8.4 no enunciado
 * @param  clientRequest (I) pedido recebido, enviado por um Cliente
 */
 void sd8_4_ValidaNomeCondutor(Estacionamento clientRequest) {
    FILE *fp = fopen("/etc/passwd", "r");
    if (!fp) {
        so_error("SD8.4", "Erro ao abrir /etc/passwd");
        sd11_EncerraServidorDedicado();
    }

    char linha[256];
    int encontrado = FALSE;

    while (fgets(linha, sizeof(linha), fp)) {
        if (strstr(linha, clientRequest.viatura.nomeCondutor)) {
            encontrado = TRUE;
            break;
        }
    }

    fclose(fp);

    if (!encontrado) {
        so_error("SD8.4", "Nome de condutor não encontrado: %s", clientRequest.viatura.nomeCondutor);
        sd11_EncerraServidorDedicado();
    }

    so_success("SD8.4", "");
}


/**
 * @brief  sd9_1_AdormeceTempoRandom Ler a descrição da tarefa SD9.1 no enunciado
 */
 void sd9_1_AdormeceTempoRandom() {
    int t = so_random_between_values(1, MAX_ESPERA);
    so_success("SD9.1", "%d", t);
    sleep(t);
}


/**
 * @brief  sd9_2_EnviaSigusr1AoCliente Ler a descrição da tarefa SD9.2 no enunciado
 * @param  clientRequest (I) pedido recebido, enviado por um Cliente
 */
 void sd9_2_EnviaSigusr1AoCliente(Estacionamento clientRequest) {
    if (kill(clientRequest.pidCliente, SIGUSR1) != 0) {
        so_error("SD9.2", "Erro ao enviar sinal para o Cliente");
        sd11_EncerraServidorDedicado();
    }

    so_success("SD9.2", "SD: Confirmei Cliente Lugar %d", indexClienteBD);
}


/**
 * @brief  sd9_3_EscreveLogEntradaViatura Ler a descrição da tarefa SD9.3 no enunciado
 * @param  logFilename (I) O nome do ficheiro de Logfile (i.e., FILE_LOGFILE)
 * @param  clientRequest (I) pedido recebido, enviado por um Cliente
 * @param  pposicaoLogfile (O) posição do ficheiro Logfile mesmo antes de inserir o log desta viatura
 * @param  plogItem (O) registo de Log para esta viatura
 */
 void sd9_3_EscreveLogEntradaViatura(char *logFilename, Estacionamento clientRequest, long *pposicaoLogfile, LogItem *plogItem) {
    FILE *f = fopen(logFilename, "ab+");  // append + leitura binária
    if (!f) {
        so_error("SD9.3", "Erro ao abrir %s", logFilename);
        sd11_EncerraServidorDedicado();
    }

    fseek(f, 0, SEEK_END);
long pos = ftell(f);
if (pos == -1L) {
    so_error("SD9.3", "Erro ao obter posição do ficheiro");
    fclose(f);
    sd11_EncerraServidorDedicado();
}
*pposicaoLogfile = pos;
  // posição atual para guardar

    // Preencher LogItem
    *plogItem = (LogItem){ .viatura = clientRequest.viatura };
    
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(plogItem->dataEntrada, sizeof(plogItem->dataEntrada), "%Y-%m-%dT%Hh%M", tm_info);
    memset(plogItem->dataSaida, 0, sizeof(plogItem->dataSaida)); // ainda não saiu

    fwrite(plogItem, sizeof(LogItem), 1, f);
    fclose(f);

    so_success("SD9.3", "SD: Guardei log na posição %ld: Entrada Cliente %s em %s", *pposicaoLogfile, plogItem->viatura.matricula, plogItem->dataEntrada);
}


/**
 * @brief  sd10_1_AguardaCheckout Ler a descrição da tarefa SD10.1 no enunciado
 */
 

 void sd10_1_AguardaCheckout() {
     while (!clienteQuerSair) {
         pause(); // Espera por sinal
     }
 
     so_success("SD10.1", "SD: A viatura %s deseja sair do parque", clientRequest.viatura.matricula);
 }
 

/**
 * @brief  sd10_2_EscreveLogSaidaViatura Ler a descrição da tarefa SD10.2 no enunciado
 * @param  logFilename (I) O nome do ficheiro de Logfile (i.e., FILE_LOGFILE)
 * @param  posicaoLogfile (I) posição do ficheiro Logfile mesmo antes de inserir o log desta viatura
 * @param  logItem (I) registo de Log para esta viatura
 */
 void sd10_2_EscreveLogSaidaViatura(char *logFilename, long posicaoLogfile, LogItem logItem) {
    FILE *f = fopen(logFilename, "rb+");  // leitura e escrita
    if (!f) {
        so_error("SD10.2", "Erro ao abrir %s", logFilename);
        sd11_EncerraServidorDedicado();
    }

    fseek(f, posicaoLogfile, SEEK_SET);

    // Atualiza dataSaida
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(logItem.dataSaida, sizeof(logItem.dataSaida), "%Y-%m-%dT%Hh%M", tm_info);

    fwrite(&logItem, sizeof(LogItem), 1, f);
    fclose(f);

    so_success("SD10.2", "SD: Atualizei log na posição %ld: Saída Cliente %s em %s", posicaoLogfile, logItem.viatura.matricula, logItem.dataSaida);
}


/**
 * @brief  sd11_EncerraServidorDedicado Ler a descrição da tarefa SD11 no enunciado
 *         OS ALUNOS NÃO DEVERÃO ALTERAR ESTA FUNÇÃO.
 */
void sd11_EncerraServidorDedicado() {
    so_debug("<");

    sd11_1_LibertaLugarViatura(lugaresEstacionamento, indexClienteBD);
    sd11_2_EnviaSighupAoClienteETermina(clientRequest);

    so_debug(">");
}

/**
 * @brief  sd11_1_LibertaLugarViatura Ler a descrição da tarefa SD11.1 no enunciado
 * @param  lugaresEstacionamento (I) array de lugares de estacionamento que irá servir de BD
 * @param  indexClienteBD (I) índice do lugar correspondente a este pedido na BD (>= 0), ou -1 se não houve nenhum lugar disponível
 */
 void sd11_1_LibertaLugarViatura(Estacionamento *lugaresEstacionamento, int indexClienteBD) {
    if (indexClienteBD == -1) {
        so_error("SD11.1", "Lugar não atribuído");
        return;
    }

    lugaresEstacionamento[indexClienteBD].pidCliente = DISPONIVEL;
    lugaresEstacionamento[indexClienteBD].pidServidorDedicado = DISPONIVEL;

    so_success("SD11.1", "SD: Libertei Lugar: %d", indexClienteBD);
}


/**
 * @brief  sd11_2_EnviaSighupAoClienteETerminaSD Ler a descrição da tarefa SD11.2 no enunciado
 * @param  clientRequest (I) pedido recebido, enviado por um Cliente
 */
 void sd11_2_EnviaSighupAoClienteETermina(Estacionamento clientRequest) {
    if (kill(clientRequest.pidCliente, SIGHUP) != 0) {
        so_error("SD11.2", "Erro ao enviar SIGHUP ao Cliente");
        exit(0);
    }

    so_success("SD11.2", "SD: Shutdown");
    exit(0);
}


/**
 * @brief  sd12_TrataSigusr2    Ler a descrição da tarefa SD12 no enunciado
 * @param  sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 */
 void sd12_TrataSigusr2(int sinalRecebido) {
    so_success("SD12", "SD: Recebi pedido do Servidor para terminar");
    sd11_EncerraServidorDedicado();
}


/**
 * @brief  sd13_TrataSigusr1    Ler a descrição da tarefa SD13 no enunciado
 * @param  sinalRecebido (I) número do sinal que é recebido por esta função (enviado pelo SO)
 */
 void sd13_TrataSigusr1(int sinalRecebido) {
    so_success("SD13", "SD: Recebi pedido do Cliente para terminar o estacionamento");
    clienteQuerSair = TRUE;
}
