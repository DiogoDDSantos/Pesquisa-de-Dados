#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define LINEBUF 16384
#define INITIAL_CAP 1024

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
char raw_line[1]; 
} Arq1Record;


typedef struct {
int nu_ano;
long co_curso;
double nt_ger;
double nt_ce; 
} Arq3Record;
