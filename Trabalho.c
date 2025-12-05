#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int ano;
    int cod_curso;
    int cod_ies;
    int cod_categoria;
    int cod_org_acad;
    int cod_grupo;
    int cod_modalidade;
    int cod_municipio;
    int cod_uf;
    int cod_regiao;
} DadosCurso;

typedef struct {
    int ano;
    int cod_curso;
    float nota_geral;
    float nota_especifica;
} DadosNotas;

