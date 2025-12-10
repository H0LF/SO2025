#include "utils.h"

void inicializar_controlador(Controlador *ctrl) {
    ctrl->num_clientes = 0;
    ctrl->num_servicos = 0;
    ctrl->proximo_id_servico = 1;
    ctrl->tempo_atual = 0;
    ctrl->total_km_percorridos = 0;
    ctrl->terminar = 0;
    
    // Ler variável de ambiente NVEICULOS
    char *env_veiculos = getenv("NVEICULOS");
    if (env_veiculos != NULL) {
        ctrl->max_veiculos = atoi(env_veiculos);
        if (ctrl->max_veiculos > MAX_VEICULOS) {
            ctrl->max_veiculos = MAX_VEICULOS;
        }
    } else {
        ctrl->max_veiculos = MAX_VEICULOS;
    }
    ctrl->veiculos_disponiveis = ctrl->max_veiculos;
    
    for (int i = 0; i < MAX_UTILIZADORES; i++) {
        ctrl->clientes[i].ativo = 0;
    }
    
    for (int i = 0; i < MAX_VEICULOS; i++) {
        ctrl->veiculos[i].ativo = 0;
    }
    
    pthread_mutex_init(&ctrl->mutex, NULL);
}

int cliente_existe(Controlador *ctrl, const char *username) {
    for (int i = 0; i < ctrl->num_clientes; i++) {
        if (ctrl->clientes[i].ativo && strcmp(ctrl->clientes[i].username, username) == 0) {
            return 1;
        }
    }
    return 0;
}

int adicionar_cliente(Controlador *ctrl, const char *username, int pid) {
    pthread_mutex_lock(&ctrl->mutex);
    
    if (ctrl->num_clientes >= MAX_UTILIZADORES) {
        pthread_mutex_unlock(&ctrl->mutex);
        return -1;
    }
    
    if (cliente_existe(ctrl, username)) {
        pthread_mutex_unlock(&ctrl->mutex);
        return -2;
    }
    
    for (int i = 0; i < MAX_UTILIZADORES; i++) {
        if (!ctrl->clientes[i].ativo) {
            strcpy(ctrl->clientes[i].username, username);
            ctrl->clientes[i].pid = pid;
            ctrl->clientes[i].ativo = 1;
            ctrl->clientes[i].em_viagem = 0;
            ctrl->num_clientes++;
            pthread_mutex_unlock(&ctrl->mutex);
            return 0;
        }
    }
    
    pthread_mutex_unlock(&ctrl->mutex);
    return -1;
}

int remover_cliente(Controlador *ctrl, const char *username) {
    pthread_mutex_lock(&ctrl->mutex);
    
    for (int i = 0; i < MAX_UTILIZADORES; i++) {
        if (ctrl->clientes[i].ativo && strcmp(ctrl->clientes[i].username, username) == 0) {
            ctrl->clientes[i].ativo = 0;
            ctrl->num_clientes--;
            pthread_mutex_unlock(&ctrl->mutex);
            return 0;
        }
    }
    
    pthread_mutex_unlock(&ctrl->mutex);
    return -1;
}

Cliente* buscar_cliente(Controlador *ctrl, const char *username) {
    for (int i = 0; i < MAX_UTILIZADORES; i++) {
        if (ctrl->clientes[i].ativo && strcmp(ctrl->clientes[i].username, username) == 0) {
            return &ctrl->clientes[i];
        }
    }
    return NULL;
}

int adicionar_servico(Controlador *ctrl, const char *username, int pid, 
                      int hora, const char *local, int distancia) {
    pthread_mutex_lock(&ctrl->mutex);
    
    if (ctrl->num_servicos >= MAX_SERVICOS) {
        pthread_mutex_unlock(&ctrl->mutex);
        return -1;
    }
    
    int idx = ctrl->num_servicos;
    ctrl->servicos[idx].id = ctrl->proximo_id_servico++;
    strcpy(ctrl->servicos[idx].username, username);
    ctrl->servicos[idx].pid_cliente = pid;
    ctrl->servicos[idx].hora_agendada = hora;
    strcpy(ctrl->servicos[idx].local_partida, local);
    ctrl->servicos[idx].local_destino[0] = '\0';
    ctrl->servicos[idx].distancia_km = distancia;
    ctrl->servicos[idx].estado = SERVICO_AGENDADO;
    ctrl->servicos[idx].pid_veiculo = 0;
    ctrl->servicos[idx].percentagem_percorrida = 0;
    ctrl->num_servicos++;
    
    int id = ctrl->servicos[idx].id;
    pthread_mutex_unlock(&ctrl->mutex);
    return id;
}

Servico* buscar_servico(Controlador *ctrl, int id) {
    for (int i = 0; i < ctrl->num_servicos; i++) {
        if (ctrl->servicos[i].id == id) {
            return &ctrl->servicos[i];
        }
    }
    return NULL;
}

int cancelar_servico(Controlador *ctrl, int id, const char *username) {
    pthread_mutex_lock(&ctrl->mutex);
    
    Servico *s = buscar_servico(ctrl, id);
    if (s == NULL) {
        pthread_mutex_unlock(&ctrl->mutex);
        return -1;
    }
    
    // Verifica se é o dono (se username != NULL)
    if (username != NULL && strcmp(s->username, username) != 0) {
        pthread_mutex_unlock(&ctrl->mutex);
        return -2;
    }
    
    // Se já está concluído ou cancelado, não faz nada
    if (s->estado == SERVICO_CONCLUIDO || s->estado == SERVICO_CANCELADO) {
        pthread_mutex_unlock(&ctrl->mutex);
        return -3;
    }
    
    // Se está em execução, envia SIGUSR1 ao veículo
    if (s->estado == SERVICO_EM_EXECUCAO && s->pid_veiculo > 0) {
        kill(s->pid_veiculo, SIGUSR1);
        liberar_veiculo(ctrl, s->pid_veiculo);
    }
    
    s->estado = SERVICO_CANCELADO;
    pthread_mutex_unlock(&ctrl->mutex);
    return 0;
}

void listar_servicos_usuario(Controlador *ctrl, const char *username, char *resultado) {
    pthread_mutex_lock(&ctrl->mutex);
    
    resultado[0] = '\0';
    char linha[256];
    int encontrados = 0;
    
    for (int i = 0; i < ctrl->num_servicos; i++) {
        if (strcmp(ctrl->servicos[i].username, username) == 0 && 
            ctrl->servicos[i].estado != SERVICO_CANCELADO) {
            
            sprintf(linha, "ID: %d | Hora: %d | Local: %s | Distância: %d km | Estado: ",
                    ctrl->servicos[i].id,
                    ctrl->servicos[i].hora_agendada,
                    ctrl->servicos[i].local_partida,
                    ctrl->servicos[i].distancia_km);
            strcat(resultado, linha);
            
            switch(ctrl->servicos[i].estado) {
                case SERVICO_AGENDADO:
                    strcat(resultado, "Agendado\n");
                    break;
                case SERVICO_EM_EXECUCAO:
                    sprintf(linha, "Em execução (%d%%)\n", ctrl->servicos[i].percentagem_percorrida);
                    strcat(resultado, linha);
                    break;
                case SERVICO_CONCLUIDO:
                    strcat(resultado, "Concluído\n");
                    break;
                default:
                    strcat(resultado, "Desconhecido\n");
            }
            encontrados++;
        }
    }
    
    if (encontrados == 0) {
        strcpy(resultado, "Nenhum serviço agendado.\n");
    }
    
    pthread_mutex_unlock(&ctrl->mutex);
}

int alocar_veiculo(Controlador *ctrl) {
    pthread_mutex_lock(&ctrl->mutex);
    
    if (ctrl->veiculos_disponiveis <= 0) {
        pthread_mutex_unlock(&ctrl->mutex);
        return -1;
    }
    
    for (int i = 0; i < ctrl->max_veiculos; i++) {
        if (!ctrl->veiculos[i].ativo) {
            ctrl->veiculos_disponiveis--;
            pthread_mutex_unlock(&ctrl->mutex);
            return i;
        }
    }
    
    pthread_mutex_unlock(&ctrl->mutex);
    return -1;
}

void liberar_veiculo(Controlador *ctrl, int pid_veiculo) {
    pthread_mutex_lock(&ctrl->mutex);
    
    for (int i = 0; i < ctrl->max_veiculos; i++) {
        if (ctrl->veiculos[i].ativo && ctrl->veiculos[i].pid == pid_veiculo) {
            ctrl->veiculos[i].ativo = 0;
            ctrl->veiculos_disponiveis++;
            break;
        }
    }
    
    pthread_mutex_unlock(&ctrl->mutex);
}

void processar_telemetria(Controlador *ctrl, int pid_veiculo, const char *info) {
    pthread_mutex_lock(&ctrl->mutex);
    
    // Encontra o veículo
    int idx_veiculo = -1;
    for (int i = 0; i < ctrl->max_veiculos; i++) {
        if (ctrl->veiculos[i].ativo && ctrl->veiculos[i].pid == pid_veiculo) {
            idx_veiculo = i;
            break;
        }
    }
    
    if (idx_veiculo < 0) {
        pthread_mutex_unlock(&ctrl->mutex);
        return;
    }
    
    int id_servico = ctrl->veiculos[idx_veiculo].id_servico;
    Servico *s = buscar_servico(ctrl, id_servico);
    
    if (s == NULL) {
        pthread_mutex_unlock(&ctrl->mutex);
        return;
    }
    
    // Processa a informação de telemetria
    if (strstr(info, "ENTROU") != NULL) {
        s->estado = SERVICO_EM_EXECUCAO;
        Cliente *c = buscar_cliente(ctrl, s->username);
        if (c) c->em_viagem = 1;
    }
    else if (strstr(info, "SAIU") != NULL || strstr(info, "CHEGOU") != NULL) {
        s->estado = SERVICO_CONCLUIDO;
        ctrl->total_km_percorridos += (s->distancia_km * s->percentagem_percorrida) / 100;
        liberar_veiculo(ctrl, pid_veiculo);
        Cliente *c = buscar_cliente(ctrl, s->username);
        if (c) c->em_viagem = 0;
    }
    else if (sscanf(info, "PERCENTAGEM:%d", &s->percentagem_percorrida) == 1) {
        ctrl->veiculos[idx_veiculo].percentagem_percorrida = s->percentagem_percorrida;
    }
    
    pthread_mutex_unlock(&ctrl->mutex);
}

void verificar_servicos_agendados(Controlador *ctrl) {
    pthread_mutex_lock(&ctrl->mutex);
    
    for (int i = 0; i < ctrl->num_servicos; i++) {
        if (ctrl->servicos[i].estado == SERVICO_AGENDADO &&
            ctrl->servicos[i].hora_agendada == ctrl->tempo_atual) {
            
            // Verifica se há veículos disponíveis
            int idx = alocar_veiculo(ctrl);
            if (idx < 0) {
                printf("[ALERTA] Sem veículos disponíveis para o serviço %d\n", 
                       ctrl->servicos[i].id);
                continue;
            }
            
            // Lança o processo veículo
            int pid = fork();
            if (pid == 0) {
                // Processo filho - veículo
                char id_str[20], pid_cliente_str[20], distancia_str[20];
                sprintf(id_str, "%d", ctrl->servicos[i].id);
                sprintf(pid_cliente_str, "%d", ctrl->servicos[i].pid_cliente);
                sprintf(distancia_str, "%d", ctrl->servicos[i].distancia_km);
                
                execl("./veiculo", "veiculo", 
                      id_str,
                      ctrl->servicos[i].username,
                      pid_cliente_str,
                      ctrl->servicos[i].local_partida,
                      distancia_str,
                      NULL);
                
                perror("Erro ao executar veículo");
                exit(1);
            }
            else if (pid > 0) {
                // Processo pai - controlador
                ctrl->veiculos[idx].pid = pid;
                ctrl->veiculos[idx].id_servico = ctrl->servicos[i].id;
                ctrl->veiculos[idx].percentagem_percorrida = 0;
                ctrl->veiculos[idx].ativo = 1;
                ctrl->servicos[i].pid_veiculo = pid;
                
                printf("[INFO] Veículo lançado (PID %d) para serviço %d\n", 
                       pid, ctrl->servicos[i].id);
            }
        }
    }
    
    pthread_mutex_unlock(&ctrl->mutex);
}
