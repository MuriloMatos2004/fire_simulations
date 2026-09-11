#!/bin/bash

# Configurações de ambiente
mkdir -p results/raw
RAW_OUT="results/raw/execucoes_raw.txt"
CSV_OUT="results/raw/tempos_execucao.csv"

echo "=== INICIANDO SUÍTE DE BENCHMARKS ===" | tee "$RAW_OUT"
echo "carga,versao,politica,tempo_segundos" > "$CSV_OUT"

# Mapeamento dos arquivos de teste existentes
declare -A TESTES
TESTES["pequena"]="tests/entrada_carga_pequena.txt"
TESTES["media"]="tests/entrada_carga_media.txt"
TESTES["grande"]="tests/entrada_carga_grande.txt"

export OMP_NUM_THREADS=8

for CARGA in "pequena" "media" "grande"; do
    ARQ=${TESTES[$CARGA]}
    
    if [ ! -f "$ARQ" ]; then
        echo "[-] Arquivo $ARQ não encontrado, pulando..."
        continue
    fi

    echo -e "\n--------------------------------------------------" | tee -a "$RAW_OUT"
    echo " Executando Carga: $CARGA ($ARQ)" | tee -a "$RAW_OUT"
    echo "--------------------------------------------------" | tee -a "$RAW_OUT"

    # 1. Sequencial
    echo -n "[1/3] Executando Sequencial... "
    OUT_SEQ=$(./fire_seq "$ARQ")
    TEMPO_SEQ=$(echo "$OUT_SEQ" | grep -i "tempo" | awk '{print $NF}')
    echo "$TEMPO_SEQ s"
    echo "$OUT_SEQ" >> "$RAW_OUT"
    echo "$CARGA,sequencial,none,$TEMPO_SEQ" >> "$CSV_OUT"

    # 2. OpenMP Dynamic (512)
    echo -n "[2/3] Executando OMP (dynamic,512)... "
    export OMP_SCHEDULE="dynamic,512"
    OUT_DYN=$(./fire_omp "$ARQ")
    TEMPO_DYN=$(echo "$OUT_DYN" | grep -i "tempo" | awk '{print $NF}')
    echo "$TEMPO_DYN s"
    echo "$OUT_DYN" >> "$RAW_OUT"
    echo "$CARGA,paralelo,dynamic,512,$TEMPO_DYN" >> "$CSV_OUT"

    # 3. OpenMP Guided (512)
    echo -n "[3/3] Executando OMP (guided,512)... "
    export OMP_SCHEDULE="guided,512"
    OUT_GUI=$(./fire_omp "$ARQ")
    TEMPO_GUI=$(echo "$OUT_GUI" | grep -i "tempo" | awk '{print $NF}')
    echo "$TEMPO_GUI s"
    echo "$OUT_GUI" >> "$RAW_OUT"
    echo "$CARGA,paralelo,guided,512,$TEMPO_GUI" >> "$CSV_OUT"
done

echo -e "\n[+] Benchmark finalizado!"
echo "[+] Dados salvos em '$RAW_OUT' e '$CSV_OUT'."
