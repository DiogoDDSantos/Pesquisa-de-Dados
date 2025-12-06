/*
 * Busca e cruzamento de dados dos microdados ENADE 2023
 * Autores: Diogo Dalbianco e Gabriela Vitoria
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define LINEBUF 16384
#define INITIAL_CAP 1024

/* Estruturas de dados */
typedef struct {
    int nu_ano;
    long co_curso;
    long co_ies;
    int co_categad;
    int co_orgacad;
    int co_grupo;
    int co_modalidade;
    long co_munic_curso;
    int co_uf_curso;
    int co_regiao_curso;
    char raw_line[1]; /* placeholder para se evitar warning ao fazer aloc dynamic se necessário */
} Arq1Record;

typedef struct {
    int nu_ano;
    long co_curso;
    double nt_ger;
    double nt_ce; /* interpretamos NT_ES pedido pelo enunciado como NT_CE (componente específico) */
} Arq3Record;

/* Vetores dinâmicos */
static Arq1Record **arq1 = NULL;
static size_t arq1_count = 0;
static size_t arq1_capacity = 0;

static Arq3Record *arq3 = NULL;
static size_t arq3_count = 0;
static size_t arq3_capacity = 0;

/* Índices (arrays de ponteiros) ordenados por chaves diferentes */
static Arq1Record **idx_by_course = NULL; /* ordenado por co_curso */
static Arq1Record **idx_by_ies = NULL;    /* ordenado por co_ies */
static Arq1Record **idx_by_uf = NULL;     /* ordenado por co_uf_curso */
static Arq1Record **idx_by_group = NULL;  /* ordenado por co_grupo */


static char *my_strdup(const char *s) {
    char *p;
    size_t n;
    if (!s) return NULL;
    n = strlen(s) + 1;
    p = (char*) malloc(n);
    if (!p) return NULL;
    memcpy(p, s, n);
    return p;
}

static void *xmalloc(size_t s) {
    void *p = malloc(s);
    if (!p) {
        fprintf(stderr, "Falha de alocação\n");
        exit(1);
    }
    return p;
}

/* Adiciona registro arq1 dinamicamente */
static void add_arq1(Arq1Record *r) {
    if (arq1_count >= arq1_capacity) {
        size_t newcap = (arq1_capacity == 0) ? INITIAL_CAP : arq1_capacity * 2;
        arq1 = (Arq1Record**) realloc(arq1, newcap * sizeof(Arq1Record*));
        if (!arq1) { fprintf(stderr, "Erro realloc arq1\n"); exit(1);}        
        arq1_capacity = newcap;
    }
    arq1[arq1_count++] = r;
}

/* Adiciona registro arq3 dinamicamente */
static void add_arq3(Arq3Record r) {
    if (arq3_count >= arq3_capacity) {
        size_t newcap = (arq3_capacity == 0) ? INITIAL_CAP : arq3_capacity * 2;
        arq3 = (Arq3Record*) realloc(arq3, newcap * sizeof(Arq3Record));
        if (!arq3) { fprintf(stderr, "Erro realloc arq3\n"); exit(1);}        
        arq3_capacity = newcap;
    }
    arq3[arq3_count++] = r;
}

static void trim(char *s) {
    char *p = s;
    char *end;
    while (*p && isspace((unsigned char)*p)) p++;
    if (p != s) memmove(s, p, strlen(p)+1);
    end = s + strlen(s) - 1;
    while (end >= s && isspace((unsigned char)*end)) { *end = '\0'; end--; }
}

/* Parse de token numerico; retorna 1 se valioso, 0 se vazio */
static int parse_int_token(const char *tok, int *out) {
    if (!tok) return 0;
    while (*tok && isspace((unsigned char)*tok)) tok++;
    if (*tok == '\0') return 0;
    *out = atoi(tok);
    return 1;
}
static int parse_long_token(const char *tok, long *out) {
    if (!tok) return 0;
    while (*tok && isspace((unsigned char)*tok)) tok++;
    if (*tok == '\0') return 0;
    *out = atol(tok);
    return 1;
}
static int parse_double_token(const char *tok, double *out) {
    if (!tok) return 0;
    while (*tok && isspace((unsigned char)*tok)) tok++;
    if (*tok == '\0') return 0;
    *out = atof(tok);
    return 1;
}

/* Detecta delimitador na linha (',' ou ';') */
static char detect_delim(const char *line) {
    const char *p = line;
    while (*p) {
        if (*p == ';') return ';';
        if (*p == ',') return ',';
        p++;
    }
    /* padrão */
    return ';';
}

/* Leitura do arq1 (microdados2023_arq1.txt)
   Colunas: NU_ANO;CO_CURSO;CO_IES;CO_CATEGAD;CO_ORGACAD;CO_GRUPO;CO_MODALIDADE;CO_MUNIC_CURSO;CO_UF_CURSO;CO_REGIAO_CURSO
*/
static void load_arq1(const char *filename) {
    FILE *f = fopen(filename, "r");
    char line[LINEBUF];
    size_t lineno = 0;
    if (!f) {
        fprintf(stderr, "Erro ao abrir %s\n", filename);
        return;
    }
    while (fgets(line, sizeof(line), f)) {
        char *tok;
        char copy[LINEBUF];
        char delim;
        const char *sep;
        lineno++;
        if (lineno == 1) {
            delim = detect_delim(line);
            continue;
        }
        strcpy(copy, line);
        delim = detect_delim(copy);
        sep = (delim==';') ? ";" : ",";
        tok = strtok(copy, sep);
        if (!tok) continue;
        {
            Arq1Record *r = (Arq1Record*) xmalloc(sizeof(Arq1Record));
            memset(r, 0, sizeof(Arq1Record));

            if (tok){
              parse_int_token(tok, &r->nu_ano);
            }  

            tok = strtok(NULL, sep);

            if (!tok){ 
              free(r); continue; 
            }

            parse_long_token(tok, &r->co_curso);
            tok = strtok(NULL, sep); if (tok) parse_long_token(tok, &r->co_ies);
            tok = strtok(NULL, sep); if (tok) parse_int_token(tok, &r->co_categad);
            tok = strtok(NULL, sep); if (tok) parse_int_token(tok, &r->co_orgacad);
            tok = strtok(NULL, sep); if (tok) parse_int_token(tok, &r->co_grupo);
            tok = strtok(NULL, sep); if (tok) parse_int_token(tok, &r->co_modalidade);
            tok = strtok(NULL, sep); if (tok) parse_long_token(tok, &r->co_munic_curso);
            tok = strtok(NULL, sep); if (tok) parse_int_token(tok, &r->co_uf_curso);
            tok = strtok(NULL, sep); if (tok) parse_int_token(tok, &r->co_regiao_curso);
            add_arq1(r);
        }
    }
    fclose(f);
    /* criar índices */
    idx_by_course = (Arq1Record**) malloc(arq1_count * sizeof(Arq1Record*));
    idx_by_ies = (Arq1Record**) malloc(arq1_count * sizeof(Arq1Record*));
    idx_by_uf = (Arq1Record**) malloc(arq1_count * sizeof(Arq1Record*));
    idx_by_group = (Arq1Record**) malloc(arq1_count * sizeof(Arq1Record*));
    {
        size_t i;
        for (i=0;i<arq1_count;i++) {
            idx_by_course[i] = arq1[i];
            idx_by_ies[i] = arq1[i];
            idx_by_uf[i] = arq1[i];
            idx_by_group[i] = arq1[i];
        }
    }
}

/* Leitura do arq3 (microdados2023_arq3.txt)
   Colunas (relevantes): NU_ANO;CO_CURSO; ... ;NT_GER; ... ;NT_CE; ...
*/
static void load_arq3(const char *filename) {
    FILE *f = fopen(filename, "r");
    char line[LINEBUF];
    size_t lineno = 0;
    if (!f) {
        fprintf(stderr, "Erro ao abrir %s\n", filename);
        return;
    }
    while (fgets(line, sizeof(line), f)) {
        char copy[LINEBUF];
        char *tok;
        char delim;
        const char *sep;
        int field = 0;
        double nt_ger = -1.0;
        double nt_ce = -1.0;
        long co_curso = -1;
        lineno++;
        if (lineno == 1) { 
          delim = detect_delim(line); 
          continue; 
        }
        strcpy(copy, line);
        delim = detect_delim(copy);
        sep = (delim==';') ? ";" : ",";
        tok = strtok(copy, sep);
        while (tok) {
            field++;
            if (field == 2) {
                parse_long_token(tok, &co_curso);
            } else {
                /* detecta números com ponto decimal - candidatos a notas */
                char *q = tok;
                while (*q && isspace((unsigned char)*q)) q++;
                if (*q) {
                    /* tenta parsear como double */
                    char *endptr;
                    double v = strtod(q, &endptr);
                    if (endptr != q) {
                        /* um número foi lido. Se estiver no intervalo 0..100, pode ser uma nota. */
                        if (v >= 0.0 && v <= 100.0) {
                            if (nt_ger < 0) nt_ger = v;
                            else if (nt_ce < 0) nt_ce = v;
                        }
                    }
                }
            }
            tok = strtok(NULL, sep);
        }
        if (co_curso > 0) {
            Arq3Record r;
            r.nu_ano = 2023;
            r.co_curso = co_curso;
            r.nt_ger = nt_ger;
            r.nt_ce = nt_ce;
            add_arq3(r);
        }
    }
    fclose(f);
}

/* Funções de comparação para qsort/bsearch */
static int cmp_course_ptr(const void *a, const void *b) {
    Arq1Record *A = *(Arq1Record**)a;
    Arq1Record *B = *(Arq1Record**)b;
    if (A->co_curso < B->co_curso) return -1;
    if (A->co_curso > B->co_curso) return 1;
    return 0;
}
static int cmp_ies_ptr(const void *a, const void *b) {
    Arq1Record *A = *(Arq1Record**)a;
    Arq1Record *B = *(Arq1Record**)b;
    if (A->co_ies < B->co_ies) return -1;
    if (A->co_ies > B->co_ies) return 1;
    return 0;
}
static int cmp_uf_ptr(const void *a, const void *b) {
    Arq1Record *A = *(Arq1Record**)a;
    Arq1Record *B = *(Arq1Record**)b;
    if (A->co_uf_curso < B->co_uf_curso) return -1;
    if (A->co_uf_curso > B->co_uf_curso) return 1;
    return 0;
}
static int cmp_group_ptr(const void *a, const void *b) {
    Arq1Record *A = *(Arq1Record**)a;
    Arq1Record *B = *(Arq1Record**)b;
    if (A->co_grupo < B->co_grupo) return -1;
    if (A->co_grupo > B->co_grupo) return 1;
    return 0;
}
static int cmp_arq3_by_ntger_desc(const void *a, const void *b) {
    const Arq3Record *A = (const Arq3Record*)a;
    const Arq3Record *B = (const Arq3Record*)b;
    if (A->nt_ger < B->nt_ger) return 1;
    if (A->nt_ger > B->nt_ger) return -1;
    return 0;
}
static int cmp_arq3_by_ntce_desc(const void *a, const void *b) {
    const Arq3Record *A = (const Arq3Record*)a;
    const Arq3Record *B = (const Arq3Record*)b;
    if (A->nt_ce < B->nt_ce) return 1;
    if (A->nt_ce > B->nt_ce) return -1;
    return 0;
}

/* busca binária por co_curso usando idx_by_course */
static Arq1Record *find_course(long co_curso) {
    Arq1Record key;
    Arq1Record *keyp = &key;
    key.co_curso = co_curso;
    Arq1Record **res = (Arq1Record**) bsearch(&keyp, idx_by_course, (size_t)arq1_count, sizeof(Arq1Record*), cmp_course_ptr);
    if (!res) return NULL;
    return *res;
}

/* encontra intervalo (primeiro..ultimo) de registros em idx_by_ies com um co_ies dado */
static void list_by_ies(long co_ies) {
    size_t i;
    /* idx_by_ies já ordenado por co_ies */
    /* encontrar primeiro índice >= co_ies */
    int found = 0;
    for (i=0;i<arq1_count;i++) {
        if (idx_by_ies[i]->co_ies == co_ies) {
            found = 1; break;
        }
    }
    if (!found) {
        printf("Nenhum curso encontrado para IES %ld\n", co_ies);
        return;
    }
    /* imprime todos iguais */
    for (; i<arq1_count && idx_by_ies[i]->co_ies == co_ies; i++) {
        Arq1Record *r = idx_by_ies[i];
        printf("Curso: %ld  IES: %ld  Grupo: %d  UF: %d\n", r->co_curso, r->co_ies, r->co_grupo, r->co_uf_curso);
    }
}

/* list by uf */
static void list_by_uf(int co_uf) {
    size_t i;
    int found = 0;
    for (i=0;i<arq1_count;i++) {
        if (idx_by_uf[i]->co_uf_curso == co_uf) { found = 1; break; }
    }
    if (!found) { printf("Nenhum curso encontrado para UF %d\n", co_uf); return; }
    for (; i<arq1_count && idx_by_uf[i]->co_uf_curso == co_uf; i++) {
        Arq1Record *r = idx_by_uf[i];
        printf("Curso: %ld  IES: %ld  Grupo: %d  UF: %d\n", r->co_curso, r->co_ies, r->co_grupo, r->co_uf_curso);
    }
}

/* list by group */
static void list_by_group(int group) {
    size_t i;
    int found = 0;
    for (i=0;i<arq1_count;i++) {
        if (idx_by_group[i]->co_grupo == group) { found = 1; break; }
    }
    if (!found) { printf("Nenhum curso encontrado para grupo %d\n", group); return; }
    for (; i<arq1_count && idx_by_group[i]->co_grupo == group; i++) {
        Arq1Record *r = idx_by_group[i];
        printf("Curso: %ld  IES: %ld  Grupo: %d  UF: %d\n", r->co_curso, r->co_ies, r->co_grupo, r->co_uf_curso);
    }
}

/* listar todas as notas de um curso (NT_GER e NT_CE) e médias */
static void show_grades_for_course(long co_curso) {
    size_t i;
    double sum_ger = 0.0;
    double sum_ce = 0.0;
    size_t cnt_ger = 0;
    size_t cnt_ce = 0;
    for (i=0;i<arq3_count;i++) {
        if (arq3[i].co_curso == co_curso) {
            if (arq3[i].nt_ger >= 0.0) { printf("NT_GER: %.2f\n", arq3[i].nt_ger); sum_ger += arq3[i].nt_ger; cnt_ger++; }
            if (arq3[i].nt_ce >= 0.0) { printf("NT_CE:  %.2f\n", arq3[i].nt_ce); sum_ce += arq3[i].nt_ce; cnt_ce++; }
        }
    }
    if (cnt_ger > 0) printf("Media NT_GER: %.4f (n=%lu)\n", sum_ger / (double)cnt_ger, (unsigned long)cnt_ger);
    else printf("Sem registros NT_GER para o curso %ld\n", co_curso);
    if (cnt_ce > 0) printf("Media NT_CE:  %.4f (n=%lu)\n", sum_ce / (double)cnt_ce, (unsigned long)cnt_ce);
    else printf("Sem registros NT_CE para o curso %ld\n", co_curso);
}

/* mostra as n maiores notas (geral ou especifica) cruzando com arq1 para mostrar IES */
static void show_top_n(int n, int tipo) {
    /* tipo: 0 = NT_GER, 1 = NT_CE */
    if (n <= 0) { 
      printf("n deve ser > 0\n"); return; 
    }
    if (arq3_count == 0) { 
      printf("Nenhum dado em arq3\n"); 
      return; 
    }
    /* Criar cópia para ordenar */
    Arq3Record *copy = (Arq3Record*) malloc(arq3_count * sizeof(Arq3Record));
    if (!copy) { 
      fprintf(stderr, "Erro malloc copy\n"); 
      return; 
    }
    memcpy(copy, arq3, arq3_count * sizeof(Arq3Record));
    if (tipo == 0){
      qsort(copy, arq3_count, sizeof(Arq3Record), cmp_arq3_by_ntger_desc);
    } else {
      qsort(copy, arq3_count, sizeof(Arq3Record), cmp_arq3_by_ntce_desc);
    }

    int printed = 0;
    int i;
    for (i=0; i<(int)arq3_count && printed < n; i++) {
      double val = (tipo==0)? copy[i].nt_ger : copy[i].nt_ce;
      if (val < 0.0){
          continue;
      }
      Arq1Record *r = find_course(copy[i].co_curso);
      if (r) {
        printf("%d) Curso: %ld  IES: %ld  Nota: %.2f\n", printed+1, copy[i].co_curso, r->co_ies, val);
      } else {
        printf("%d) Curso: %ld  IES: (desconhecido)  Nota: %.2f\n", printed+1, copy[i].co_curso, val);
      }
      printed++;
        }
    
    free(copy);
}

/* Menu interativo */
static void print_menu() {
    printf("\n--- Menu ENADE 2023 ---\n");
    printf("1 - Procurar curso por codigo (arq1)\n");
    printf("2 - Listar cursos por IES (arq1)\n");
    printf("3 - Listar cursos por UF (arq1)\n");
    printf("4 - Listar cursos por grupo (arq1)\n");
    printf("5 - Mostrar notas e medias de um curso (arq3)\n");
    printf("6 - Mostrar as n maiores notas (arq3)\n");
    printf("7 - Sair\n");
    printf("Escolha: ");
}

int main(int argc, char **argv) {
    char arq1file[512] = "microdados2023_arq1.txt";
    char arq3file[512] = "microdados2023_arq3.txt";
    char line[256];
    int choice;
    /* mensagem inicial */
    printf("Programa de buscas ENADE 2023 (ANSI C)\n");
    if (argc >= 3) {
        strncpy(arq1file, argv[1], sizeof(arq1file)-1); arq1file[sizeof(arq1file)-1] = '\0';
        strncpy(arq3file, argv[2], sizeof(arq3file)-1); arq3file[sizeof(arq3file)-1] = '\0';
    } else {
        printf("Usando arquivos padrao: %s e %s\n", arq1file, arq3file);
        printf("Se desejar usar outros arquivos, execute: %s arq1.txt arq3.txt\n", argv[0]);
    }
    load_arq1(arq1file);
    load_arq3(arq3file);
    if (arq1_count == 0) { printf("Aviso: nenhum registro em arq1. Verifique arquivo.\n"); }
    if (arq3_count == 0) { printf("Aviso: nenhum registro em arq3. Verifique arquivo.\n"); }
    /* ordenar índices */
    if (arq1_count > 0) {
        qsort(idx_by_course, arq1_count, sizeof(Arq1Record*), cmp_course_ptr);
        qsort(idx_by_ies, arq1_count, sizeof(Arq1Record*), cmp_ies_ptr);
        qsort(idx_by_uf, arq1_count, sizeof(Arq1Record*), cmp_uf_ptr);
        qsort(idx_by_group, arq1_count, sizeof(Arq1Record*), cmp_group_ptr);
    }
    while (1) {
        print_menu();
        if (!fgets(line, sizeof(line), stdin)) break;
        choice = atoi(line);
        if (choice == 1) {
            long co;
            printf("Digite o codigo do curso (CO_CURSO): ");
            if (!fgets(line, sizeof(line), stdin)) break;
            co = atol(line);
            Arq1Record *r = find_course(co);
            if (!r) printf("Curso %ld nao encontrado em arq1.\n", co);
            else {
                printf("Curso: %ld\nIES: %ld\nGrupo: %d\nUF: %d\nAno: %d\n", r->co_curso, r->co_ies, r->co_grupo, r->co_uf_curso, r->nu_ano);
            }
        } else if (choice == 2) {
            long ies;
            printf("Digite CO_IES: ");
            if (!fgets(line, sizeof(line), stdin)) break;
            ies = atol(line);
            list_by_ies(ies);
        } else if (choice == 3) {
            int uf;
            printf("Digite CO_UF_CURSO (codigo numerico da UF): ");
            if (!fgets(line, sizeof(line), stdin)) break;
            uf = atoi(line);
            list_by_uf(uf);
        } else if (choice == 4) {
            int gp;
            printf("Digite CO_GRUPO (codigo do grupo): ");
            if (!fgets(line, sizeof(line), stdin)) break;
            gp = atoi(line);
            list_by_group(gp);
        } else if (choice == 5) {
            long co;
            printf("Digite CO_CURSO para listar notas: ");
            if (!fgets(line, sizeof(line), stdin)) break;
            co = atol(line);
            show_grades_for_course(co);
        } else if (choice == 6) {
            int tipo, n;
            printf("Tipo de nota: 0 - NT_GER, 1 - NT_CE : ");
            if (!fgets(line, sizeof(line), stdin)) break;
            tipo = atoi(line);
            printf("Quantas maiores (n): ");
            if (!fgets(line, sizeof(line), stdin)) break;
            n = atoi(line);
            show_top_n(n, tipo);
        } else if (choice == 7) {
            break;
        } else {
            printf("Opcao invalida. Tente novamente.\n");
        }
    }
    printf("Saindo.\n");
    return 0;
}