#include "utils.h"

void imprime_prompt(){
    printf("\nManager > ");
    fflush(stdout);
}

// limpa estruturas no incio
void preparacao(Data *d){

    // Inicializar utilizadores
    for (int i = 0; i < MAX_UTILIZADORES; i++) {
        memset(&d->user[i], 0, sizeof(Utilizador)); 
        d->user[i].pid = 0; 
        d->user[i].estado = 0; 
    }

    // Inicializar tópicos
    for (int y = 0; y < MAX_TOPICOS; y++) {
        memset(&d->topico[y], 0, sizeof(Topico)); 
        d->topico[y].numsub = 0; 
        d->topico[y].npermanentes = 0; 
        d->topico[y].bloqueio = 0;

        // Inicializar mensagens dentro do tópico
        for (int s = 0; s < MAX_TOPICOS_PERMANENTES; s++) {
            d->topico[y].msg[s].tempodevida = 0; 
            memset(d->topico[y].msg[s].conteudo, 0, sizeof(d->topico[y].msg[s].conteudo)); 
            memset(d->topico[y].msg[s].quem_enviou, 0, sizeof(d->topico[y].msg[s].quem_enviou)); 
        }
    }
    
    //Testa se ha e se nao houver cia a variavel ambiente

    char* var_nome = "MSG_FICH"; // Nome da variável de ambiente
    char* valor_padrao = "memoria.txt"; // Valor padrão, caso seja necessário criar
    
    // Verifica se a variável de ambiente existe
    char* valor = getenv(var_nome);
    if (valor) {
        // A variável existe, imprime o valor atual
        printf("A variável de ambiente '%s' existe com o valor: %s\n", var_nome, valor);
        
    } else {
        // A variável não existe
        printf("A variável de ambiente '%s' não existe.\n", var_nome);
        if (setenv(var_nome, valor_padrao, 1) != 0) {
            perror("Erro ao criar a variável de ambiente");
            return;
        }
        printf("A variável de ambiente '%s' foi criada.\n", var_nome);
    }
    
    // Inicializar mutex para controle de threads
    d->trinco = 0; 
    le_txt(d);
    
    
}





void validacomandosF(int cliente_fd, FeedData *dt, Envia3 *tm, Envia4 *de, Data *data) {
    
    char command[50];
    char argument1[50];
    char mensagem[TAM_MSG];
    int duracao;

    // Divide o comando e os argumentos conforme a lógica
    int numArgs = sscanf(dt->conteudo, "%s %s %d %[^\n]", command, argument1, &duracao, mensagem);

    // Limpando a estrutura de resposta
    memset(de, 0, sizeof(Envia4));
    de->tipo = 2;
    de->resposta.flag = 0;

    // Verifica se o comando está vazio
    if (numArgs <= 0 || strlen(command) == 0) {
        snprintf(de->resposta.conteudo, 100, "[ERRO]: Comando não reconhecido. Tente novamente.");
        write(cliente_fd, de, sizeof(Envia4));
        return;
    }

    // Verifica os comandos
    if (strcmp(command, "topics") == 0) {
        if (numArgs == 1) {
            int res = mostra_topicsF(data, tm, de);
            if (res == 0){
                tm->tipo = 3;
                write(cliente_fd, tm, sizeof(Envia3));
            } else {
                de->tipo = 2;
                write(cliente_fd, de, sizeof(Envia4));
            }
            
        } else {
            snprintf(de->resposta.conteudo, TAM_MSG, "[ERRO]: Sintaxe inválida para o comando 'topics'.");
            write(cliente_fd, de, sizeof(Envia4));
        }
    } else if (strcmp(command, "msg") == 0) {
        //int duracao_int = atoi(duracao);
        if (numArgs == 4 && duracao >= 0) {
            int resultado = adiciona_Msg(argument1, mensagem, data, duracao, dt->nome);

            if (resultado == 1) {
                snprintf(de->resposta.conteudo, TAM_MSG, "Mensagem enviada/adicionada ao tópico '%s' com sucesso.", argument1);
            } else if (resultado == 2) {
                snprintf(de->resposta.conteudo, TAM_MSG, "[ERRO]-> Sem espaço para novas mensagens persistentes no tópico '%s'.", argument1);
            } else if (resultado == 3) {
                snprintf(de->resposta.conteudo, TAM_MSG, "[ERRO]-> Tópico '%s' está bloqueado. Mensagem não enviada.", argument1);
            } else if (resultado == 4){
                snprintf(de->resposta.conteudo, TAM_MSG, "[ERRO]-> Apenas subscritores podem enviar mensagens ao tópico '%s'.", argument1);
            } else {
                snprintf(de->resposta.conteudo, TAM_MSG, "[ERRO]-> Tópico '%s' não encontrado.", argument1);
            }
        } else {
            snprintf(de->resposta.conteudo, TAM_MSG, "[ERRO]-> Sintaxe inválida para o comando 'msg'. Exemplo: msg <topico> <duracao> <mensagem>");
        }
        write(cliente_fd, de, sizeof(Envia4));

    } else if (strcmp(command, "subscribe") == 0) {
        if (numArgs == 2) {
            
            for (int i = 0; i < MAX_UTILIZADORES; i++) {
                if(dt->pid == data->user[i].pid){
                    int g = cria_topico(argument1, data, data->user[i].nome);
                    if(g==0){
                        snprintf(de->resposta.conteudo, TAM_MSG, "Tópico '%s' subscrito com sucesso!", argument1);
                        write(cliente_fd, de, sizeof(Envia4));
                        // Chamar função para enviar mensagens persistentes
                        envia_mensagens_persistentes(data, argument1, data->user[i].nome, cliente_fd);
                    } else if (g==-1) {
                        snprintf(de->resposta.conteudo, TAM_MSG, "Ja esta subscrito no tópico '%s'...", argument1);
                        write(cliente_fd, de, sizeof(Envia4));
                    } else if (g==-2){
                        snprintf(de->resposta.conteudo, TAM_MSG, "Nº máximo de tópicos já atingido, não foi possível criar novo!", argument1);
                        write(cliente_fd, de, sizeof(Envia4));
                    }
                    break;
                } 
            }
            //write(cliente_fd, de, sizeof(Envia4));
        } else {
            snprintf(de->resposta.conteudo, TAM_MSG, "[ERRO]: Sintaxe inválida para o comando 'subscribe'. Exemplo: subscribe <topico>");
            write(cliente_fd, de, sizeof(Envia4));
        }
    } else if (strcmp(command, "unsubscribe") == 0) {
        if (numArgs == 2) {
            for (int i = 0; i < MAX_UTILIZADORES; i++){
                if(dt->pid == data->user[i].pid){
                    int g = unsubscribe_topico(argument1,data,data->user[i].nome);
                    if(g==0){
                        snprintf(de->resposta.conteudo, TAM_MSG, "Sucesso ao remover o utilizador do topico '%s'...", argument1);
                    }else if (g==1){
                        snprintf(de->resposta.conteudo, TAM_MSG, "Utilizador nao esta subsctito no topico '%s'...", argument1);
                    }else if (g==-1){
                        snprintf(de->resposta.conteudo, TAM_MSG, "O Topico '%s' nao existe", argument1);
                    }
                    break;
                } 
            }
            write(cliente_fd, de, sizeof(Envia4));
            
        } else {
            snprintf(de->resposta.conteudo, TAM_MSG, "[ERRO]: Sintaxe inválida para o comando 'unsubscribe'. Exemplo: unsubscribe <topico>");
            write(cliente_fd, de, sizeof(Envia4));
        }
    } else if (strcmp(command, "exit") == 0) {
        if (numArgs == 1) {
            encerrar_feed(data, dt->pid, "Sessão encerrada. Adeus!");
        } else {
            snprintf(de->resposta.conteudo, sizeof(de->resposta.conteudo), "[ERRO]: Sintaxe inválida para o comando 'exit'. Nenhum argumento é necessário.");
            write(cliente_fd, de, sizeof(Envia4));
        }
    

    } else {
        snprintf(de->resposta.conteudo, TAM_MSG, "[ERRO]: Comando inválido. Tente novamente.");
        write(cliente_fd, de, sizeof(Envia4));
    }
}

//adiciona user
int processa_mensagem(Data *data, Login *dt) {

    if(dt->flag == 0){

        for (int i = 0; i < MAX_UTILIZADORES; i++){
            if (strcmp(data->user[i].nome, dt->nome) == 0){
                printf("\r[ERRO]: Nome de utilizador '%s' ja usado.\n", dt->nome);
                imprime_prompt();
                return -1; // erro nome
            }
        }

        // Encontra uma posição vazia no array `user`
        for ( int i = 0; i < MAX_UTILIZADORES; i++) {
            if (data->user[i].pid == 0) {  // Assumindo que pid = 0 indica uma posição vazia
                // username
                strncpy(data->user[i].nome,dt->nome, TAM_NOME - 1);
                data->user[i].nome[TAM_NOME - 1] = '\0'; // Garante terminação da string
                // pid cliente
                data->user[i].pid = dt->pid;
                data->user[i].estado = 1;

                char cliente_pipe[TAM_FIFO];
                snprintf(cliente_pipe, TAM_FIFO, Client_Pipe, dt->pid);
                strcpy(data->user[i].fifo, cliente_pipe);

                printf("\r[INFO]: Utilizador '%s' com PID %d adicionado na posição %d.\n", dt->nome, dt->pid, i);
                imprime_prompt();
                return 0; // sucesso
            }
        }

        printf("\r[ERRO]: Limite máximo de utilizadores atingido.\n");
        imprime_prompt();
        return -2;

    } 
    // flag é um entao é para processar comando
    
    return 1; // codigo para processamento de comandos
} 

void mostra_users(Data *dt) {
    printf("\n[INFO]: Lista de utilizadores ativos:\n");

    int encontrado = 0;
    for (int i = 0; i < MAX_UTILIZADORES; i++) {
        if (dt->user[i].estado == 1) { // Verifica se o utilizador está ativo
            printf(" - Nome: %s, PID: %d\n", dt->user[i].nome, dt->user[i].pid);
            encontrado = 1;
        }
    }

    if (!encontrado) {
        printf("\rNenhum utilizador está atualmente ativo.\n");
    }
    
}


void remove_user(Data *dt, const char *username) {
    int indice_user = -1;

    // Procurar pelo utilizador na estrutura
    for (int i = 0; i < MAX_UTILIZADORES; i++) {
        if (dt->user[i].estado == 1 && strcmp(dt->user[i].nome, username) == 0) {
            indice_user = i;
            break;
        }
    }

    if (indice_user == -1) {
        printf("\r[ERRO]: Utilizador '%s' não encontrado.\n", username);
        return;
    }

    int cliente_fd = open(dt->user[indice_user].fifo, O_WRONLY | O_NONBLOCK);
    if (cliente_fd == -1) {
        perror("\r[ERRO]: Não foi possível comunicar com o feed do utilizador a remover");
        return;
    }

    Envia4 msg;
    memset(&msg, 0, sizeof(Envia4));
    msg.tipo = 2;
    msg.resposta.flag = -1;
    snprintf(msg.resposta.conteudo, 100, "Desculpa '%s', foste removido da plataforma.", username);
    write(cliente_fd, &msg, sizeof(Envia4));
    close(cliente_fd);

    // Notificar outros feeds sobre a remoção
    
    for (int i = 0; i < MAX_UTILIZADORES; i++) {
        if (dt->user[i].estado == 1 && i != indice_user) {
            //snprintf(cliente_pipe, TAM_FIFO, Client_Pipe, dt->user[i].pid);
            cliente_fd = open(dt->user[i].fifo, O_WRONLY);
            if (cliente_fd != -1) {
                memset(&msg, 0, sizeof(Envia4)); // Limpa a estrutura para nova mensagem
                msg.tipo = 2;
                msg.resposta.flag = 0; // Mensagem genérica para outros feeds
                snprintf(msg.resposta.conteudo, 100, "O Utilizador '%s' foi removido da plataforma.", username);
                write(cliente_fd, &msg, sizeof(Envia4));
                close(cliente_fd);
            }
        }
    }

    // Libertar o slot do utilizador
    memset(&dt->user[indice_user], 0, sizeof(Utilizador));

    printf("\n[INFO]: Utilizador '%s' removido com sucesso.\n", username);
}

 

 void envia_mensagem_para_subscritores(Data *dt, const char *topico, const char *mensagem, const char *user) {

    // Percorre os tópicos para encontrar o tópico correto
    for (int t = 0; t < MAX_TOPICOS; t++) {
        if (strcmp(dt->topico[t].titulo, topico) == 0) {
            // Envia a mensagem para todos os subscritores
            for (int i = 0; i < dt->topico[t].numsub; i++) {
                for (int u = 0; u < MAX_UTILIZADORES; u++) {
                    if (strcmp(dt->topico[t].subs[i].nome, dt->user[u].nome) == 0 && strcmp(dt->user[u].nome, user) != 0) {
                        int cliente_fd = open(dt->user[u].fifo, O_WRONLY);
                        if (cliente_fd != -1) {
                            Envia4 de;
                            de.tipo = 2;
                            de.resposta.flag = 1;
                            snprintf(de.resposta.conteudo, sizeof(de.resposta.conteudo), "[%s (%s)]: %s", topico, user, mensagem);
                            write(cliente_fd, &de, sizeof(Envia4));
                            close(cliente_fd);
                        } else {
                            perror("[ERRO]: Não foi possível enviar a mensagem ao subscritor.");
                        }
                    }
                }
            }
            break;
        }
    }
}


void envia_mensagens_persistentes(Data *dt, const char *topico, const char *nome_user, int cliente_fd) {
    for (int t = 0; t < MAX_TOPICOS; t++) {
        if (strcmp(dt->topico[t].titulo, topico) == 0) { // Encontra o tópico
            if (dt->topico[t].npermanentes > 0){ //verifica se ha persistentes
                Envia4 de;
                de.tipo = 2;
                de.resposta.flag = 1;

                // Mensagem inicial indicando que existem mensagens
                snprintf(de.resposta.conteudo, TAM_MSG, "\r- Mensagens persistentes no tópico '%s':", topico);
                write(cliente_fd, &de, sizeof(Envia4));

                // Envia cada mensagem persistente separadamente
                for (int j = 0; j < MAX_TOPICOS_PERMANENTES; j++) {
                    if (dt->topico[t].msg[j].tempodevida > 0) { // Verifica mensagens válidas
                        snprintf(de.resposta.conteudo, TAM_MSG, "  - [%s] %s", dt->topico[t].msg[j].quem_enviou,      dt->topico[t].msg[j].conteudo);

                        write(cliente_fd, &de, sizeof(Envia4));
                    }
                }
            }

            break; // Tópico encontrado, não é necessário continuar
        }
    }
}

void encerrar_todos_feeds(Data *data, const char *mensagem) {
    for (int i = 0; i < MAX_UTILIZADORES; i++) {
        if (data->user[i].estado == 1) { // Verifica se o utilizador está ativo
            int cliente_fd = open(data->user[i].fifo, O_WRONLY);
            if (cliente_fd != -1) {
                Envia4 msg;
                memset(&msg, 0, sizeof(Envia4));
                msg.tipo = 2; // Tipo de mensagem para feed
                msg.resposta.flag = -1; // Indica encerramento do feed
                snprintf(msg.resposta.conteudo, sizeof(msg.resposta.conteudo), "%s", mensagem);
                write(cliente_fd, &msg, sizeof(Envia4));
                close(cliente_fd);
            } else {
                perror("[ERRO]: Não foi possível enviar mensagem de encerramento ao feed.");
            }

            // Limpa a estrutura do utilizador
            pthread_mutex_lock(data->mutex);
            memset(&data->user[i], 0, sizeof(Utilizador));
            pthread_mutex_unlock(data->mutex);
        }
    }
}
