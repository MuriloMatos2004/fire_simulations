#!/bin/bash

# Diretórios e binários
INPUT_FILE=${1:-"tests/entrada_pequena.txt"}
SEQ_BIN="./fire_seq"
OMP_BIN="./fire_omp"

if [ ! -f "$SEQ_BIN" ] || [ ! -f "$OMP_BIN" ]; then
    echo "[-] Erro: Compilados não encontrados. Execute 'make' na raiz do projeto primeiro."
    exit 1
fi

if [ ! -f "$INPUT_FILE" ]; then
    echo "[-] Erro: Arquivo de entrada '$INPUT_FILE' não encontrado."
    exit 1
fi

echo "=================================================="
echo " Validando Correção Paralela (Checksum Comparison) "
echo " Arquivo de Entrada: $INPUT_FILE"
echo "=================================================="

# Executa sequencial e extrai checksum
OUT_SEQ=$($SEQ_BIN "$INPUT_FILE")
CHECKSUM_SEQ=$(echo "$OUT_SEQ" | grep -i "checksum" | awk '{print $NF}')

# Executa paralelo com 8 threads e extrai checksum
export OMP_NUM_THREADS=8
export OMP_SCHEDULE="guided,512"
OUT_OMP=$($OMP_BIN "$INPUT_FILE")
CHECKSUM_OMP=$(echo "$OUT_OMP" | grep -i "checksum" | awk '{print $NF}')

echo "Checksum Sequencial : $CHECKSUM_SEQ"
echo "Checksum Paralelo   : $CHECKSUM_OMP"

if [ "$CHECKSUM_SEQ" == "$CHECKSUM_OMP" ] && [ -n "$CHECKSUM_SEQ" ]; then
    echo -e "\n[SUCCESS] VALIDAÇÃO CONCLUÍDA: As saídas são IDÊNTICAS!"
    exit 0
else
    echo -e "\n[FAIL] ERRO DE VALIDAÇÃO: As saídas DIVERGEM!"
    exit 1
fi
