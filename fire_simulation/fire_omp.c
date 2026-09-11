#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

typedef struct {
    int linha;
    int coluna;
} Foco;

typedef struct {
    int passo;
    int l_ini;
    int c_ini;
    int l_fim;
    int c_fim;
} Zona;

// Função auxiliar para calcular abs em inteiros
static inline int abs_int(int x) {
    return x < 0 ? -x : x;
}

// Função auxiliar para calcular o máximo entre dois inteiros
static inline int max_int(int a, int b) {
    return a > b ? a : b;
}

int main(int argc, char *argv[]) {
    /* =========================================================================
     * FASE 1: VALIDAÇÃO DE ARGUMENTOS E ARQUIVO
     * ========================================================================= */
    if (argc != 2) {
        fprintf(stderr, "Erro: Uso correto %s <arquivo_entrada>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "r");
    if (!f) {
        fprintf(stderr, "Erro: Nao foi possivel abrir o arquivo %s\n", argv[1]);
        return 1;
    }

    /* =========================================================================
     * FASE 2: LEITURA E VALIDAÇÃO DO CONTEÚDO
     * ========================================================================= */
    long long L, C;
    int P, T, LIMIAR;
    unsigned int seed;

    if (fscanf(f, "%lld %lld %d %d %u %d", &L, &C, &P, &T, &seed, &LIMIAR) != 6) {
        fprintf(stderr, "Erro ao ler a linha 1\n");
        fclose(f);
        return 1;
    }

    if (L <= 0 || C <= 0 || P < 0 || T <= 0 || LIMIAR <= 0) {
        fprintf(stderr, "Erro: Parametros da linha 1 invalidos\n");
        fclose(f);
        return 1;
    }

    int vento_linha, vento_coluna, intensidade;
    if (fscanf(f, "%d %d %d", &vento_linha, &vento_coluna, &intensidade) != 3) {
        fprintf(stderr, "Erro ao ler a linha 2\n");
        fclose(f);
        return 1;
    }

    if (vento_linha < -1 || vento_linha > 1 || vento_coluna < -1 || vento_coluna > 1) {
        fprintf(stderr, "Erro: Componentes do vento invalidas\n");
        fclose(f);
        return 1;
    }
    if (vento_linha == 0 && vento_coluna == 0) {
        fprintf(stderr, "Erro: Direcao do vento nao pode ser (0,0)\n");
        fclose(f);
        return 1;
    }
    if (intensidade < 0 || intensidade > 5) {
        fprintf(stderr, "Erro: Intensidade do vento invalida\n");
        fclose(f);
        return 1;
    }

    int F, Z;
    if (fscanf(f, "%d %d", &F, &Z) != 2 || F < 0 || Z < 0) {
        fprintf(stderr, "Erro ao ler a linha 3 ou F/Z invalidos\n");
        fclose(f);
        return 1;
    }

    Foco *focos = NULL;
    if (F > 0) {
        focos = (Foco *) malloc(F * sizeof(Foco));
        if (!focos) {
            fprintf(stderr, "Erro ao alocar memoria para focos\n");
            fclose(f);
            return 1;
        }
        for (int i = 0; i < F; i++) {
            if (fscanf(f, "%d %d", &focos[i].linha, &focos[i].coluna) != 2) {
                fprintf(stderr, "Erro ao ler foco %d\n", i);
                free(focos);
                fclose(f);
                return 1;
            }
            if (focos[i].linha < 0 || focos[i].linha >= L || focos[i].coluna < 0 || focos[i].coluna >= C) {
                fprintf(stderr, "Erro: Foco fora dos limites da matriz\n");
                free(focos);
                fclose(f);
                return 1;
            }
            for (int j = 0; j < i; j++) {
                if (focos[i].linha == focos[j].linha && focos[i].coluna == focos[j].coluna) {
                    fprintf(stderr, "Erro: Foco repetido detectado\n");
                    free(focos);
                    fclose(f);
                    return 1;
                }
            }
        }
    }

    Zona *zonas = NULL;
    if (Z > 0) {
        zonas = (Zona *) malloc(Z * sizeof(Zona));
        if (!zonas) {
            fprintf(stderr, "Erro ao alocar memoria para zonas\n");
            free(focos);
            fclose(f);
            return 1;
        }
        for (int i = 0; i < Z; i++) {
            if (fscanf(f, "%d %d %d %d %d", &zonas[i].passo, &zonas[i].l_ini, &zonas[i].c_ini, &zonas[i].l_fim, &zonas[i].c_fim) != 5) {
                fprintf(stderr, "Erro ao ler zona %d\n", i);
                free(focos); free(zonas);
                fclose(f);
                return 1;
            }
            if (zonas[i].l_ini < 0 || zonas[i].l_ini >= L ||
                zonas[i].l_fim < 0 || zonas[i].l_fim >= L ||
                zonas[i].c_ini < 0 || zonas[i].c_ini >= C ||
                zonas[i].c_fim < 0 || zonas[i].c_fim >= C ||
                zonas[i].l_ini > zonas[i].l_fim || zonas[i].c_ini > zonas[i].c_fim) {
                fprintf(stderr, "Erro: Limites da zona invalidos\n");
                free(focos); free(zonas);
                fclose(f);
                return 1;
            }
            if (zonas[i].passo < 0 || zonas[i].passo >= P) {
                fprintf(stderr, "Erro: Passo de ativacao da zona invalido\n");
                free(focos); free(zonas);
                fclose(f);
                return 1;
            }
        }
    }

    fclose(f);

    /* =========================================================================
     * FASE 3: ALOCAÇÃO DE MEMÓRIA
     * ========================================================================= */
    long long num_celulas = L * C;
    int *cobertura      = (int *) malloc(num_celulas * sizeof(int));
    int *umidade        = (int *) malloc(num_celulas * sizeof(int));
    int *estado_atual   = (int *) malloc(num_celulas * sizeof(int));
    int *proximo_estado = (int *) malloc(num_celulas * sizeof(int));
    int *tempo_atual    = (int *) malloc(num_celulas * sizeof(int));
    int *proximo_tempo  = (int *) malloc(num_celulas * sizeof(int));
    int *ativacao       = (int *) malloc(num_celulas * sizeof(int));

    if (!cobertura || !umidade || !estado_atual || !proximo_estado || !tempo_atual || !proximo_tempo || !ativacao) {
        fprintf(stderr, "Erro na alocacao de memoria para a matriz/vetores da simulacao\n");
        return 1;
    }

    /* =========================================================================
     * FASE 4: GERAÇÃO DA MATRIZ (Sequencial e Determinística)
     * ========================================================================= */
    long long combustiveis_iniciais = 0;

    for (long long idx = 0; idx < num_celulas; idx++) {
        int val_cob = rand_r(&seed) % 100;
        int cob_type;
        if (val_cob < 10)       cob_type = 0; // Agua
        else if (val_cob < 20)  cob_type = 1; // Solo exposto
        else if (val_cob < 55)  cob_type = 2; // Vegetacao rasteira
        else                    cob_type = 3; // Floresta

        cobertura[idx] = cob_type;
        umidade[idx]   = rand_r(&seed) % 101;

        if (cob_type <= 1) {
            estado_atual[idx] = 0; // Nao combustivel
            tempo_atual[idx]  = 0;
        } else {
            estado_atual[idx] = 1; // Intacta
            tempo_atual[idx]  = (cob_type == 2) ? 2 : 4;
            combustiveis_iniciais++;
        }
    }

    /* =========================================================================
     * FASE 5: VALIDAÇÃO DOS FOCOS SOBRE A COBERTURA
     * ========================================================================= */
    for (int i = 0; i < F; i++) {
        long long idx = (long long)focos[i].linha * C + focos[i].coluna;
        if (cobertura[idx] <= 1) {
            fprintf(stderr, "Erro: Foco posicionado em celula nao combustivel (%d, %d)\n", focos[i].linha, focos[i].coluna);
            return 1;
        }
    }

    /* =========================================================================
     * FASE 6: APLICAÇÃO DOS FOCOS INICIAIS
     * ========================================================================= */
    for (int i = 0; i < F; i++) {
        long long idx = (long long)focos[i].linha * C + focos[i].coluna;
        estado_atual[idx] = 2; // Em chamas
    }

    /* =========================================================================
     * FASE 7: CONSTRUÇÃO DO MAPA DE ATIVAÇÃO
     * ========================================================================= */
    for (long long i = 0; i < num_celulas; i++) {
        ativacao[i] = -1;
    }

    for (int z = 0; z < Z; z++) {
        for (int r = zonas[z].l_ini; r <= zonas[z].l_fim; r++) {
            for (int c = zonas[z].c_ini; c <= zonas[z].c_fim; c++) {
                long long idx = (long long)r * C + c;
                if (ativacao[idx] == -1 || zonas[z].passo < ativacao[idx]) {
                    ativacao[idx] = zonas[z].passo;
                }
            }
        }
    }

    /* =========================================================================
     * FASE 8: LOOP DE SIMULAÇÃO (PARALELO OPENMP CRONOMETRADO)
     * ========================================================================= */
    double t_inicio = omp_get_wtime();

    long long total_ignicoes = 0;
    int melhor_passo = -1;
    long long melhor_quantidade = 0;
    int passo = 0;

    long long nao_combustiveis = 0;
    long long intactas = 0;
    long long em_chamas = 0;
    long long queimadas = 0;
    long long contencao = 0;

    for (long long i = 0; i < num_celulas; i++) {
        switch (estado_atual[i]) {
            case 0: nao_combustiveis++; break;
            case 1: intactas++;         break;
            case 2: em_chamas++;        break;
            case 3: queimadas++;        break;
            case 4: contencao++;        break;
        }
    }

    int existe_chama = (em_chamas > 0);

    static const int d_linha[8]  = {-1, -1, 0, 1, 1, 1, 0, -1};
    static const int d_coluna[8] = { 0,  1, 1, 1, 0,-1,-1, -1};

    // Variáveis usadas na reduction: precisam ser SHARED no contexto que
    // envolve a região paralela (por isso ficam declaradas aqui fora, e não
    // dentro do "#pragma omp parallel" -- declará-las lá dentro faz cada
    // thread ter automaticamente sua própria cópia privada, o que conflita
    // com a cláusula reduction do "omp for" e nem compila).
    long long l_nc, l_int, l_cha, l_que, l_cnt, novas_ignicoes_no_passo;

    // =========================================================================
    // REGIÃO PARALELA PERSISTENTE
    // adicionado num_threads(T) para respeitar o número de threads
    // especificado na entrada 
    // =========================================================================
    #pragma omp parallel num_threads(T) default(none) \
            shared(d_linha, d_coluna, L, C, P, num_celulas, LIMIAR, vento_linha, vento_coluna, intensidade, \
                   cobertura, umidade, estado_atual, proximo_estado, tempo_atual, proximo_tempo, \
                   ativacao, passo, existe_chama, total_ignicoes, \
                   melhor_passo, melhor_quantidade, nao_combustiveis, intactas, em_chamas, \
                   queimadas, contencao, \
                   l_nc, l_int, l_cha, l_que, l_cnt, novas_ignicoes_no_passo)
    {
        while (passo < P && existe_chama) {

            // 1. Ativação de zonas
            #pragma omp for simd schedule(runtime)
            for (long long i = 0; i < num_celulas; i++) {
                if (ativacao[i] == passo && estado_atual[i] == 1) {
                    estado_atual[i] = 4; // Contenção
                }
            }

            // Zera os contadores deste passo (uma única thread faz isso,
            // com barreira implícita ao final garantindo visibilidade
            // para todas as threads antes do próximo "omp for")
            #pragma omp single
            {
                l_nc = 0; l_int = 0; l_cha = 0; l_que = 0; l_cnt = 0;
                novas_ignicoes_no_passo = 0;
            }

            // 2 & 3. Cálculo dos novos estados e estatísticas
            #pragma omp for simd schedule(runtime) reduction(+: l_nc, l_int, l_cha, l_que, l_cnt, novas_ignicoes_no_passo)
            for (long long idx = 0; idx < num_celulas; idx++) {
                int r = idx / C;
                int c = idx % C;
                int st = estado_atual[idx];

                if (st == 0) { // Não combustível
                    proximo_estado[idx] = 0;
                    proximo_tempo[idx]  = 0;
                    l_nc++;
                } else if (st == 3) { // Queimada
                    proximo_estado[idx] = 3;
                    proximo_tempo[idx]  = 0;
                    l_que++;
                } else if (st == 4) { // Contenção
                    proximo_estado[idx] = 4;
                    proximo_tempo[idx]  = 0;
                    l_cnt++;
                } else if (st == 2) { // Em chamas
                    int novo_tempo = tempo_atual[idx] - 1;
                    if (novo_tempo == 0) {
                        proximo_estado[idx] = 3; // Passa a queimada
                        proximo_tempo[idx]  = 0;
                        l_que++;
                    } else {
                        proximo_estado[idx] = 2; // Continua em chamas
                        proximo_tempo[idx]  = novo_tempo;
                        l_cha++;
                    }
                } else if (st == 1) { // Intacta
                    int S = 0;
                    for (int k = 0; k < 8; k++) {
                        int nr = r + d_linha[k];
                        int nc = c + d_coluna[k];

                        if (nr >= 0 && nr < L && nc >= 0 && nc < C) {
                            long long n_idx = (long long)nr * C + nc;
                            if (estado_atual[n_idx] == 2) {
                                int prop_linha  = r - nr;
                                int prop_coluna = c - nc;

                                int p_basico = (abs_int(prop_linha) + abs_int(prop_coluna) == 1) ? 10 : 7;
                                int A = prop_linha * vento_linha + prop_coluna * vento_coluna;
                                int Pv = max_int(1, p_basico + intensidade * A);
                                S += Pv;
                            }
                        }
                    }

                    int fator_combustivel = (cobertura[idx] == 2) ? 8 : 12;
                    int I = (S * fator_combustivel * (100 - umidade[idx])) / 100;

                    if (I >= LIMIAR) {
                        proximo_estado[idx] = 2; // Virou chamas
                        proximo_tempo[idx]  = (cobertura[idx] == 2) ? 2 : 4;
                        l_cha++;
                        novas_ignicoes_no_passo++;
                    } else {
                        proximo_estado[idx] = 1; // Permanece intacta
                        proximo_tempo[idx]  = tempo_atual[idx];
                        l_int++;
                    }
                }
            } // Fim do loop de atualização (barreira implícita garante atualização das reduções)

            // 4. Troca das matrizes e atualização dos dados globais
            #pragma omp single
            {
                nao_combustiveis = l_nc;
                intactas = l_int;
                em_chamas = l_cha;
                queimadas = l_que;
                contencao = l_cnt;

                total_ignicoes += novas_ignicoes_no_passo;
                if (novas_ignicoes_no_passo > melhor_quantidade) {
                    melhor_quantidade = novas_ignicoes_no_passo;
                    melhor_passo = passo;
                }

                int *temp_p = estado_atual;   estado_atual   = proximo_estado; proximo_estado = temp_p;
                temp_p      = tempo_atual;    tempo_atual    = proximo_tempo;  proximo_tempo  = temp_p;

                existe_chama = (em_chamas > 0);
                passo++;
            } // Barreira implícita garante troca de ponteiros para a próxima iteração
        }
    } // Fim da Região Paralela

    double t_fim = omp_get_wtime();
    double tempo_execucao = t_fim - t_inicio;

    /* =========================================================================
     * FASE 9: PÓS-PROCESSAMENTO E IMPRESSÃO
     * ========================================================================= */
    unsigned long long checksum = 0;
    for (long long i = 0; i < num_celulas; i++) {
        checksum = checksum * 31ULL + (unsigned long long)estado_atual[i];
        checksum = checksum * 31ULL + (unsigned long long)tempo_atual[i];
    }

    double percentual_queimado = 0.0;
    double percentual_protegido = 0.0;

    if (combustiveis_iniciais > 0) {
        percentual_queimado  = 100.0 * (double)(queimadas + em_chamas) / (double)combustiveis_iniciais;
        percentual_protegido = 100.0 * (double)contencao / (double)combustiveis_iniciais;
    }

    printf("passos: %d\n", passo);
    printf("nao_combustiveis: %lld\n", nao_combustiveis);
    printf("intactas: %lld\n", intactas);
    printf("em_chamas: %lld\n", em_chamas);
    printf("queimadas: %lld\n", queimadas);
    printf("contencao: %lld\n", contencao);
    printf("total_ignicoes: %lld\n", total_ignicoes);
    if (melhor_quantidade > 0) {
        printf("pico_ignicoes: %d %lld\n", melhor_passo, melhor_quantidade);
    } else {
        printf("pico_ignicoes: -1 0\n");
    }
    printf("percentual_queimado: %.2f\n", percentual_queimado);
    printf("percentual_protegido: %.2f\n", percentual_protegido);
    printf("checksum: %llu\n", checksum);
    printf("tempo: %.6f\n", tempo_execucao);

    /* =========================================================================
     * FASE 10: LIMPEZA
     * ========================================================================= */
    if (focos) free(focos);
    if (zonas) free(zonas);
    free(cobertura);
    free(umidade);
    free(estado_atual);
    free(proximo_estado);
    free(tempo_atual);
    free(proximo_tempo);
    free(ativacao);

    return 0;
}
