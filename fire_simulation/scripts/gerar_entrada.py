#!/usr/bin/env python3
import sys
import random
import os

# Perfis de teste do projeto
CARGAS_PADRAO = {
    "pequena": {"l": 400,  "c": 500,  "passos": 80,  "focos": 5,  "seed": 2027, "param": 35},
    "media":   {"l": 1500, "c": 1500, "passos": 100, "focos": 15, "seed": 2027, "param": 35},
    "grande":  {"l": 3000, "c": 3000, "passos": 100, "focos": 30, "seed": 2027, "param": 35}
}

def gerar_entrada_trabalho(linhas, colunas, passos, num_focos, seed, param, arquivo_saida):
    diretorio = os.path.dirname(arquivo_saida)
    if diretorio:
        os.makedirs(diretorio, exist_ok=True)

    random.seed(seed)

    # Gera coordenadas X e Y separadas para os focos
    focos_x = [random.randint(0, colunas - 1) for _ in range(num_focos)]
    focos_y = [random.randint(0, linhas - 1) for _ in range(num_focos)]

    with open(arquivo_saida, 'w') as f:
        # Linha 1: Parâmetros globais
        f.write(f"{linhas} {colunas} {passos} {num_focos} {seed} {param}\n")
        
        # Linha 2: Configuração de terreno/estados
        f.write("0 1 3\n")
        
        # Linha 3: Matriz/Vento
        f.write("3 2\n")
        
        # Linhas intermediárias de suporte (ex: pares fixos)
        f.write("100 83\n")
        f.write("200 250\n")
        f.write("300 416\n")
        
        # Penúltima linha: Lista de coordenadas X
        f.write(" ".join(map(str, focos_x)) + "\n")
        
        # Última linha: Lista de coordenadas Y
        f.write(" ".join(map(str, focos_y)) + "\n")

    print(f"[+] Entrada padronizada gerada em '{arquivo_saida}'.")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Uso: python3 scripts/gerar_entrada.py <pequena|media|grande> [arquivo_saida]")
        sys.exit(1)

    tipo = sys.argv[1].lower()
    if tipo in CARGAS_PADRAO:
        p = CARGAS_PADRAO[tipo]
        saida = sys.argv[2] if len(sys.argv) > 2 else f"tests/entrada_{tipo}.txt"
        gerar_entrada_trabalho(p["l"], p["c"], p["passos"], p["focos"], p["seed"], p["param"], saida)
    else:
        print("[-] Escolha entre: pequena, media ou grande.")
        sys.exit(1)
