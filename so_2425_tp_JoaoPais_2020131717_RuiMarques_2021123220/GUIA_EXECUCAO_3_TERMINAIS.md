# 🚀 GUIA DE EXECUÇÃO - 3 TERMINAIS

## ⚠️ IMPORTANTE: Abre 3 terminais WSL separados

---

## 📺 TERMINAL 1 - CONTROLADOR

### Passo 1: Navegar para a pasta

```bash
cd '/mnt/c/Users/H0LF/Documents/SO2025/so_2425_tp_JoaoPais_2020131717_RuiMarques_2021123220'
```

### Passo 2: Configurar número de veículos

```bash
export NVEICULOS=3
```

### Passo 3: Iniciar o controlador

```bash
./controlador
```

**Saída esperada:**

```
=== CONTROLADOR DE FROTA DE VEÍCULOS AUTÓNOMOS ===
Número máximo de veículos: 3
Sistema iniciado.

> _
```

### Comandos disponíveis:

```
> listar          # Ver todos os serviços agendados
> utiliz          # Ver utilizadores conectados
> frota           # Ver estado dos veículos ativos
> cancelar <id>   # Cancelar serviço (0 = todos)
> km              # Total de km percorridos
> hora            # Tempo simulado atual
> terminar        # Encerrar todo o sistema
```

---

## 📺 TERMINAL 2 - CLIENTE 1 (João)

### Passo 1: Navegar para a pasta (novo terminal)

```bash
cd '/mnt/c/Users/H0LF/Documents/SO2025/so_2425_tp_JoaoPais_2020131717_RuiMarques_2021123220'
```

### Passo 2: Iniciar cliente

```bash
./cliente joao
```

**Saída esperada:**

```
=== CLIENTE DE TRANSPORTE ===
A conectar como joao...
Bem-vindo joao!
Digite 'ajuda' para ver os comandos disponíveis.

joao> _
```

### Exemplo de uso:

```bash
joao> agendar 10 Coimbra 50
# Saída: Serviço agendado com ID 1 para hora 10

joao> agendar 20 Porto 30
# Saída: Serviço agendado com ID 2 para hora 20

joao> consultar
# Saída: Lista dos serviços agendados por joao

# Esperar que o tempo chegue a 10 segundos...
# O veículo vai contactar quando chegar

# Quando receber mensagem do veículo:
joao> entrar Lisboa

# Durante a viagem, pode sair a meio:
joao> sair

# Ou deixar completar até ao destino

joao> terminar
```

---

## 📺 TERMINAL 3 - CLIENTE 2 (Maria)

### Passo 1: Navegar para a pasta (novo terminal)

```bash
cd '/mnt/c/Users/H0LF/Documents/SO2025/so_2425_tp_JoaoPais_2020131717_RuiMarques_2021123220'
```

### Passo 2: Iniciar cliente

```bash
./cliente maria
```

**Saída esperada:**

```
=== CLIENTE DE TRANSPORTE ===
A conectar como maria...
Bem-vindo maria!
Digite 'ajuda' para ver os comandos disponíveis.

maria> _
```

### Exemplo de uso:

```bash
maria> agendar 15 Lisboa 40
# Saída: Serviço agendado com ID 3 para hora 15

maria> consultar
# Saída: Lista dos serviços agendados por maria

maria> cancelar 3
# Saída: Serviço 3 cancelado

maria> terminar
```

---

## 🎬 FLUXO COMPLETO DE TESTE

### Fase 1: Inicialização (0s)

1. **Terminal 1**: Iniciar controlador
2. **Terminal 2**: Conectar cliente joao
3. **Terminal 3**: Conectar cliente maria

### Fase 2: Agendamentos (0-5s)

4. **Terminal 2** (joao): `agendar 10 Coimbra 50`
5. **Terminal 3** (maria): `agendar 15 Porto 30`
6. **Terminal 1**: `listar` → Ver os 2 serviços

### Fase 3: Execução do Serviço (10s)

7. **Aguardar** até tempo = 10s (controlador lança veículo automaticamente)
8. **Terminal 2** receberá: `[VEÍCULO] Veículo chegou ao local: Coimbra`
9. **Terminal 2** (joao): `entrar Lisboa`
10. **Veículo** reporta: `ENTROU`, `PERCENTAGEM:10`, `PERCENTAGEM:20`, ...
11. **Terminal 1**: `frota` → Ver veículo ativo

### Fase 4: Monitorização (durante viagem)

12. **Terminal 1**: `utiliz` → Ver joão em viagem
13. **Terminal 1**: `frota` → Ver percentagem
14. **Terminal 1**: `km` → Ver km percorridos (vai aumentando)

### Fase 5: Conclusão

15. Veículo reporta `PERCENTAGEM:100` e `CHEGOU`
16. **Terminal 2** recebe: `[VEÍCULO] Chegou ao destino: Lisboa`
17. **Terminal 1**: `km` → Ver total atualizado
18. **Terminal 2** (joao): `terminar`
19. **Terminal 3** (maria): `terminar`
20. **Terminal 1**: `terminar` → Encerra todo o sistema

---

## 🧪 TESTES ESPECÍFICOS

### Teste 1: Limite de Utilizadores (30)

Abrir 30 terminais e conectar 30 clientes diferentes. O 31º deve ser rejeitado.

### Teste 2: Limite de Veículos (NVEICULOS)

```bash
export NVEICULOS=2
./controlador
```

Agendar 3 serviços para a mesma hora. Apenas 2 veículos lançados, o 3º fica em espera.

### Teste 3: Cancelamento com SIGUSR1

```bash
# Terminal 1 (durante viagem):
cancelar 1

# Terminal 2 receberá: [VEÍCULO] Viagem cancelada
```

### Teste 4: Cliente sai a meio

```bash
# Terminal 2 (durante viagem):
sair

# Veículo reporta: SAIU: Cliente saiu a meio da viagem (40%)
```

### Teste 5: Select() no Cliente

```bash
# Terminal 2:
# Enquanto escreves um comando, deves receber mensagens do veículo
# sem interromper o que estás a escrever
```

---

## 🐛 TROUBLESHOOTING

### Problema: "Controlador não está em execução"

**Solução**: Iniciar primeiro o Terminal 1 (controlador)

### Problema: "Username já em uso"

**Solução**: Usar username diferente ou terminar o cliente anterior

### Problema: "Limite de utilizadores atingido"

**Solução**: Terminar alguns clientes ou reiniciar controlador

### Problema: FIFOs não são removidos

**Solução**:

```bash
rm -f /tmp/fifo_controlador /tmp/fifo_cliente_* /tmp/fifo_veiculo_*
```

### Problema: Processos "zombie"

**Solução**:

```bash
pkill controlador
pkill cliente
pkill veiculo
```

---

## 📝 OBSERVAÇÕES IMPORTANTES

1. **Ordem de execução**: SEMPRE iniciar o controlador primeiro
2. **Tempo simulado**: Cada segundo real = 1 segundo simulado
3. **Veículos automáticos**: São lançados pelo controlador, não executar manualmente
4. **Named pipes**: Criados/removidos automaticamente
5. **Telemetria**: Visível no terminal do controlador (stdout do veículo)
6. **Select()**: Cliente consegue receber mensagens enquanto digita comandos

---

## ✅ VERIFICAÇÃO DE FUNCIONAMENTO

Se tudo estiver correto, deves ver:

**Terminal 1 (Controlador):**

```
[LOGIN] Cliente joao conectado (PID 1234)
[AGENDAR] joao agendou serviço 1
[INFO] Veículo lançado (PID 5678) para serviço 1
VEICULO_LANCADO: ID=1, Cliente=joao, Local=Coimbra, Distancia=50km
ENTROU: Cliente entrou no veículo. Destino: Lisboa
PERCENTAGEM:10
PERCENTAGEM:20
...
CHEGOU: Cliente chegou ao destino: Lisboa
```

**Terminal 2 (Cliente joao):**

```
Bem-vindo joao!
joao> agendar 10 Coimbra 50
Serviço agendado com ID 1 para hora 10
joao> [VEÍCULO] Veículo chegou ao local: Coimbra. Use 'entrar <destino>' para iniciar.
joao> entrar Lisboa
Entrando no veículo. Destino: Lisboa
joao> [VEÍCULO] Chegou ao destino: Lisboa. Distância: 50 km
```

**Terminal 3 (Cliente maria):**

```
Bem-vindo maria!
maria> consultar
Nenhum serviço agendado.
```

---

## 🎯 COMANDOS RÁPIDOS

### Setup inicial (copiar/colar):

```bash
# Terminal 1
cd '/mnt/c/Users/H0LF/Documents/SO2025/so_2425_tp_JoaoPais_2020131717_RuiMarques_2021123220'
export NVEICULOS=3
./controlador

# Terminal 2 (novo terminal)
cd '/mnt/c/Users/H0LF/Documents/SO2025/so_2425_tp_JoaoPais_2020131717_RuiMarques_2021123220'
./cliente joao

# Terminal 3 (novo terminal)
cd '/mnt/c/Users/H0LF/Documents/SO2025/so_2425_tp_JoaoPais_2020131717_RuiMarques_2021123220'
./cliente maria
```

### Teste rápido (copiar/colar em cada terminal):

```bash
# Terminal 2 (joao)
agendar 5 Coimbra 20
consultar

# Terminal 3 (maria)
agendar 10 Porto 30
consultar

# Terminal 1 (controlador)
listar
utiliz

# Aguardar 5 segundos...

# Terminal 2 (quando veículo chegar)
entrar Lisboa

# Terminal 1 (durante viagem)
frota
km
```

---

**📚 Documentação completa em README.md**
**✅ Verificação de requisitos em VERIFICACAO_100_PERCENT.md**
