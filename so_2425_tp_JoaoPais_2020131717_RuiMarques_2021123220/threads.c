#include "utils.h"

// Função para a thread de comunicação
void *thread_comunicacao(void *args) {
    Data * dt = (Data*)args;
    
    Login feedlog;
    FeedData recv;
    Envia3 env_topicos;
    Envia4 env;

    char cliente_pipe[100];
    int cliente_fd;
    int aux;

    //printf("Thread de comunicação iniciada.\n");

    // Abrir FIFO do servidor para leitura
    dt->fd_server_fifo = open(FIFO_SERVER, O_RDWR);
    if (dt->fd_server_fifo < 0) {
        perror("Erro ao abrir FIFOs");
        exit(1);
    }

    while (dt->trinco!= 1) {
        
        if (read(dt->fd_server_fifo, &aux, sizeof(int)) > 0){
            switch (aux)
            {
            case 0:
                // Limpar estrutura `recv` antes de ler uma nova mensagem
                memset(&feedlog, 0, sizeof(Login));

                if (read(dt->fd_server_fifo, &feedlog, sizeof(Login)) > 0) {
                    // Processar mensagem recebida
                    int res = processa_mensagem(dt, &feedlog);
                    
                    // Construir caminho do FIFO do cliente
                    sprintf(cliente_pipe, "/tmp/fifo%d", feedlog.pid);
                    cliente_fd = open(cliente_pipe, O_WRONLY);
                    if (cliente_fd == -1) {
                        perror("Erro ao abrir o pipe do cliente para escrita");
                    } 
                    
                    if (res == 0) {  // Novo login
                        env.resposta.flag = 0;
                        snprintf(env.resposta.conteudo, TAM_MSG, "Olá '%s', foste logado com sucesso.", feedlog.nome);
                    } else if (res == -1) {  // nome utilizador invalido
                        env.resposta.flag = -1; // Sinal para encerrar o cliente
                        snprintf(env.resposta.conteudo, TAM_MSG, "ERRO: Nome de utilizador '%s' já está em uso.", feedlog.nome);
                    } else if (res == -2){
                        env.resposta.flag = -1;
                        snprintf(env.resposta.conteudo, TAM_MSG, "ERRO: Limite maximo de utilizadores atingido.");
                    }
                    
                    // enviar resposta ao cliente
                    env.tipo = 2;
                    write(cliente_fd, &env, sizeof(Envia4));
                    close(cliente_fd);
                }
                break;
            case 1:
                // Limpar estrutura `recv` antes de ler uma nova mensagem
                memset(&recv, 0, sizeof(FeedData));

                if (read(dt->fd_server_fifo, &recv, sizeof(FeedData)) > 0){

                    // Construir caminho do FIFO do cliente
                    sprintf(cliente_pipe, "/tmp/fifo%d", recv.pid);
                    cliente_fd = open(cliente_pipe, O_WRONLY);
                    if (cliente_fd == -1) {
                        perror("Erro ao abrir o pipe do cliente para escrita");
                    } else {
                        validacomandosF(cliente_fd, &recv, &env_topicos, &env, dt);
                        close(cliente_fd);
                    }
                }
            default:
                break;
            }
        }
        
    }
    close(dt->fd_server_fifo);
    //printf("Thread de temporizador encerrada.\n");
    pthread_exit(NULL);
}








// Função para a thread de processamento de comandos
void *thread_tempo(void *args) {
    Data * dt = (Data*)args;

    //printf("Thread do timmer\n");

    do {
        sleep(1);

        pthread_mutex_lock(dt->mutex);
        for (int t = 0; t < MAX_TOPICOS; t++) {
            for (int i = 0; i < MAX_TOPICOS_PERMANENTES; i++) {
                if (dt->topico[t].msg[i].tempodevida > 0) {
                    dt->topico[t].msg[i].tempodevida--;

                    if (dt->topico[t].msg[i].tempodevida == 0) {
                        printf("\r[INFO]: Mensagem expirada e removida do tópico '%s': %s\n", 
                               dt->topico[t].titulo, dt->topico[t].msg[i].conteudo);
                        imprime_prompt();

                        memset(&dt->topico[t].msg[i], 0, sizeof(Mensagem));

                        // Atualizar contador de mensagens permanentes
                        if (dt->topico[t].npermanentes > 0) {
                            dt->topico[t].npermanentes--;
                        }

                    }
                }
            }
        }
        pthread_mutex_unlock(dt->mutex);

    } while (dt->trinco != 1);

    //printf("Thread de temporizador encerrada.\n");
    pthread_exit(NULL);
}