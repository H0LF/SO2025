# Sistema de Gestão de Frota de Veículos Autónomos

## Sistemas Operativos 2024/2025 - Trabalho Prático

---

## 📋 Descrição Geral

Este projeto implementa uma plataforma de gestão de serviços de transporte com veículos autónomos, composta por três aplicações principais:

- **Controlador**: Gere a frota de veículos, recebe pedidos de transporte e aloca veículos
- **Cliente**: Interface para utilizadores agendarem, consultarem e cancelarem serviços
- **Veículo**: Simula um veículo autónomo que executa os serviços agendados

---

## 🏗️ Arquitetura do Sistema

### Comunicação Entre Processos

1. **Cliente ↔ Controlador**: Named pipes (`/tmp/fifo_controlador` e `/tmp/fifo_cliente_<PID>`)
2. **Controlador → Veículo**: Argumentos da linha de comandos (ao lançar o processo)
3. **Veículo → Controlador**: stdout (telemetria via printf)
4. **Veículo ↔ Cliente**: Named pipe (`/tmp/fifo_veiculo_<PID>`)

### Concorrência

O controlador utiliza **2 threads**:

- **Thread de Comunicação**: Processa mensagens dos clientes via named pipes
- **Thread de Tempo**: Incrementa o tempo simulado e verifica serviços agendados

**Sincronização**: `pthread_mutex_t` para proteger acesso à estrutura `Controlador`

### Select()

O cliente utiliza `select()` para multiplexar:

- **stdin**: Comandos do utilizador
- **FIFO**: Mensagens do veículo/controlador

---

## 🚀 Compilação

```bash
make clean
make
```

Gera 3 executáveis:

- `controlador`
- `cliente`
- `veiculo`

---

## 📊 Limites do Sistema

Conforme enunciado:

- **Máximo de utilizadores**: 30 (definido em `MAX_UTILIZADORES`)
- **Máximo de veículos**: 10 (configurável via variável de ambiente `NVEICULOS`)
- **Máximo de serviços**: 100 (definido em `MAX_SERVICOS`)

---

## 🎯 Utilização

### 1. Controlador (Administrador)

```bash
export NVEICULOS=5    # Define tamanho da frota (opcional, padrão=10)
./controlador
```

**Comandos disponíveis:**

- `listar` - Mostra todos os serviços agendados
- `utiliz` - Lista utilizadores conectados e seu estado
- `frota` - Mostra estado de cada veículo (percentagem da viagem)
- `cancelar <id>` - Cancela um serviço (0 = todos)
- `km` - Mostra quilómetros totais percorridos
- `hora` - Mostra tempo simulado atual
- `terminar` - Encerra todo o sistema

**Notas:**

- Apenas uma instância pode estar ativa
- Lança processos `veiculo` automaticamente na hora agendada
- Recebe telemetria de todos os veículos via stdout

### 2. Cliente (Utilizador)

```bash
./cliente <username>
```

Exemplo:

```bash
./cliente joao
```

**Comandos disponíveis:**

- `agendar <hora> <local> <distancia>` - Agenda serviço de transporte
  - Exemplo: `agendar 30 Coimbra 50`
- `cancelar <id>` - Cancela serviço agendado
- `consultar` - Lista seus serviços
- `entrar <destino>` - Entra no veículo quando ele chega
  - Exemplo: `entrar Lisboa`
- `sair` - Sai do veículo a meio da viagem
- `terminar` - Desconecta do sistema
- `ajuda` - Mostra ajuda

**Notas:**

- Requer controlador ativo
- Username deve ser único
- Usa `select()` para receber comandos e mensagens simultaneamente

### 3. Veículo (Lançado automaticamente)

O processo veículo é lançado pelo controlador, **não deve ser executado manualmente**.

**Fluxo de execução:**

1. Recebe dados do serviço via argumentos
2. Contacta o cliente via named pipe
3. Aguarda cliente entrar (60s timeout)
4. Simula viagem (1 km/segundo)
5. Reporta telemetria a cada 10% para stdout
6. Termina quando cliente chega ao destino ou sai

**Telemetria enviada:**

```
VEICULO_LANCADO: ID=1, Cliente=joao, Local=Coimbra, Distancia=50km
ENTROU: Cliente entrou no veículo. Destino: Lisboa
PERCENTAGEM:10
PERCENTAGEM:20
...
PERCENTAGEM:100
CHEGOU: Cliente chegou ao destino: Lisboa
```

**Cancelamento:**

- Recebe `SIGUSR1` do controlador para cancelar a meio

---

## 📝 Exemplo de Uso Completo

### Terminal 1 - Controlador

```bash
export NVEICULOS=3
./controlador
> utiliz
> frota
> km
> terminar
```

### Terminal 2 - Cliente 1

```bash
./cliente joao
joao> agendar 10 Coimbra 50
joao> consultar
joao> entrar Lisboa
[VEÍCULO] Veículo chegou ao local: Coimbra
[VEÍCULO] Chegou ao destino: Lisboa. Distância: 50 km
joao> terminar
```

### Terminal 3 - Cliente 2

```bash
./cliente maria
maria> agendar 15 Porto 30
maria> consultar
maria> cancelar 2
maria> terminar
```

---

## 🔧 Mecanismos do Sistema Operativo Utilizados

### Comunicação

- ✅ **Named Pipes (FIFOs)**: `mkfifo()`, `open()`, `read()`, `write()`
- ✅ **Argumentos linha de comandos**: Passagem de dados para veículos
- ✅ **Redirecionamento stdout**: Captura de telemetria

### Processos

- ✅ **fork()**: Criação de processos veículo
- ✅ **exec()**: `execl()` para executar programa veículo
- ✅ **wait()**: Gestão de processos filho

### Sinais

- ✅ **SIGUSR1**: Cancelamento de serviços em execução
- ✅ **SIGINT**: Terminação limpa do cliente
- ✅ **signal()** e **sigaction()**: Tratamento de sinais

### Threads e Sincronização

- ✅ **pthread_create()**: Criação de threads
- ✅ **pthread_mutex**: Proteção de dados partilhados
- ✅ **pthread_join()**: Sincronização na terminação

### Multiplexagem I/O

- ✅ **select()**: Cliente aguarda stdin + FIFO simultaneamente
- ✅ **FD_SET, FD_ZERO, FD_ISSET**: Gestão de file descriptors

### Sistema de Ficheiros

- ✅ **mkfifo()**: Criação de named pipes
- ✅ **unlink()**: Remoção de FIFOs
- ✅ **access()**: Verificação de existência de ficheiros

---

## ⚙️ Variáveis de Ambiente

- **NVEICULOS**: Define o número máximo de veículos simultâneos (padrão: 10, máx: 10)

Exemplo:

```bash
export NVEICULOS=5
./controlador
```

---

## 🧪 Verificação de Requisitos

| Requisito                       | Implementação                      | Ficheiro           |
| ------------------------------- | ---------------------------------- | ------------------ |
| 30 utilizadores máx             | `MAX_UTILIZADORES 30`              | utils.h            |
| 10 veículos máx                 | `MAX_VEICULOS 10` + `NVEICULOS`    | utils.h, utils.c   |
| Named pipes cliente↔controlador | `FIFO_CONTROLADOR`, `FIFO_CLIENTE` | Todos              |
| Args para veículo               | `execl()` com 6 argumentos         | controlador.c      |
| Telemetria via stdout           | `printf()` no veículo              | veiculo.c          |
| Select() no cliente             | `select()` + `FD_SET`              | cliente.c          |
| Threads no controlador          | 2 threads (comunicação + tempo)    | controlador.c      |
| Mutex                           | `pthread_mutex_t`                  | utils.h, utils.c   |
| SIGUSR1 cancela veículo         | `signal(SIGUSR1)` + `kill()`       | veiculo.c, utils.c |
| Velocidade 1km/s                | `sleep(1)` por km                  | veiculo.c          |
| Reporta a cada 10%              | Cálculo percentagem                | veiculo.c          |

---

## 🗂️ Estrutura de Ficheiros

```
.
├── controlador.c       # Aplicação controlador (servidor)
├── cliente.c           # Aplicação cliente (interface utilizador)
├── veiculo.c           # Aplicação veículo (lançada pelo controlador)
├── utils.c             # Funções auxiliares
├── utils.h             # Estruturas e declarações
├── makefile            # Regras de compilação
└── README.md           # Esta documentação
```

---

## 🐛 Tratamento de Erros

- **Controlador já ativo**: Deteta pelo FIFO existente
- **Cliente sem controlador**: Verifica `access()` antes de conectar
- **Username duplicado**: Controlador rejeita login
- **Limite de utilizadores**: Controlador retorna erro
- **Sem veículos disponíveis**: Serviço fica agendado até libertar veículo
- **Cliente não entra no veículo**: Timeout de 60s, veículo termina
- **Limpeza de recursos**: FIFOs removidos ao terminar

---

## 📚 Funcionalidades Implementadas

### Controlador

- ✅ Gestão de até 30 utilizadores
- ✅ Gestão de frota de veículos (configurável)
- ✅ Agendamento e controlo de serviços
- ✅ Lançamento automático de veículos
- ✅ Recepção de telemetria
- ✅ Contador de quilómetros total
- ✅ Tempo simulado (segundos)
- ✅ Cancelamento individual e em massa

### Cliente

- ✅ Login com username único
- ✅ Agendar serviços
- ✅ Consultar serviços pessoais
- ✅ Cancelar serviços
- ✅ Interagir com veículo (entrar/sair)
- ✅ Receção assíncrona de mensagens (select)

### Veículo

- ✅ Recebe dados via argumentos
- ✅ Contacta cliente automaticamente
- ✅ Aguarda entrada do cliente
- ✅ Simula viagem (1km/s)
- ✅ Reporta telemetria (10% incrementos)
- ✅ Permite saída a meio
- ✅ Responde a SIGUSR1 (cancelamento)

---

## 👥 Autores

- João Pais - 2020131717
- Rui Marques - 2021123220

---

## 📅 Data de Entrega

13 de dezembro de 2025

---

## 📖 Notas Finais

Este sistema implementa todos os requisitos do enunciado utilizando exclusivamente chamadas ao sistema POSIX (não utiliza bibliotecas de terceiros). A arquitetura é modular e escalável, respeitando os limites definidos e garantindo sincronização adequada entre processos e threads.
