CC = gcc
CFLAGS = -O3

all: interativo aplicador

interativo: interativo.c
	$(CC) $(CFLAGS) interativo.c -o interativo

aplicador: aplicador.c
	$(CC) $(CFLAGS) aplicador.c -o aplicador

clean:
	rm -f interativo aplicador