#ifndef UTILS_H
#define UTILS_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Limites conforme enunciado
#define MAX_UTILIZADORES 30
#define MAX_VEICULOS 10
#define MAX_SERVICOS 100

// Tamanhos de buffers
#define TAM_NOME 50
#define TAM_LOCAL 100
#define TAM_COMANDO 256
#define TAM_BUFFER 512

// Named pipes
#define FIFO_CONTROLADOR "/tmp/fifo_controlador"
#define FIFO_CLIENTE "/tmp/fifo_cliente_%d"

// Estados do serviço
typedef enum {
    SERVICO_AGENDADO = 0,
    SERVICO_EM_EXECUCAO = 1,
    SERVICO_CONCLUIDO = 2,
    SERVICO_CANCELADO = 3
} EstadoServico;

// Tipos de mensagens entre cliente e controlador
typedef enum {
    MSG_LOGIN = 1,
    MSG_AGENDAR = 2,
    MSG_CANCELAR = 3,
    MSG_CONSULTAR = 4,
    MSG_TERMINAR = 5,
    MSG_RESPOSTA = 10,
    MSG_NOTIFICACAO = 11
} TipoMensagem;

// Estrutura de comunicação cliente<->controlador
typedef struct {
    TipoMensagem tipo;
    char username[TAM_NOME];
    int pid_cliente;
    
    // Campos para agendar
    int hora_agendada;
    char local_partida[TAM_LOCAL];
    int distancia_km;
    
    // Campos para cancelar
    int id_servico;
    
    // Resposta
    int sucesso;
    char mensagem[TAM_BUFFER];
} MensagemCliente;

// Estrutura de comunicação veiculo->cliente
typedef struct {
    int tipo; // 1=chegada, 2=iniciar, 3=cancelado
    char destino[TAM_LOCAL];
    char mensagem[TAM_BUFFER];
} MensagemVeiculo;

// Estrutura de um serviço de transporte
typedef struct {
    int id;
    char username[TAM_NOME];
    int pid_cliente;
    int hora_agendada;
    char local_partida[TAM_LOCAL];
    char local_destino[TAM_LOCAL];
    int distancia_km;
    EstadoServico estado;
    int pid_veiculo;
    int percentagem_percorrida;
} Servico;

// Estrutura de um cliente conectado
typedef struct {
    char username[TAM_NOME];
    int pid;
    int ativo;
    int em_viagem;
} Cliente;

// Estrutura de um veículo em operação
typedef struct {
    int pid;
    int id_servico;
    int percentagem_percorrida;
    int ativo;
} Veiculo;

// Estrutura principal do controlador
typedef struct {
    Cliente clientes[MAX_UTILIZADORES];
    int num_clientes;
    
    Servico servicos[MAX_SERVICOS];
    int num_servicos;
    int proximo_id_servico;
    
    Veiculo veiculos[MAX_VEICULOS];
    int max_veiculos;
    int veiculos_disponiveis;
    
    int tempo_atual;
    int total_km_percorridos;
    int terminar;
    
    pthread_mutex_t mutex;
    int fd_controlador;
} Controlador;

// Funções auxiliares
int cliente_existe(Controlador *ctrl, const char *username);
int adicionar_cliente(Controlador *ctrl, const char *username, int pid);
int remover_cliente(Controlador *ctrl, const char *username);
Cliente* buscar_cliente(Controlador *ctrl, const char *username);

int adicionar_servico(Controlador *ctrl, const char *username, int pid, int hora, 
                      const char *local, int distancia);
Servico* buscar_servico(Controlador *ctrl, int id);
int cancelar_servico(Controlador *ctrl, int id, const char *username);
void listar_servicos_usuario(Controlador *ctrl, const char *username, char *resultado);

int alocar_veiculo(Controlador *ctrl);
void liberar_veiculo(Controlador *ctrl, int pid_veiculo);
void processar_telemetria(Controlador *ctrl, int pid_veiculo, const char *info);

void inicializar_controlador(Controlador *ctrl);
void verificar_servicos_agendados(Controlador *ctrl);

#endif
