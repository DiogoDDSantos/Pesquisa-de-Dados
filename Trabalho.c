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

static Arq1Record **arq1 = NULL;
static size_t arq1_count = 0;
static size_t arq1_capacity = 0;


static Arq3Record *arq3 = NULL;
static size_t arq3_count = 0;
static size_t arq3_capacity = 0;

static Arq1Record **idx_by_course = NULL; 
static Arq1Record **idx_by_ies = NULL; 
static Arq1Record **idx_by_uf = NULL; 
static Arq1Record **idx_by_group = NULL; 


static char detect_delim(const char *line) {
  const char *p = line;
  while (*p) {
    if (*p == ';') return ';';
    if (*p == ',') return ',';
    p++;
}
  return ';';
}

static void load_arq1(const char *filename) {
  char delim;
  lineno++;
  if (lineno == 1) {
    delim = detect_delim(line);
    continue;
  }
  strcpy(copy, line);
  delim = detect_delim(copy);
  p = copy;
  tok = strtok_r(p, (delim==';')?";":"," , &saveptr);
  if (!tok) continue;
  Arq1Record *r = (Arq1Record*) xmalloc(sizeof(Arq1Record));
  memset(r, 0, sizeof(Arq1Record));

  if (tok) parse_int_token(tok, &r->nu_ano);
  tok = strtok_r(NULL, (delim==';')?";":",", &saveptr);
  if (!tok) { free(r); continue; }
  parse_long_token(tok, &r->co_curso);
  tok = strtok_r(NULL, (delim==';')?";":",", &saveptr); if (tok) parse_long_token(tok, &r->co_ies);
  tok = strtok_r(NULL, (delim==';')?";":",", &saveptr); if (tok) parse_int_token(tok, &r->co_categad);
  tok = strtok_r(NULL, (delim==';')?";":",", &saveptr); if (tok) parse_int_token(tok, &r->co_orgacad);
  tok = strtok_r(NULL, (delim==';')?";":",", &saveptr); if (tok) parse_int_token(tok, &r->co_grupo);
  tok = strtok_r(NULL, (delim==';')?";":",", &saveptr); if (tok) parse_int_token(tok, &r->co_modalidade);
  tok = strtok_r(NULL, (delim==';')?";":",", &saveptr); if (tok) parse_long_token(tok, &r->co_munic_curso);
  tok = strtok_r(NULL, (delim==';')?";":",", &saveptr); if (tok) parse_int_token(tok, &r->co_uf_curso);
  tok = strtok_r(NULL, (delim==';')?";":",", &saveptr); if (tok) parse_int_token(tok, &r->co_regiao_curso);
  add_arq1(r);
}
  fclose(f);
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
