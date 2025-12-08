#include "utils.h"


int cria_topico( const char *nome, Data *dt, const char *nome_user) {
    // Verifica se já existe um tópico com o mesmo nome
    
    for (int i = 0; i < MAX_TOPICOS; i++) {
        if (strcmp(dt->topico[i].titulo, nome) == 0) {
            for (int r = 0; r < MAX_UTILIZADORES; r++){
                if(strcmp(dt->topico[i].subs[r].nome, nome_user)==0){
                    return -1;
                }
            }
           
            strcpy(dt->topico[i].subs[dt->topico[i].numsub].nome ,nome_user);
            dt->topico[i].numsub++;

            return 0;  
        }
    }
 
    // Procura uma posição vazia na lista de tópicos
    for (int i = 0; i < MAX_TOPICOS; i++) {
        if (dt->topico[i].titulo[0] == '\0') {  // Verifica se o título está vazio
            // Inicializa o novo tópico
            strncpy(dt->topico[i].titulo, nome, TAM_TOPICO - 1);
            dt->topico[i].titulo[TAM_TOPICO - 1] = '\0'; // Garante terminação em null
            dt->topico[i].npermanentes = 0;
            strcpy(dt->topico[i].subs[dt->topico[i].numsub].nome ,nome_user);
            dt->topico[i].numsub++;
            
            // Limpa mensagens do tópico
            for (int j = 0; j < 5; j++) {
                dt->topico[i].msg[j].conteudo[0] = '\0';
                dt->topico[i].msg[j].tempodevida = 0;
            }

          
            return 0;
        }
    }

    // Se chegou aqui, não há espaço para mais tópicos
    printf("\r[ERRO]: limite máximo de tópicos atingido.\n");
    imprime_prompt();
    return -2;
}

int unsubscribe_topico(const char *nome, Data *dt, const char *nome_user){

 // Verifica se já existe um tópico com o mesmo nome
    
    for (int i = 0; i < MAX_TOPICOS; i++) {
        if (strcmp(dt->topico[i].titulo, nome) == 0) {
            for (int r = 0; r < MAX_UTILIZADORES; r++){
                if(strcmp(dt->topico[i].subs[r].nome,nome_user)==0){
                    strcpy(dt->topico[i].subs[dt->topico[i].numsub].nome ,"");
                    dt->topico[i].numsub--;

                    if(dt->topico[i].numsub == 0 && dt->topico[i].npermanentes==0){
                        strcpy(dt->topico[i].titulo,""); 
                    }
                    return 0;
                }
            }
           return 1;  
        }
    }
    return -1;
}

int mostra_topicsF(Data *dt, Envia3 *tm, Envia4 *de) {
    tm->tipo = 3;
    int n_topicos = 0;


    // Limpa o conteúdo de `de` para evitar resíduos de dados antigos
    memset(tm->lista.top, 0, sizeof(tm->lista.top));
    tm->lista.num_Topicos = 0;

    for (int i = 0; i < MAX_TOPICOS; i++) {
        if (dt->topico[i].titulo[0] != '\0') { // Apenas tópicos válidos
            strcpy(tm->lista.top[n_topicos].titulo, dt->topico[i].titulo);
            tm->lista.top[n_topicos].nperm = dt->topico[i].npermanentes;
            n_topicos++;
        }
    }

    tm->lista.num_Topicos = n_topicos;

    if (n_topicos == 0) {
        snprintf(de->resposta.conteudo, 300, "Ainda nao existem tópicos!");
        return 1; // nao ha topicos
    } 
    return 0;
}

void mostra_topicsM(Data * dt){
    printf("\n[INFO]: Mostra os topicos ativos\n");
    int n_topicos = 0;
    for (int i = 0; i < MAX_TOPICOS; i++){
        if(dt->topico[i].titulo[0] != '\0') {
            printf( "> Topico: '%s'\n - Mensagens Persistentes: '%d' \n",dt->topico[i].titulo, dt->topico[i].npermanentes);
            n_topicos++;
        }
    }
    if (n_topicos == 0){
        printf(" > Nao existem topicos\n");
    }
    imprime_prompt();
}

void bloqueiaTopico(Data *dt, const char *nome) {
    int encontrado = 0;

    pthread_mutex_lock(dt->mutex); // Proteger o acesso à estrutura compartilhada

    for (int i = 0; i < MAX_TOPICOS; i++) {
        if (strcmp(dt->topico[i].titulo, nome) == 0) {
            dt->topico[i].bloqueio = 1; // Bloquear o tópico
            encontrado = 1;
            break; // Interrompe o loop após encontrar o tópico
        }
    }

    pthread_mutex_unlock(dt->mutex);

    if (encontrado) {
        printf("\n[INFO]: O tópico '%s' foi bloqueado com sucesso!\n", nome);
    } else {
        printf("\n[ERRO]: Tópico '%s' não encontrado. Não foi possível bloqueá-lo.\n", nome);
    }

    imprime_prompt();
}


void desbloqueiaTopico(Data *dt, const char *nome) {
    int encontrado = 0;

    pthread_mutex_lock(dt->mutex); // Protege o acesso à estrutura compartilhada

    for (int i = 0; i < MAX_TOPICOS; i++) {
        if (strcmp(dt->topico[i].titulo, nome) == 0) {
            dt->topico[i].bloqueio = 0; // Desbloqueia o tópico
            encontrado = 1;
            break; // Interrompe o loop após encontrar o tópico
        }
    }

    pthread_mutex_unlock(dt->mutex);

    if (encontrado) {
        printf("\n[INFO]: O tópico '%s' foi desbloqueado com sucesso!\n", nome);
    } else {
        printf("\n[ERRO]: Tópico '%s' não encontrado. Não foi possível desbloqueá-lo.\n", nome);
    }

    imprime_prompt();
}


int adiciona_Msg(const char *nome, const char *msg, Data *dt, int tempo, const char *user) {
    pthread_mutex_lock(dt->mutex);

    for (int t = 0; t < MAX_TOPICOS; t++) {
        if (strcmp(dt->topico[t].titulo, nome) == 0) {

            int subscritor = 0;
            for (int i = 0; i < dt->topico[t].numsub; i++) {
                if (strcmp(dt->topico[t].subs[i].nome, user) == 0) {
                    subscritor = 1;
                    break;
                }
            }
            if (!subscritor) {
                pthread_mutex_unlock(dt->mutex);
                return 4; // Não é subscritor
            }

            if (dt->topico[t].bloqueio == 1) {
                pthread_mutex_unlock(dt->mutex);
                return 3; // Tópico bloqueado
            }

            if (tempo > 0) { // Mensagem persistente
                // Verificar se há espaço para mensagens persistentes antes de enviar
                int espaco_disponivel = 0;
                for (int i = 0; i < MAX_TOPICOS_PERMANENTES; i++) {
                    if (dt->topico[t].msg[i].tempodevida == 0) {
                        espaco_disponivel = 1;
                        break;
                    }
                }

                if (!espaco_disponivel) {
                    pthread_mutex_unlock(dt->mutex);
                    return 2; // Sem espaço para mensagens persistentes
                }

                // Envia mensagem para os subscritores
                envia_mensagem_para_subscritores(dt, nome, msg, user);

                // Armazenar como mensagem persistente
                for (int i = 0; i < MAX_TOPICOS_PERMANENTES; i++) {
                    if (dt->topico[t].msg[i].tempodevida == 0) {
                        strcpy(dt->topico[t].msg[i].conteudo, msg);
                        strcpy(dt->topico[t].msg[i].quem_enviou, user);
                        dt->topico[t].msg[i].tempodevida = tempo;

                        dt->topico[t].npermanentes++;
                        
                        pthread_mutex_unlock(dt->mutex);
                        return 1; // Sucesso
                    }
                }
            } else {
                // Mensagem não persistente
                envia_mensagem_para_subscritores(dt, nome, msg, user);
                pthread_mutex_unlock(dt->mutex);
                return 1;
            }
        }
    }
    pthread_mutex_unlock(dt->mutex);
    return 0; // Tópico não encontrado
}


void sh_Topico(Data * dt , const char *nome ){
    int encontrado = 0;

    for (int i = 0; i < MAX_TOPICOS; i++) {
        if (strcmp(dt->topico[i].titulo, nome)==0) {
            encontrado = 1;
            if(dt->topico[i].npermanentes != 0){
                for (int j = 0; j < dt->topico[i].npermanentes; j++) {
                    printf("-> %s\n",dt->topico[i].msg[j].conteudo);
                }
                
            }
            else{
                printf("\n[INFO]: Este topico nao tem mensagens de momento\n");
            }
            break;

        }
    }

    if (!encontrado){
        printf("\n[ERRO]: Topico nao encontrado\n");
    }
    imprime_prompt();
}



void encerrar_feed(Data *data, int pid, const char *mensagem) {
    int cliente_fd, i;
    char nome_user[TAM_NOME];
    Envia4 msg;
    msg.tipo = 2;

    for (i = 0; i < MAX_UTILIZADORES; i++) {
        if (data->user[i].pid == pid) {
            // Enviar mensagem de encerramento ao feed
            cliente_fd = open(data->user[i].fifo, O_WRONLY);
            if (cliente_fd > 0) {
                strcpy(nome_user, data->user[i].nome);
                msg.resposta.flag = -1; // Código para encerrar o feed
                snprintf(msg.resposta.conteudo, sizeof(msg.resposta.conteudo), "%s", mensagem);
                write(cliente_fd, &msg, sizeof(Envia4));
            } else {
                perror("[ERRO]: Não foi possível abrir o pipe do cliente.");
            }

            // Limpar a estrutura do utilizador
            pthread_mutex_lock(data->mutex);
            memset(&data->user[i], 0, sizeof(Utilizador));
            pthread_mutex_unlock(data->mutex);

            printf("\r[INFO]: Feed do utilizador na posição %d foi encerrado.\n", i);
            imprime_prompt();
            break;
        } 
    }
    for (int u = 0; u < MAX_UTILIZADORES; u++) {
        if (data->user[u].estado == 1 && u != i) {
            //snprintf(cliente_pipe, TAM_FIFO, Client_Pipe, dt->user[i].pid);
            cliente_fd = open(data->user[u].fifo, O_WRONLY);
            if (cliente_fd != -1) {
                memset(&msg, 0, sizeof(msg.resposta)); // Limpa a estrutura para nova mensagem
                msg.tipo = 2;
                msg.resposta.flag = 0; // Mensagem genérica para outros feeds
                snprintf(msg.resposta.conteudo, 100, "O Utilizador '%s' saiu da plataforma.", nome_user);
                write(cliente_fd, &msg, sizeof(Envia4));
            }
        }
    }
}


int cria_txt(Data *dt){

    char *nomeFicheiro = getenv("MSG_FICH");

    if (nomeFicheiro == NULL) {
        printf("Erro: Variável de ambiente MSG_FICH não definida.\n");
        return -1;
    }

        // Abrir o ficheiro para escrita
    FILE *ficheiro = fopen(nomeFicheiro, "w");

    if (ficheiro == NULL) {
        printf("Erro ao abrir o ficheiro");
        return -1;
    }

    //For para iterar os topicos
    pthread_mutex_lock(dt->mutex);
    for (int i = 0; i < MAX_TOPICOS; i++)
    {
        //iterar nas mensagens permanentes
        for (int j = 0; j < MAX_TOPICOS_PERMANENTES; j++)
        {
            if (dt->topico[i].msg[j].tempodevida!=0)
            {
                fprintf(ficheiro, "%s %s %d %s\n",dt->topico[i].titulo,dt->topico[i].msg[j].quem_enviou,dt->topico[i].msg[j].tempodevida,dt->topico[i].msg[j].conteudo);
            }

        }
        
    }
    pthread_mutex_unlock(dt->mutex);
    // Fechar o ficheiro
    fclose(ficheiro);
    return 0;


}

int le_txt(Data *dt) {
    char *nomeFicheiro = getenv("MSG_FICH");
   
    if (nomeFicheiro == NULL) {
        printf("Erro: Variável de ambiente MSG_FICH não definida.\n");
        return -1;
    }

    // Abrir o ficheiro para leitura
    FILE *ficheiro = fopen(nomeFicheiro, "r");
    if (ficheiro == NULL) {
        printf("Erro ao abrir o ficheiro para leitura.\n");
        return -1;
    }
      
    char linha[1000];

    while (fgets(linha, sizeof(linha), ficheiro)) {
        char titulo[TAM_TOPICO];
        char quem_enviou[TAM_NOME];
        int tempodevida;
        char conteudo[TAM_MSG];
          
        // Lê a linha e extrai os valores
        if (sscanf(linha, "%s %s %d %[^\n]", titulo, quem_enviou, &tempodevida, conteudo) == 4) {
            // Verifica se o tópico já existe
            int topicoIndex = -1;
            for (int i = 0; i < MAX_TOPICOS; i++) {
                if (strcmp(dt->topico[i].titulo, titulo) == 0) {
                    topicoIndex = i;
                    break;
                }
            }
              
            // Se o tópico não existe, cria um novo
            if (topicoIndex == -1) {
                for (int i = 0; i < MAX_TOPICOS; i++) {
                    if (dt->topico[i].titulo[0] == '\0') { // Tópico vazio
                        strncpy(dt->topico[i].titulo, titulo, TAM_TOPICO - 1);
                        dt->topico[i].titulo[TAM_TOPICO - 1] = '\0';
                        topicoIndex = i;
                        break;
                    }
                }
            }

            if (topicoIndex != -1) {
                // Adiciona a mensagem ao tópico encontrado/criado
                for (int j = 0; j < MAX_TOPICOS_PERMANENTES; j++) {
                    if (dt->topico[topicoIndex].msg[j].tempodevida == 0) { // Posição livre
                        strncpy(dt->topico[topicoIndex].msg[j].conteudo, conteudo, TAM_MSG - 1);
                        dt->topico[topicoIndex].msg[j].conteudo[TAM_MSG - 1] = '\0';

                        strncpy(dt->topico[topicoIndex].msg[j].quem_enviou, quem_enviou, TAM_NOME - 1);
                        dt->topico[topicoIndex].msg[j].quem_enviou[TAM_NOME - 1] = '\0';
                          
                        dt->topico[topicoIndex].msg[j].tempodevida = tempodevida;
                        dt->topico[topicoIndex].npermanentes++;
                        break;
                    }
                }
            } else {
                printf("[ERRO]: Limite máximo de tópicos atingido. Não foi possível adicionar o tópico '%s'.\n", titulo);
            }
        } else {
            printf("[ERRO]: Linha no formato incorreto: %s\n", linha);
        }
    }

   
    fclose(ficheiro);

    return 0;
}

