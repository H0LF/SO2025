#include "utils.h"

//void acorda(int s, siginfo_t *info, void *c) { }
void acorda(int s) {
    //printf("\nServidor vai encerrar %d\n",s); 
}

int main()
{    
    pthread_t th_comunicacao, th_tempo;
    Data dt;

    if (mkfifo(FIFO_SERVER, 0666) == -1){
        printf("[ERRO]-> Ja existe uma central de controlo a correr!\n");
        exit(EXIT_FAILURE);
    }

    preparacao(&dt);
    
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = acorda;
    // act.sa_flags = SA_RESTART | SA_SIGINFO;
    sigaction(SIGUSR1, &act, NULL);

    pthread_mutex_t mutex; 
    pthread_mutex_init(&mutex,NULL);
    dt.mutex = & mutex; // partilha da variavel do mutex
    
    printf("Central de Controlo de Frota pronta.\n");

    if(pthread_create(&th_comunicacao, NULL, &thread_comunicacao, &dt)!= 0){
        printf("[ERRO]->Nao foi possivel criar a thread de comunicacao\n");
        exit(-1);
    }
    if(pthread_create(&th_tempo, NULL, &thread_tempo, &dt)!= 0){
        printf("[ERRO]->Nao foi possivel criar a thread de Alarme\n");
        exit(-1);
    }
    //sleep(1);

    imprime_prompt();

    //comandos
    do{
        char input[100];
        char command[50];
        char argument1[50];

        
        fgets(input, sizeof(input), stdin);

        // Dividir o comando e os argumentos
        int numArgs = sscanf(input, "%s %s" , command, argument1);
        if (strcmp(command, "users") == 0 && numArgs == 1){
            mostra_users(&dt);  // Lista veículos ativos na frota
            imprime_prompt();
        }
        else if (strcmp(command, "remove") == 0 && numArgs == 2) {
            remove_user(&dt, argument1); 
            imprime_prompt();
        }
        else if (strcmp(command, "topics") == 0 && numArgs == 1){
            mostra_topicsM(&dt);  // Lista rotas/zonas ativas            
        }
        else if (strcmp(command, "show") == 0 && numArgs == 2){
            
            //printf("Mostra as mensagens num determinado topico\n");
            sh_Topico(&dt,argument1);
        }
        else if (strcmp(command, "lock") == 0 && numArgs == 2){
            
            //printf("Bloqueio do topico: %s\n",argument1);
            bloqueiaTopico(&dt,argument1);
        }
        else if (strcmp(command, "unlock") == 0 && numArgs == 2){
            //printf("Desbloqueio do topico: %s\n",argument1);
            desbloqueiaTopico(&dt,argument1);
        }
        else if (strcmp(command, "close") == 0 && numArgs == 1){
            
            int control = cria_txt(&dt);
            if(control ==0)
            {
                printf("Ficheiro Criado com sucesso!\n");
            }else if (control==-1)
            {
                printf("[ERRO]->Nao foi possivel criar o ficheiro!\n");
            }
            

            pthread_mutex_lock(dt.mutex);
            dt.trinco = 1; // Sinal para encerrar as threads
            pthread_mutex_unlock(dt.mutex);

            encerrar_todos_feeds(&dt, "Plataforma encerrada pelo administrador.");
            
            unlink(FIFO_SERVER);
            pthread_kill(th_comunicacao, SIGUSR1);
            pthread_kill(th_tempo, SIGUSR1);

            printf("Encerrando a plataforma...\n");

            
        } else {
            printf("\n[ERRO]: Comando inválido. Tente novamente.\n");
            imprime_prompt();
        }

    } while(dt.trinco!=1);



    pthread_join(th_comunicacao, NULL);
    pthread_join(th_tempo,NULL);

    //unlink(FIFO_SERVER);
    pthread_mutex_destroy(&mutex);

    return 0;
}

