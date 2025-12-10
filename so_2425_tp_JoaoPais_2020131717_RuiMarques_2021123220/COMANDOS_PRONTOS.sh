#!/bin/bash
# ═══════════════════════════════════════════════════════════
# COMANDOS PRONTOS - SISTEMA DE VEÍCULOS AUTÓNOMOS
# ═══════════════════════════════════════════════════════════
# 
# Este ficheiro contém todos os comandos prontos para copiar e colar
# 
# INSTRUÇÕES:
# 1. Abre 3 terminais WSL
# 2. Copia e cola os comandos de cada secção no terminal correspondente
# 3. Segue a ordem indicada
# 
# ═══════════════════════════════════════════════════════════

# ═══════════════════════════════════════════════════════════
# TERMINAL 1 - CONTROLADOR
# ═══════════════════════════════════════════════════════════

# Copiar TUDO (selecionar e Ctrl+C, depois Ctrl+V no terminal):

cd '/mnt/c/Users/H0LF/Documents/SO2025/so_2425_tp_JoaoPais_2020131717_RuiMarques_2021123220'
export NVEICULOS=3
./controlador

# Depois de iniciar, podes testar estes comandos (um de cada vez):
listar
utiliz
frota
km
hora


# ═══════════════════════════════════════════════════════════
# TERMINAL 2 - CLIENTE JOÃO
# ═══════════════════════════════════════════════════════════

# Copiar TUDO (selecionar e Ctrl+C, depois Ctrl+V no terminal):

cd '/mnt/c/Users/H0LF/Documents/SO2025/so_2425_tp_JoaoPais_2020131717_RuiMarques_2021123220'
./cliente joao

# Depois de conectar, testa estes comandos (um de cada vez):
agendar 10 Coimbra 50
agendar 20 Porto 30
consultar

# Quando o veículo chegar (após 10 segundos):
entrar Lisboa

# Durante a viagem, podes:
sair

# Ou deixar completar e depois:
terminar


# ═══════════════════════════════════════════════════════════
# TERMINAL 3 - CLIENTE MARIA
# ═══════════════════════════════════════════════════════════

# Copiar TUDO (selecionar e Ctrl+C, depois Ctrl+V no terminal):

cd '/mnt/c/Users/H0LF/Documents/SO2025/so_2425_tp_JoaoPais_2020131717_RuiMarques_2021123220'
./cliente maria

# Depois de conectar, testa estes comandos:
agendar 15 Lisboa 40
consultar
cancelar 3
terminar


# ═══════════════════════════════════════════════════════════
# TESTE COMPLETO PASSO A PASSO
# ═══════════════════════════════════════════════════════════

# PASSO 1: Terminal 1 - Iniciar controlador
cd '/mnt/c/Users/H0LF/Documents/SO2025/so_2425_tp_JoaoPais_2020131717_RuiMarques_2021123220'
export NVEICULOS=3
./controlador

# PASSO 2: Terminal 2 - Conectar João
cd '/mnt/c/Users/H0LF/Documents/SO2025/so_2425_tp_JoaoPais_2020131717_RuiMarques_2021123220'
./cliente joao

# PASSO 3: Terminal 3 - Conectar Maria
cd '/mnt/c/Users/H0LF/Documents/SO2025/so_2425_tp_JoaoPais_2020131717_RuiMarques_2021123220'
./cliente maria

# PASSO 4: Terminal 2 (João) - Agendar
agendar 5 Coimbra 20

# PASSO 5: Terminal 3 (Maria) - Agendar
agendar 10 Porto 30

# PASSO 6: Terminal 1 (Controlador) - Ver serviços
listar

# PASSO 7: Terminal 1 (Controlador) - Ver utilizadores
utiliz

# PASSO 8: Aguardar 5 segundos (o veículo vai ser lançado automaticamente)

# PASSO 9: Terminal 2 (João) - Quando receber mensagem do veículo
entrar Lisboa

# PASSO 10: Terminal 1 (Controlador) - Ver frota ativa
frota

# PASSO 11: Terminal 1 (Controlador) - Ver km percorridos
km

# PASSO 12: Aguardar viagem completar

# PASSO 13: Terminal 2 (João) - Sair
terminar

# PASSO 14: Terminal 3 (Maria) - Sair
terminar

# PASSO 15: Terminal 1 (Controlador) - Encerrar sistema
terminar


# ═══════════════════════════════════════════════════════════
# TESTE DE CANCELAMENTO
# ═══════════════════════════════════════════════════════════

# Terminal 2 (João):
agendar 30 Faro 100

# Terminal 1 (Controlador) - Cancelar serviço
cancelar 1

# Terminal 1 (Controlador) - Cancelar TODOS os serviços
cancelar 0


# ═══════════════════════════════════════════════════════════
# COMPILAÇÃO E VERIFICAÇÃO
# ═══════════════════════════════════════════════════════════

# Compilar o projeto:
cd '/mnt/c/Users/H0LF/Documents/SO2025/so_2425_tp_JoaoPais_2020131717_RuiMarques_2021123220'
make clean
make

# Verificar executáveis:
ls -lh controlador cliente veiculo

# Contar linhas de código:
wc -l *.c utils.h

# Verificar limites:
grep "define MAX_" utils.h

# Verificar NVEICULOS:
grep "getenv.*NVEICULOS" utils.c

# Verificar select():
grep -c "select(" cliente.c
grep -c "select(" controlador.c


# ═══════════════════════════════════════════════════════════
# LIMPEZA (se necessário)
# ═══════════════════════════════════════════════════════════

# Matar todos os processos:
pkill controlador
pkill cliente
pkill veiculo

# Remover FIFOs:
rm -f /tmp/fifo_controlador /tmp/fifo_cliente_* /tmp/fifo_veiculo_*

# Limpar executáveis:
make clean


# ═══════════════════════════════════════════════════════════
# TESTE DE LIMITES
# ═══════════════════════════════════════════════════════════

# Teste 1: Testar NVEICULOS=2 (apenas 2 veículos simultâneos)
export NVEICULOS=2
./controlador

# Depois agendar 3 serviços para a mesma hora em clientes diferentes
# Apenas 2 veículos serão lançados simultaneamente

# Teste 2: Testar 30 utilizadores (máximo)
# Abrir 30 terminais e conectar 30 clientes com usernames diferentes
# O 31º deve ser rejeitado


# ═══════════════════════════════════════════════════════════
# DEMONSTRAÇÃO RÁPIDA (5 MINUTOS)
# ═══════════════════════════════════════════════════════════

# TERMINAL 1:
cd '/mnt/c/Users/H0LF/Documents/SO2025/so_2425_tp_JoaoPais_2020131717_RuiMarques_2021123220'
export NVEICULOS=3
./controlador

# TERMINAL 2:
cd '/mnt/c/Users/H0LF/Documents/SO2025/so_2425_tp_JoaoPais_2020131717_RuiMarques_2021123220'
./cliente joao

# TERMINAL 3:
cd '/mnt/c/Users/H0LF/Documents/SO2025/so_2425_tp_JoaoPais_2020131717_RuiMarques_2021123220'
./cliente maria

# Terminal 2: agendar 3 Coimbra 15
# Terminal 3: agendar 5 Porto 20
# Terminal 1: listar
# (aguardar 3 segundos)
# Terminal 2: entrar Lisboa
# Terminal 1: frota
# Terminal 1: km
# (aguardar viagem)
# Terminal 2: terminar
# Terminal 3: terminar
# Terminal 1: terminar


# ═══════════════════════════════════════════════════════════
# FIM - COMANDOS PRONTOS
# ═══════════════════════════════════════════════════════════
