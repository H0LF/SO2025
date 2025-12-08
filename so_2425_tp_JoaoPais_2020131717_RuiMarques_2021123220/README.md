# Sistema de Gestão de Frota de Veículos - Trabalho Prático SO 2024/2025

## Autores

- João Pais - 2020131717
- Rui Marques - 2021123220

## Descrição

Sistema de gestão de frota de veículos e rotas implementado em C, utilizando FIFOs para comunicação entre a central de controlo e os veículos, com threads para gestão concorrente de rotas e alertas em tempo real.

## Estrutura do Projeto

### Ficheiros Principais

- **manager.c** - Central de Controlo que gere a frota de veículos e rotas
- **feed.c** - Interface do veículo/motorista para comunicar com a central
- **utils.h** - Declarações de estruturas e funções partilhadas
- **utils.c** - Implementação de funções auxiliares
- **operacoesmemoria.c** - Operações sobre rotas, zonas e alertas de trânsito
- **threads.c** - Implementação das threads de comunicação e expiração de alertas
- **makefile** - Ficheiro para compilação automática

### Ficheiros Auxiliares

- **flags.txt** - Documentação sobre os tipos e flags usados
- **memoria.txt** - Ficheiro de persistência de dados
- **Relatorio_SO_2425.pdf** - Relatório do trabalho

## Funcionalidades

### Manager (Central de Controlo)

- `users` - Lista todos os veículos conectados à frota
- `remove <veiculo>` - Remove um veículo do sistema
- `topics` - Lista todas as rotas/zonas ativas
- `show <rota>` - Mostra os alertas de uma rota específica
- `lock <rota>` - Fecha/bloqueia uma rota (impede novos alertas)
- `unlock <rota>` - Reabre uma rota
- `close` - Encerra a central e guarda o estado

### Feed (Interface do Veículo)

- `topics` - Lista todas as rotas/zonas disponíveis
- `subscribe <rota>` - Atribui veículo a uma rota/zona
- `unsubscribe <rota>` - Remove veículo de uma rota/zona
- `msg <rota> <duracao> <alerta>` - Envia alerta/evento para uma rota
  - Se duracao = 0: alerta instantâneo (ex: acidente, perigo)
  - Se duracao > 0: alerta persistente (ex: obra na via, trânsito intenso)
- `exit` - Termina a sessão do veículo

## Compilação

### Compilar tudo

```bash
make
```

### Compilar apenas o manager

```bash
make manager
```

### Compilar apenas o feed

```bash
make feed
```

### Limpar ficheiros compilados

```bash
make clean
```

## Execução

### 1. Iniciar o Manager (Central de Controlo)

```bash
./manager
```

### 2. Iniciar Feeds (Veículos) - em terminais separados

```bash
./feed <identificador_veiculo>
```

Exemplo:

```bash
./feed Autocarro101
./feed Taxi42
./feed Ambulancia5
```

## Variável de Ambiente

O sistema utiliza a variável de ambiente `MSG_FICH` para definir o ficheiro de persistência:

```bash
export MSG_FICH=memoria.txt
```

Se não estiver definida, o sistema cria-a automaticamente com o valor padrão `memoria.txt`.

## Limites do Sistema

- **MAX_TOPICOS**: 20 rotas/zonas simultâneas
- **MAX_UTILIZADORES**: 10 veículos simultâneos na frota
- **MAX_TOPICOS_PERMANENTES**: 5 alertas persistentes por rota
- **TAM_MSG**: 300 caracteres por alerta
- **TAM_NOME**: 50 caracteres para identificação do veículo
- **TAM_TOPICO**: 20 caracteres para nomes de rotas

## Comunicação

### FIFOs

- **Servidor**: `/tmp/fifo_server`
- **Clientes**: `/tmp/fifo<PID>` (onde PID é o ID do processo)

### Tipos de Mensagens

- **Tipo 0**: Registo de veículo na central
- **Tipo 1**: Mensagem/alerta do veículo para central
- **Tipo 2**: Mensagem da central para veículo
- **Tipo 3**: Lista de rotas/zonas

### Flags de Resposta

- **0**: Resposta normal da central
- **1**: Alerta entre veículos na mesma rota
- **2**: Alertas persistentes
- **-1**: Sinal para desconectar veículo

## Características Técnicas

### Concorrência

- **Thread de Comunicação**: Gere comunicação entre central e veículos via FIFO
- **Thread de Temporização**: Controla expiração de alertas persistentes nas rotas

### Sincronização

- Utilização de `pthread_mutex_t` para proteção de dados da frota
- `select()` para multiplexing I/O na interface do veículo

### Persistência

- Guarda estado em ficheiro de texto ao encerrar o manager
- Carrega estado automaticamente ao iniciar

## Notas Importantes

1. A central de controlo deve ser iniciada antes dos veículos
2. Cada veículo deve ter um identificador único
3. Alertas persistentes são removidos automaticamente após expiração
4. Rotas bloqueadas não aceitam novos alertas
5. Apenas veículos atribuídos a uma rota podem enviar alertas
6. O sistema limpa todos os FIFOs ao encerrar

## Exemplo de Utilização

```bash
# Terminal 1 - Central de Controlo
$ ./manager
Servidor pronto.

Manager > users
Manager > topics

# Terminal 2 - Autocarro 101
$ ./feed Autocarro101
feed> subscribe Rota_Centro
feed> msg Rota_Centro 300 Transito intenso na Avenida Principal
feed> topics

# Terminal 3 - Ambulância 5
$ ./feed Ambulancia5
feed> subscribe Rota_Centro
[MANAGER]: Transito intenso na Avenida Principal
feed> msg Rota_Centro 0 Acidente! Solicitando apoio
```

## Troubleshooting

### Erro: "Ja existe um servidor a correr"

- Remover o FIFO manualmente: `rm /tmp/fifo_server`

### Erro: "Nome de utilizador já em uso"

- Escolher um identificador diferente para o veículo

### Veículos não recebem alertas

- Verificar se estão atribuídos à rota
- Verificar se a rota não está bloqueada

## Licença

Trabalho académico - Sistemas Operativos 2024/2025
