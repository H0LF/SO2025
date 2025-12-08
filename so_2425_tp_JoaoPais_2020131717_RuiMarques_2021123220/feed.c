#include "utils.h"

int main(int argc, char *argv[]) {

    Envia1 login;
    Envia2 env;
    Mresposta recv;
    MSGTopicos ltopicos;

    char Client_Final[TAM_FIFO]; // Nome do FIFO do cliente
    int aux;
    int pid = getpid();
    
    int fd_envio;
    int fd_recebe;
    fd_set readfds; // Conjunto de descritores para o `select`

    if (argc != 2) {
        printf("%s\n", "[Erro]: Não inseriu o identificador do veículo!");
        exit(1);
    }
    
    login.login.pid = getpid(); // Obtém o PID do processo
    env.msg.pid = getpid();
    sprintf(Client_Final, Client_Pipe, login.login.pid);

    // Cria o FIFO do cliente
    if (mkfifo(Client_Final, 0666) == -1) {
        perror("[Erro]: Falha ao criar o FIFO do cliente");
        exit(1);
    }

    // Configuração inicial do utilizador
    strcpy(login.login.nome, argv[1]);
    strcpy(env.msg.nome, argv[1]);
    login.tipo = 0;
    login.login.flag = 0;

    fd_envio = open(FIFO_SERVER, O_WRONLY);
    if (fd_envio == -1) {
        perror("[Erro]: Não foi possível conectar à central de controlo.");
        unlink(Client_Final);
        exit(1);
    }
    write(fd_envio, &login, sizeof(Login));
    close(fd_envio);

    fd_recebe = open(Client_Final, O_RDWR);
    if (fd_recebe == -1) {
        perror("[Erro]: Não foi possível abrir o FIFO do cliente para leitura.");
        unlink(Client_Final);
        exit(1);
    }

    printf("feed> ");
    fflush(stdout);

    // Loop principal utilizando `select`
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(fd_recebe, &readfds); // Adiciona FIFO do cliente
        FD_SET(0, &readfds); // Adiciona entrada do utilizador

        int max_fd = (fd_recebe > 0) ? fd_recebe : 0;
        int ready = select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if (ready > 0) {
            if (FD_ISSET(fd_recebe, &readfds)) {
                // Mensagem do manager disponível

                if (read(fd_recebe, &aux, sizeof(int)) > 0){
                    switch (aux)
                    {
                    case 2:
                        if (read(fd_recebe, &recv, sizeof(Mresposta)) > 0){
                            // Verifica flag para encerrar
                            if (recv.flag == -1) {
                                printf("\r[MANAGER]: %s\n", recv.conteudo);
                                unlink(Client_Final);
                                exit(0);
                            } else if (recv.flag == 1) {
                                printf("\r%s\n", recv.conteudo);
                            } else {
                                printf("\r[MANAGER]: %s\n", recv.conteudo);
                            }
                            // Redesenha o prompt
                            printf("feed> ");
                            fflush(stdout);
                        }
                        break;
                    case 3:
                        if (read(fd_recebe, &ltopicos, sizeof(MSGTopicos)) > 0){
                            if (ltopicos.num_Topicos > 0) {
                                printf("\r[MANAGER]: Lista de tópicos disponíveis:\n");
                                for (int i = 0; i < ltopicos.num_Topicos; i++) {
                                    printf(" - %s (%d mensagens persistentes)\n",
                                        ltopicos.top[i].titulo, ltopicos.top[i].nperm);
                                }
                            } else {
                                printf("\r[MANAGER]: %s\n", recv.conteudo); // Mensagem: sem tópicos
                            }
                            // Redesenha o prompt
                            printf("feed> ");
                            fflush(stdout);
                        }
                    default:
                        break;
                    }
                }
                
            }

            if (FD_ISSET(0, &readfds)) {
                // Entrada do utilizador disponível
                if (fgets(env.msg.conteudo, sizeof(env.msg.conteudo), stdin) != NULL) {
                    env.msg.conteudo[strcspn(env.msg.conteudo, "\n")] = '\0'; // Remove '\n'
                    env.tipo = 1;
                    //env.msg.pid = getpid();
                    
                    fd_envio = open(FIFO_SERVER, O_WRONLY);
                    if (fd_envio != -1) {
                        write(fd_envio, &env, sizeof(Envia2));
                        close(fd_envio);
                    }

                    // Limpa o conteúdo de `env.conteudo` após envio
                    //memset(env.conteudo, 0, sizeof(env.conteudo));

                    // Limpa o buffer para evitar restos em redesenhos
                    printf("feed> ");
                    fflush(stdout);
                }
            }
        } else if (ready == -1) {
            perror("[Erro]: Falha no select");
            break;
        }
    }

    close(fd_recebe);
    unlink(Client_Final);
    return 0;
}
