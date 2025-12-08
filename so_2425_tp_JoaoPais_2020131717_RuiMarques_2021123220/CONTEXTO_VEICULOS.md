# Sistema de Gestão de Frota de Veículos - Contextualização

## 📋 Visão Geral

Este projeto implementa uma **Central de Controlo de Frota** que gere veículos em tempo real, permitindo comunicação bidirecional entre a central e os veículos através de rotas e zonas geográficas.

---

## 🚗 Componentes do Sistema

### 1. **Central de Controlo (Manager)**

- Monitoriza todos os veículos ativos na frota
- Gere rotas e zonas de circulação
- Recebe e distribui alertas entre veículos
- Controla acesso a rotas (pode bloquear/fechar rotas)
- Mantém histórico de alertas persistentes

### 2. **Interface do Veículo (Feed)**

- Conecta veículos individuais à central
- Permite atribuição a rotas/zonas específicas
- Envia alertas de trânsito e eventos
- Recebe notificações de outros veículos na mesma rota
- Mantém comunicação em tempo real com a central

---

## 🛣️ Conceitos Adaptados ao Tema

| Conceito Técnico         | Aplicação em Veículos                      |
| ------------------------ | ------------------------------------------ |
| **Tópico**               | Rota, Zona Geográfica, Linha de Transporte |
| **Subscribe**            | Veículo atribuído a uma rota/zona          |
| **Mensagem**             | Alerta de trânsito, evento na via          |
| **Mensagem Persistente** | Obra na via, trânsito prolongado           |
| **Mensagem Instantânea** | Acidente pontual, perigo imediato          |
| **Bloqueio de Tópico**   | Rota fechada/bloqueada                     |
| **Utilizador**           | Veículo/Motorista                          |

---

## 📡 Cenários de Uso

### Cenário 1: Autocarro em Serviço

```bash
$ ./feed Autocarro_101
feed> subscribe Rota_Centro_Baixa
[CENTRAL]: Atribuído à Rota_Centro_Baixa

# Autocarro detecta trânsito intenso
feed> msg Rota_Centro_Baixa 600 Transito muito intenso na Av. Liberdade
[CENTRAL]: Alerta enviado aos veículos na Rota_Centro_Baixa
```

### Cenário 2: Ambulância em Emergência

```bash
$ ./feed Ambulancia_INEM5
feed> subscribe Rota_Centro_Baixa

# Recebe alerta de trânsito do autocarro
[Rota_Centro_Baixa (Autocarro_101)]: Transito muito intenso na Av. Liberdade

# Reporta acidente
feed> msg Rota_Centro_Baixa 0 EMERGENCIA! Acidente grave - via bloqueada
```

### Cenário 3: Táxi Recebe Notificações

```bash
$ ./feed Taxi_42
feed> subscribe Rota_Centro_Baixa

# Recebe automaticamente alertas persistentes existentes
[CENTRAL]: Alertas persistentes na Rota_Centro_Baixa:
  - [Autocarro_101] Transito muito intenso na Av. Liberdade

# Recebe notificação de emergência
[Rota_Centro_Baixa (Ambulancia_INEM5)]: EMERGENCIA! Acidente grave - via bloqueada
```

### Cenário 4: Central Bloqueia Rota

```bash
Manager > lock Rota_Centro_Baixa
[INFO]: Rota 'Rota_Centro_Baixa' foi bloqueada

# Veículos não podem enviar mais alertas
# (Simula rota oficialmente fechada)
```

---

## ⏱️ Gestão Temporal de Alertas

### Alertas Instantâneos (duracao = 0)

- **Uso**: Eventos pontuais
- **Exemplos**:
  - Acidente acabado de acontecer
  - Veículo avariado
  - Perigo imediato na via
- **Comportamento**: Enviados imediatamente, não persistem

### Alertas Persistentes (duracao > 0)

- **Uso**: Condições prolongadas
- **Exemplos**:
  - Obras na via (3600s = 1 hora)
  - Trânsito intenso (1800s = 30 min)
  - Evento programado (7200s = 2 horas)
- **Comportamento**:
  - Enviados a veículos que entrarem na rota posteriormente
  - Expiram automaticamente após o tempo definido
  - Central notifica quando expiram

---

## 🎯 Funcionalidades da Central

### Monitorização

```bash
Manager > users
[INFO]: Veículos ativos na frota:
 - Autocarro_101 (PID: 1234)
 - Ambulancia_INEM5 (PID: 1235)
 - Taxi_42 (PID: 1236)
```

### Gestão de Rotas

```bash
Manager > topics
[INFO]: Rotas ativas:
 > Rota: 'Rota_Centro_Baixa'
   - Alertas Persistentes: 2
 > Rota: 'Rota_Aeroporto'
   - Alertas Persistentes: 0
```

### Consulta de Alertas

```bash
Manager > show Rota_Centro_Baixa
[INFO]: Alertas na rota 'Rota_Centro_Baixa':
 - Transito muito intenso na Av. Liberdade
 - Obras na faixa da direita - desvio ativo
```

### Controlo de Acesso

```bash
Manager > lock Rota_Ponte_25Abril
[INFO]: Rota 'Rota_Ponte_25Abril' bloqueada

Manager > unlock Rota_Ponte_25Abril
[INFO]: Rota 'Rota_Ponte_25Abril' reaberta
```

### Remoção de Veículo

```bash
Manager > remove Taxi_42
[INFO]: Veículo 'Taxi_42' removido da frota
```

---

## 💾 Persistência de Dados

O sistema guarda alertas persistentes em ficheiro quando a central é encerrada:

```txt
Rota_Centro_Baixa Autocarro_101 450 Transito muito intenso na Av. Liberdade
Rota_Aeroporto Taxi_15 900 Via expressa em obras - usar alternativa
```

Ao reiniciar, a central carrega automaticamente estes alertas.

---

## 🔒 Segurança e Validações

1. **Apenas veículos atribuídos podem enviar alertas**
   - Veículo deve fazer `subscribe` antes de enviar mensagens
2. **Rotas bloqueadas rejeitam novos alertas**
   - Simula rota oficialmente fechada
3. **Limite de veículos simultâneos: 10**
   - Controlo de capacidade da frota
4. **Limite de rotas ativas: 20**
   - Gestão eficiente de zonas
5. **Máximo 5 alertas persistentes por rota**
   - Previne sobrecarga de informação

---

## 🚀 Vantagens do Sistema

✅ **Comunicação em Tempo Real**: Veículos recebem alertas instantaneamente  
✅ **Histórico de Eventos**: Alertas persistentes informam veículos que chegam depois  
✅ **Gestão Centralizada**: Central controla toda a frota  
✅ **Escalabilidade**: Suporta múltiplos veículos e rotas  
✅ **Tolerância a Falhas**: Dados persistidos em ficheiro  
✅ **Thread-Safe**: Sincronização garante consistência

---

## 🎓 Aplicação Prática

Este sistema pode ser adaptado para:

- **Frotas de Transporte Público**: Autocarros, metro
- **Serviços de Emergência**: Ambulâncias, bombeiros
- **Táxis e TVDE**: Uber, Bolt
- **Logística**: Camiões de entregas
- **Gestão de Tráfego Urbano**: Coordenação de vias

---

**Conclusão**: O sistema implementa um sistema robusto de gestão de frota com comunicação em tempo real, adaptado especificamente ao contexto de veículos e rotas.
