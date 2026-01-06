#include "utils.h"

char username[TAM_NOME];
int pid_cliente;
char fifo_cliente[50];
int fd_cliente;
int conectado = 0;
volatile sig_atomic_t terminar_cliente = 0;

void limpar_recursos() {
    if (fd_cliente > 0) {
        close(fd_cliente);
    }
    unlink(fifo_cliente);
}

void sigint_handler(int sig) {
    terminar_cliente = 1;
}

int enviar_mensagem(MensagemCliente *msg) {
    int fd = open(FIFO_CONTROLADOR, O_WRONLY);
    if (fd == -1) {
        printf("[ERRO] Controlador não está em execução!\n");
        return -1;
    }
    
    write(fd, msg, sizeof(MensagemCliente));
    close(fd);
    
    // Aguardar resposta
    MensagemCliente resposta;
    int n = read(fd_cliente, &resposta, sizeof(MensagemCliente));
    
    if (n > 0) {
        printf("%s\n", resposta.mensagem);
        return resposta.sucesso;
    }
    
    return -1;
}

int fazer_login() {
    MensagemCliente msg;
    memset(&msg, 0, sizeof(MensagemCliente));
    
    msg.tipo = MSG_LOGIN;
    strcpy(msg.username, username);
    msg.pid_cliente = pid_cliente;
    
    return enviar_mensagem(&msg);
}

void processar_comando(char *comando) {
    char cmd[50];
    sscanf(comando, "%s", cmd);
    
    MensagemCliente msg;
    memset(&msg, 0, sizeof(MensagemCliente));
    strcpy(msg.username, username);
    msg.pid_cliente = pid_cliente;
    
    if (strcmp(cmd, "agendar") == 0) {
        int hora, distancia;
        char local[TAM_LOCAL];
        
        if (sscanf(comando, "agendar %d %s %d", &hora, local, &distancia) == 3) {
            msg.tipo = MSG_AGENDAR;
            msg.hora_agendada = hora;
            strcpy(msg.local_partida, local);
            msg.distancia_km = distancia;
            enviar_mensagem(&msg);
        } else {
            printf("Uso: agendar <hora> <local> <distancia>\n");
        }
    }
    else if (strcmp(cmd, "cancelar") == 0) {
        int id;
        if (sscanf(comando, "cancelar %d", &id) == 1) {
            msg.tipo = MSG_CANCELAR;
            msg.id_servico = id;
            enviar_mensagem(&msg);
        } else {
            printf("Uso: cancelar <id>\n");
        }
    }
    else if (strcmp(cmd, "consultar") == 0) {
        msg.tipo = MSG_CONSULTAR;
        enviar_mensagem(&msg);
    }
    else if (strcmp(cmd, "entrar") == 0) {
        char destino[TAM_LOCAL];
        if (sscanf(comando, "entrar %s", destino) == 1) {
            // Enviar para o veículo via FIFO específico
            char fifo_veiculo[50];
            sprintf(fifo_veiculo, "/tmp/fifo_veiculo_%d", pid_cliente);
            
            int fd = open(fifo_veiculo, O_WRONLY | O_NONBLOCK);
            if (fd > 0) {
                MensagemVeiculo msg_veiculo;
                msg_veiculo.tipo = 1; // entrar
                strcpy(msg_veiculo.destino, destino);
                sprintf(msg_veiculo.mensagem, "Cliente entrou. Destino: %s", destino);
                write(fd, &msg_veiculo, sizeof(MensagemVeiculo));
                close(fd);
                printf("Entrando no veículo. Destino: %s\n", destino);
            } else {
                printf("[ERRO] Nenhum veículo à espera!\n");
            }
        } else {
            printf("Uso: entrar <destino>\n");
        }
    }
    else if (strcmp(cmd, "sair") == 0) {
        char fifo_veiculo[50];
        sprintf(fifo_veiculo, "/tmp/fifo_veiculo_%d", pid_cliente);
        
        int fd = open(fifo_veiculo, O_WRONLY | O_NONBLOCK);
        if (fd > 0) {
            MensagemVeiculo msg_veiculo;
            msg_veiculo.tipo = 2; // sair
            strcpy(msg_veiculo.mensagem, "Cliente saiu a meio da viagem");
            write(fd, &msg_veiculo, sizeof(MensagemVeiculo));
            close(fd);
            printf("Saindo do veículo...\n");
        } else {
            printf("[ERRO] Não está em viagem!\n");
        }
    }
    else if (strcmp(cmd, "terminar") == 0) {
        msg.tipo = MSG_TERMINAR;
        enviar_mensagem(&msg);
        conectado = 0;
    }
    else if (strcmp(cmd, "ajuda") == 0) {
        printf("\nComandos disponíveis:\n");
        printf("  agendar <hora> <local> <distancia> - Agendar serviço\n");
        printf("  cancelar <id>                      - Cancelar serviço\n");
        printf("  consultar                          - Ver seus serviços\n");
        printf("  entrar <destino>                   - Entrar no veículo\n");
        printf("  sair                               - Sair do veículo\n");
        printf("  terminar                           - Desconectar\n");
        printf("  ajuda                              - Mostrar esta ajuda\n\n");
    }
    else {
        printf("Comando desconhecido. Digite 'ajuda' para ver comandos.\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <username>\n", argv[0]);
        return 1;
    }
    
    strcpy(username, argv[1]);
    pid_cliente = getpid();
    sprintf(fifo_cliente, FIFO_CLIENTE, pid_cliente);
    
    // Verificar se controlador está ativo
    if (access(FIFO_CONTROLADOR, F_OK) == -1) {
        printf("[ERRO] Controlador não está em execução!\n");
        return 1;
    }
    
    // Criar FIFO do cliente
    if (mkfifo(fifo_cliente, 0666) == -1) {
        perror("Erro ao criar FIFO do cliente");
        return 1;
    }
    
    fd_cliente = open(fifo_cliente, O_RDWR);
    if (fd_cliente == -1) {
        perror("Erro ao abrir FIFO do cliente");
        unlink(fifo_cliente);
        return 1;
    }
    
    signal(SIGINT, sigint_handler);
    
    // Fazer login
    printf("=== CLIENTE DE TRANSPORTE ===\n");
    printf("A conectar como %s...\n", username);
    
    if (fazer_login() != 1) {
        printf("Falha no login!\n");
        limpar_recursos();
        return 1;
    }
    
    conectado = 1;
    printf("Digite 'ajuda' para ver os comandos disponíveis.\n\n");
    
    // Loop principal com select()
    char comando[TAM_COMANDO];
    fd_set read_fds;
    
    while (conectado && !terminar_cliente) {
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(fd_cliente, &read_fds);
        
        int max_fd = (fd_cliente > STDIN_FILENO) ? fd_cliente : STDIN_FILENO;
        
        printf("%s> ", username);
        fflush(stdout);
        
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        
        if (activity < 0) {
            perror("Erro no select");
            break;
        }
        
        // Dados do stdin (comando do utilizador)
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            if (fgets(comando, TAM_COMANDO, stdin) == NULL) break;
            
            comando[strcspn(comando, "\n")] = 0;
            if (strlen(comando) > 0) {
                processar_comando(comando);
            }
        }
        
        // Mensagens do veículo ou controlador
        if (FD_ISSET(fd_cliente, &read_fds)) {
            MensagemVeiculo msg_veiculo;
            int n = read(fd_cliente, &msg_veiculo, sizeof(MensagemVeiculo));
            
            if (n > 0) {
                printf("\n[VEÍCULO] %s\n", msg_veiculo.mensagem);
            }
        }
    }
    
    if (terminar_cliente) {
        printf("\nA terminar cliente...\n");
    }
    
    limpar_recursos();
    printf("Desconectado.\n");
    return 0;
}
