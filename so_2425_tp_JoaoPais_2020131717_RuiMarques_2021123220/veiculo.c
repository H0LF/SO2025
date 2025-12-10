#include "utils.h"

int cancelado = 0;
int id_servico;
char username_cliente[TAM_NOME];
int pid_cliente;
char local_partida[TAM_LOCAL];
char local_destino[TAM_LOCAL];
int distancia_km;
int percentagem = 0;
char fifo_veiculo[50];
int fd_veiculo = -1;

void sigusr1_handler(int sig) {
    cancelado = 1;
    printf("CANCELADO: Serviço cancelado pelo controlador\n");
    fflush(stdout);
}

void limpar_recursos() {
    if (fd_veiculo > 0) {
        close(fd_veiculo);
    }
    unlink(fifo_veiculo);
}

int main(int argc, char *argv[]) {
    if (argc != 6) {
        fprintf(stderr, "Uso: %s <id_servico> <username> <pid_cliente> <local_partida> <distancia_km>\n", argv[0]);
        return 1;
    }
    
    // Receber argumentos da linha de comandos
    id_servico = atoi(argv[1]);
    strcpy(username_cliente, argv[2]);
    pid_cliente = atoi(argv[3]);
    strcpy(local_partida, argv[4]);
    distancia_km = atoi(argv[5]);
    
    // Configurar handler para SIGUSR1
    signal(SIGUSR1, sigusr1_handler);
    
    // Criar FIFO para comunicação com o cliente
    sprintf(fifo_veiculo, "/tmp/fifo_veiculo_%d", pid_cliente);
    if (mkfifo(fifo_veiculo, 0666) == -1) {
        // FIFO já existe, não é erro crítico
    }
    
    fd_veiculo = open(fifo_veiculo, O_RDWR | O_NONBLOCK);
    
    printf("VEICULO_LANCADO: ID=%d, Cliente=%s, Local=%s, Distancia=%dkm\n",
           id_servico, username_cliente, local_partida, distancia_km);
    fflush(stdout);
    
    // Notificar cliente que chegou
    char fifo_cliente[50];
    sprintf(fifo_cliente, FIFO_CLIENTE, pid_cliente);
    int fd_cliente = open(fifo_cliente, O_WRONLY | O_NONBLOCK);
    
    if (fd_cliente > 0) {
        MensagemVeiculo msg;
        msg.tipo = 1;
        sprintf(msg.mensagem, "Veículo chegou ao local: %s. Use 'entrar <destino>' para iniciar.", 
                local_partida);
        write(fd_cliente, &msg, sizeof(MensagemVeiculo));
        close(fd_cliente);
    }
    
    // Aguardar cliente entrar (com timeout)
    int cliente_entrou = 0;
    int timeout = 60; // 60 segundos
    
    while (!cliente_entrou && timeout > 0 && !cancelado) {
        if (fd_veiculo > 0) {
            MensagemVeiculo msg;
            int n = read(fd_veiculo, &msg, sizeof(MensagemVeiculo));
            
            if (n > 0 && msg.tipo == 1) {
                strcpy(local_destino, msg.destino);
                cliente_entrou = 1;
                printf("ENTROU: Cliente entrou no veículo. Destino: %s\n", local_destino);
                fflush(stdout);
                break;
            }
        }
        
        sleep(1);
        timeout--;
    }
    
    if (!cliente_entrou) {
        printf("TIMEOUT: Cliente não entrou no veículo\n");
        fflush(stdout);
        limpar_recursos();
        return 1;
    }
    
    if (cancelado) {
        limpar_recursos();
        return 0;
    }
    
    // Simular viagem (1 km por segundo, reportar a cada 10%)
    int intervalo = distancia_km / 10;
    if (intervalo < 1) intervalo = 1;
    
    int km_percorridos = 0;
    int cliente_saiu = 0;
    
    while (km_percorridos < distancia_km && !cliente_saiu && !cancelado) {
        sleep(1);
        km_percorridos++;
        
        // Verificar se cliente quer sair
        if (fd_veiculo > 0) {
            MensagemVeiculo msg;
            int n = read(fd_veiculo, &msg, sizeof(MensagemVeiculo));
            
            if (n > 0 && msg.tipo == 2) {
                cliente_saiu = 1;
                printf("SAIU: Cliente saiu a meio da viagem (%d%%)\n", percentagem);
                fflush(stdout);
                break;
            }
        }
        
        // Reportar percentagem a cada 10%
        int nova_percentagem = (km_percorridos * 100) / distancia_km;
        if (nova_percentagem >= percentagem + 10) {
            percentagem = (nova_percentagem / 10) * 10;
            printf("PERCENTAGEM:%d\n", percentagem);
            fflush(stdout);
        }
    }
    
    if (cancelado) {
        limpar_recursos();
        return 0;
    }
    
    if (cliente_saiu) {
        // Notificar cliente
        fd_cliente = open(fifo_cliente, O_WRONLY | O_NONBLOCK);
        if (fd_cliente > 0) {
            MensagemVeiculo msg;
            msg.tipo = 3;
            sprintf(msg.mensagem, "Viagem interrompida. Percorrido: %d%%", percentagem);
            write(fd_cliente, &msg, sizeof(MensagemVeiculo));
            close(fd_cliente);
        }
    } else {
        // Chegou ao destino
        percentagem = 100;
        printf("PERCENTAGEM:100\n");
        fflush(stdout);
        printf("CHEGOU: Cliente chegou ao destino: %s\n", local_destino);
        fflush(stdout);
        
        fd_cliente = open(fifo_cliente, O_WRONLY | O_NONBLOCK);
        if (fd_cliente > 0) {
            MensagemVeiculo msg;
            msg.tipo = 3;
            sprintf(msg.mensagem, "Chegou ao destino: %s. Distância: %d km", 
                    local_destino, distancia_km);
            write(fd_cliente, &msg, sizeof(MensagemVeiculo));
            close(fd_cliente);
        }
    }
    
    limpar_recursos();
    return 0;
}
