#include "utils.h"

Controlador ctrl;

void *thread_tempo(void *arg) {
    while (!ctrl.terminar) {
        sleep(1);
        pthread_mutex_lock(&ctrl.mutex);
        ctrl.tempo_atual++;
        pthread_mutex_unlock(&ctrl.mutex);
        
        // verificar_servicos_agendados já faz seu próprio locking
        verificar_servicos_agendados(&ctrl);
    }
    return NULL;
}

void *thread_telemetria(void *arg) {
    while (!ctrl.terminar) {
        pthread_mutex_lock(&ctrl.mutex);
        for (int i = 0; i < ctrl.max_veiculos; i++) {
            if (ctrl.veiculos[i].ativo && ctrl.veiculos[i].pid > 0) {
                // Verificar se processo ainda está ativo
                int status;
                pid_t result = waitpid(ctrl.veiculos[i].pid, &status, WNOHANG);
                
                if (result > 0) {
                    // Processo terminou - recolher zombie
                    Servico *s = buscar_servico(&ctrl, ctrl.veiculos[i].id_servico);
                    if (s != NULL) {
                        if (s->estado == SERVICO_EM_EXECUCAO) {
                            s->estado = SERVICO_CONCLUIDO;
                            ctrl.total_km_percorridos += s->distancia_km;
                        }
                        
                        Cliente *c = buscar_cliente(&ctrl, s->username);
                        if (c) c->em_viagem = 0;
                    }
                    
                    ctrl.veiculos[i].ativo = 0;
                    ctrl.veiculos[i].pid = 0;
                    ctrl.veiculos_disponiveis++;
                    
                    printf("[TELEMETRIA] Veículo (PID %d) terminado e zombie recolhido\n", result);
                }
            }
        }
        pthread_mutex_unlock(&ctrl.mutex);
        sleep(1);
    }
    return NULL;
}

void *thread_comunicacao(void *arg) {
    MensagemCliente msg;
    char fifo_cliente[50];
    
    while (!ctrl.terminar) {
        int n = read(ctrl.fd_controlador, &msg, sizeof(MensagemCliente));
        if (n <= 0) continue;
        
        MensagemCliente resposta;
        memset(&resposta, 0, sizeof(MensagemCliente));
        resposta.tipo = MSG_RESPOSTA;
        resposta.pid_cliente = msg.pid_cliente;
        
        sprintf(fifo_cliente, FIFO_CLIENTE, msg.pid_cliente);
        int fd_cliente = open(fifo_cliente, O_WRONLY | O_NONBLOCK);
        
        if (fd_cliente == -1) {
            continue; // Cliente desconectou
        }
        
        switch (msg.tipo) {
            case MSG_LOGIN:
                if (adicionar_cliente(&ctrl, msg.username, msg.pid_cliente) == 0) {
                    resposta.sucesso = 1;
                    sprintf(resposta.mensagem, "Bem-vindo %s!", msg.username);
                    printf("[LOGIN] Cliente %s conectado (PID %d)\n", msg.username, msg.pid_cliente);
                } else {
                    resposta.sucesso = 0;
                    if (cliente_existe(&ctrl, msg.username)) {
                        strcpy(resposta.mensagem, "Username já em uso!");
                    } else {
                        strcpy(resposta.mensagem, "Limite de utilizadores atingido!");
                    }
                }
                break;
                
            case MSG_AGENDAR: {
                int id = adicionar_servico(&ctrl, msg.username, msg.pid_cliente,
                                          msg.hora_agendada, msg.local_partida, 
                                          msg.distancia_km);
                if (id > 0) {
                    resposta.sucesso = 1;
                    sprintf(resposta.mensagem, "Serviço agendado com ID %d para hora %d", 
                           id, msg.hora_agendada);
                    printf("[AGENDAR] %s agendou serviço %d\n", msg.username, id);
                } else if (id == -2) {
                    resposta.sucesso = 0;
                    strcpy(resposta.mensagem, "Dados inválidos (hora<0, distância<=0 ou local vazio)!");
                } else {
                    resposta.sucesso = 0;
                    strcpy(resposta.mensagem, "Erro ao agendar serviço (limite atingido)!");
                }
                break;
            }
            
            case MSG_CANCELAR: {
                int resultado = cancelar_servico(&ctrl, msg.id_servico, msg.username);
                if (resultado == 0) {
                    resposta.sucesso = 1;
                    sprintf(resposta.mensagem, "Serviço %d cancelado", msg.id_servico);
                    printf("[CANCELAR] %s cancelou serviço %d\n", msg.username, msg.id_servico);
                } else {
                    resposta.sucesso = 0;
                    strcpy(resposta.mensagem, "Erro ao cancelar serviço!");
                }
                break;
            }
            
            case MSG_CONSULTAR: {
                resposta.sucesso = 1;
                listar_servicos_usuario(&ctrl, msg.username, resposta.mensagem);
                break;
            }
            
            case MSG_TERMINAR:
                remover_cliente(&ctrl, msg.username);
                resposta.sucesso = 1;
                strcpy(resposta.mensagem, "Desconectado");
                printf("[LOGOUT] Cliente %s desconectou\n", msg.username);
                break;
        }
        
        if (fd_cliente > 0) {
            write(fd_cliente, &resposta, sizeof(MensagemCliente));
            close(fd_cliente);
        }
    }
    
    return NULL;
}

void processar_comando(char *comando) {
    char cmd[50];
    sscanf(comando, "%s", cmd);
    
    if (strcmp(cmd, "listar") == 0) {
        pthread_mutex_lock(&ctrl.mutex);
        printf("\n=== SERVIÇOS AGENDADOS ===\n");
        for (int i = 0; i < ctrl.num_servicos; i++) {
            if (ctrl.servicos[i].estado != SERVICO_CANCELADO) {
                printf("ID: %d | User: %s | Hora: %d | Local: %s | Distância: %d km | Estado: ",
                       ctrl.servicos[i].id,
                       ctrl.servicos[i].username,
                       ctrl.servicos[i].hora_agendada,
                       ctrl.servicos[i].local_partida,
                       ctrl.servicos[i].distancia_km);
                
                switch(ctrl.servicos[i].estado) {
                    case SERVICO_AGENDADO: printf("Agendado\n"); break;
                    case SERVICO_EM_EXECUCAO: printf("Em execução (%d%%)\n", 
                                                     ctrl.servicos[i].percentagem_percorrida); break;
                    case SERVICO_CONCLUIDO: printf("Concluído\n"); break;
                    default: printf("?\n");
                }
            }
        }
        pthread_mutex_unlock(&ctrl.mutex);
    }
    else if (strcmp(cmd, "utiliz") == 0) {
        pthread_mutex_lock(&ctrl.mutex);
        printf("\n=== UTILIZADORES CONECTADOS ===\n");
        for (int i = 0; i < MAX_UTILIZADORES; i++) {
            if (ctrl.clientes[i].ativo) {
                printf("User: %s | PID: %d | Estado: %s\n",
                       ctrl.clientes[i].username,
                       ctrl.clientes[i].pid,
                       ctrl.clientes[i].em_viagem ? "Em viagem" : "Aguardando");
            }
        }
        printf("Total: %d utilizadores\n", ctrl.num_clientes);
        pthread_mutex_unlock(&ctrl.mutex);
    }
    else if (strcmp(cmd, "frota") == 0) {
        pthread_mutex_lock(&ctrl.mutex);
        printf("\n=== ESTADO DA FROTA ===\n");
        for (int i = 0; i < ctrl.max_veiculos; i++) {
            if (ctrl.veiculos[i].ativo) {
                Servico *s = buscar_servico(&ctrl, ctrl.veiculos[i].id_servico);
                if (s) {
                    printf("Veículo PID %d | Serviço %d | Percentagem: %d%%\n",
                           ctrl.veiculos[i].pid,
                           ctrl.veiculos[i].id_servico,
                           s->percentagem_percorrida);
                }
            }
        }
        printf("Veículos disponíveis: %d/%d\n", ctrl.veiculos_disponiveis, ctrl.max_veiculos);
        pthread_mutex_unlock(&ctrl.mutex);
    }
    else if (strcmp(cmd, "cancelar") == 0) {
        int id;
        if (sscanf(comando, "cancelar %d", &id) == 1) {
            if (id == 0) {
                pthread_mutex_lock(&ctrl.mutex);
                for (int i = 0; i < ctrl.num_servicos; i++) {
                    if (ctrl.servicos[i].estado != SERVICO_CONCLUIDO && 
                        ctrl.servicos[i].estado != SERVICO_CANCELADO) {
                        cancelar_servico(&ctrl, ctrl.servicos[i].id, NULL);
                    }
                }
                pthread_mutex_unlock(&ctrl.mutex);
                printf("Todos os serviços foram cancelados\n");
            } else {
                if (cancelar_servico(&ctrl, id, NULL) == 0) {
                    printf("Serviço %d cancelado\n", id);
                } else {
                    printf("Erro ao cancelar serviço %d\n", id);
                }
            }
        }
    }
    else if (strcmp(cmd, "km") == 0) {
        pthread_mutex_lock(&ctrl.mutex);
        printf("Total de quilómetros percorridos: %d km\n", ctrl.total_km_percorridos);
        pthread_mutex_unlock(&ctrl.mutex);
    }
    else if (strcmp(cmd, "hora") == 0) {
        pthread_mutex_lock(&ctrl.mutex);
        printf("Tempo atual: %d segundos\n", ctrl.tempo_atual);
        pthread_mutex_unlock(&ctrl.mutex);
    }
    else if (strcmp(cmd, "terminar") == 0) {
        printf("A terminar sistema...\n");
        ctrl.terminar = 1;
    }
    else {
        printf("Comando desconhecido: %s\n", cmd);
        printf("Comandos: listar, utiliz, frota, cancelar <id>, km, hora, terminar\n");
    }
}

int main() {
    // Verificar se já existe outro controlador
    if (mkfifo(FIFO_CONTROLADOR, 0666) == -1) {
        printf("[ERRO] Já existe um controlador em execução!\n");
        exit(1);
    }
    
    inicializar_controlador(&ctrl);
    
    ctrl.fd_controlador = open(FIFO_CONTROLADOR, O_RDWR);
    if (ctrl.fd_controlador == -1) {
        perror("Erro ao abrir FIFO do controlador");
        unlink(FIFO_CONTROLADOR);
        exit(1);
    }
    
    printf("=== CONTROLADOR DE FROTA DE VEÍCULOS AUTÓNOMOS ===\n");
    printf("Número máximo de veículos: %d\n", ctrl.max_veiculos);
    printf("Sistema iniciado.\n\n");
    
    // Criar threads
    pthread_t th_comunicacao, th_tempo, th_telemetria;
    
    if (pthread_create(&th_tempo, NULL, thread_tempo, NULL) != 0) {
        perror("Erro ao criar thread de tempo");
        exit(1);
    }
    
    if (pthread_create(&th_comunicacao, NULL, thread_comunicacao, NULL) != 0) {
        perror("Erro ao criar thread de comunicação");
        exit(1);
    }
    
    if (pthread_create(&th_telemetria, NULL, thread_telemetria, NULL) != 0) {
        perror("Erro ao criar thread de telemetria");
        exit(1);
    }
    
    // Loop de comandos do administrador
    char comando[TAM_COMANDO];
    while (!ctrl.terminar) {
        printf("> ");
        fflush(stdout);
        
        if (fgets(comando, TAM_COMANDO, stdin) == NULL) break;
        
        comando[strcspn(comando, "\n")] = 0;
        if (strlen(comando) > 0) {
            processar_comando(comando);
        }
    }
    
    // Cancelar todos os serviços ativos
    printf("\n[SHUTDOWN] A cancelar serviços ativos...\n");
    for (int i = 0; i < ctrl.num_servicos; i++) {
        if (ctrl.servicos[i].estado == SERVICO_EM_EXECUCAO) {
            kill(ctrl.servicos[i].pid_veiculo, SIGUSR1);
        }
    }
    
    // Dar tempo para veículos terminarem graciosamente
    sleep(2);
    
    // Aguardar threads
    pthread_join(th_comunicacao, NULL);
    pthread_join(th_tempo, NULL);
    pthread_join(th_telemetria, NULL);
    
    // Recolher TODOS os zombies restantes (failsafe)
    printf("[SHUTDOWN] A recolher processos zombie restantes...\n");
    int zombies_recolhidos = 0;
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
        zombies_recolhidos++;
        printf("[SHUTDOWN] Zombie recolhido: PID %d\n", pid);
    }
    if (zombies_recolhidos > 0) {
        printf("[SHUTDOWN] Total de zombies recolhidos: %d\n", zombies_recolhidos);
    }
    
    // Limpar recursos
    close(ctrl.fd_controlador);
    unlink(FIFO_CONTROLADOR);
    pthread_mutex_destroy(&ctrl.mutex);
    
    printf("Sistema terminado.\n");
    return 0;
}
