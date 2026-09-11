# Compiladores e flags
CC       = gcc
CFLAGS   = -Wall -Wextra -O3 -std=c99
OMPFLAGS = -fopenmp

# Arquivos executáveis finais
EXEC_SEQ = fire_seq
EXEC_OMP = fire_omp

# Fontes
SRC_SEQ  = fire_seq.c
SRC_OMP  = fire_omp.c

# Alvo padrão: compila as duas versões
all: $(EXEC_SEQ) $(EXEC_OMP)

# Compilação da versão Sequencial
# Compilação da versão Sequencial
$(EXEC_SEQ): $(SRC_SEQ)
	$(CC) $(CFLAGS) $(OMPFLAGS) $(SRC_SEQ) -o $(EXEC_SEQ)

# Compilação da versão Paralela (OpenMP)
$(EXEC_OMP): $(SRC_OMP)
	$(CC) $(CFLAGS) $(OMPFLAGS) $(SRC_OMP) -o $(EXEC_OMP)

# Alvos práticos para rodar testes rápidos no cluster
run-seq: $(EXEC_SEQ)
	./$(EXEC_SEQ) entrada_carga_media.txt > saida_seq.txt

run-omp: $(EXEC_OMP)
	./$(EXEC_OMP) entrada_carga_media.txt > saida_omp.txt

# Limpeza dos binários gerados
clean:
	rm -f $(EXEC_SEQ) $(EXEC_OMP) *.o

.PHONY: all clean run-seq run-omp
