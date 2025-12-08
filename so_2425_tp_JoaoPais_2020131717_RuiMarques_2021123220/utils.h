#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>


#define TAM_MSG 300
#define TAM_NOME 50
#define TAM_FIFO 17
#define TAM_TOPICO 20
#define MAX_TOPICOS 20
#define MAX_UTILIZADORES 10
#define MAX_TOPICOS_PERMANENTES 5


#define Client_Pipe "/tmp/fifo%d"
#define FIFO_SERVER "/tmp/fifo_server"





// ESTRUTURAS

typedef struct {
    char conteudo[TAM_MSG];
    int tempodevida;
    char quem_enviou[TAM_NOME];
} Mensagem;


typedef struct {
    char nome[TAM_NOME];
} Subscritores;


typedef struct {
    char titulo[TAM_TOPICO];          // Nome da rota/zona
    Mensagem msg[5];
    int npermanentes;  
    Subscritores subs[MAX_UTILIZADORES];
    int numsub;
    int bloqueio;    // 0 rota aberta, 1 rota fechada/bloqueada         
} Topico;

 
typedef struct {
    char nome[TAM_NOME];  // Identificador do veículo/motorista
    char fifo[TAM_FIFO];
    int pid;     
    int estado; // 1 = veículo ativo na frota, 0 = inativo   
} Utilizador;


typedef struct {
    Topico topico[MAX_TOPICOS];
    Utilizador user[MAX_UTILIZADORES];
    int fd_server_fifo;
    pthread_mutex_t * mutex;
    int trinco;
} Data;


typedef struct {
    char titulo[TAM_TOPICO];  
    int nperm;           
} TopicosF; 


typedef struct {
    int flag;
    int pid;
    char nome[TAM_NOME];
} Login;


typedef struct {
    int pid;
    char nome[TAM_NOME];
    char conteudo[300];
} FeedData;


typedef struct {
    int flag;
    char conteudo[300];
} Mresposta;


typedef struct {
    int num_Topicos;
    TopicosF top[MAX_TOPICOS];
} MSGTopicos;


// ESTRUTURAS - envelopes
typedef struct {
    int tipo;
    Login login;
} Envia1;

typedef struct {
    int tipo;
    FeedData msg;
} Envia2;

typedef struct {
    int tipo;
    MSGTopicos lista;
} Envia3;

typedef struct {
    int tipo;
    Mresposta resposta;
} Envia4;




// Definições de funções

// utils.c
void imprime_prompt();
void preparacao(Data *d);
void mostra_users(Data *dt);
void remove_user(Data *dt, const char *username);
void envia_mensagem_para_subscritores(Data *dt, const char *topico, const char *mensagem, const char *user);
void envia_mensagens_persistentes(Data *dt, const char *topico, const char *nome_user, int cliente_fd);
void encerrar_todos_feeds(Data *data, const char *mensagem);
void validacomandosF(int cliente_fd, FeedData *dt, Envia3 *tm, Envia4 *de, Data *data);


// operacoesmemoria.c
int processa_mensagem(Data *data, Login *dt);
int cria_topico(const char *nome, Data *dt, const char *nome_user);
int unsubscribe_topico(const char *nome, Data *dt, const char *nome_user);
int mostra_topicsF(Data *dt, Envia3 *tm, Envia4 *de);
void mostra_topicsM(Data *dt);
void bloqueiaTopico(Data *dt, const char *nome);
void desbloqueiaTopico(Data *dt, const char *nome);
int adiciona_Msg(const char *nome, const char *msg, Data *dt, int tempo, const char *user);
void sh_Topico(Data *dt, const char *nome);
void encerrar_feed(Data *data, int pid, const char *mensagem);
int cria_txt(Data *dt);
int le_txt(Data *dt);


// threads.c
void *thread_comunicacao(void *args);
void *thread_tempo(void *args);


#endif // UTILS_H
