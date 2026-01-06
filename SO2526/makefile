CC = gcc
CFLAGS = -Wall -pthread

all: controlador cliente veiculo

controlador: controlador.c utils.c utils.h
	$(CC) $(CFLAGS) -o controlador controlador.c utils.c

cliente: cliente.c utils.h
	$(CC) $(CFLAGS) -o cliente cliente.c

veiculo: veiculo.c utils.h
	$(CC) $(CFLAGS) -o veiculo veiculo.c

clean:
	rm -f *.o controlador cliente veiculo
	rm -f /tmp/fifo_controlador /tmp/fifo_cliente_* /tmp/fifo_veiculo_*

.PHONY: all clean
