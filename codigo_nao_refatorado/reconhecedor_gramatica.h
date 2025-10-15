#ifndef reconhecedor_gramatica_h
#define reconhecedor_gramatica_h

#include <stdio.h>

#define EPSILON_INPUT_CHAR '!'

typedef struct {
    char *lhs;
    char *rhs;
    int is_epsilon_production;
} ProductionRule;

typedef struct {
    ProductionRule *rules;
    int num_rules;
    int max_rules;

    char *non_terminals;
    int nt_count;
    int max_nt_size;

    char *terminals;
    int t_count;
    int max_t_size;

    char start_symbol;
} Grammar;


char* ler_conteudo_arquivo(const char* filename);

int analisar_gramatica(const char* input_str, Grammar *g);

void imprimir_gramatica(const Grammar *g);

void liberar_gramatica(Grammar *g);

#endif
