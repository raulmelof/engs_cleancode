#ifndef reconhecedor_gramatica_h
#define reconhecedor_gramatica_h

#include <stdio.h>

#define EPSILON_INPUT_CHAR '!'

typedef enum {
    SUCESSO_ANALISE,
    ERRO_ALOCACAO_MEMORIA,
    ERRO_FALTA_CIFRAO_FINAL,
    ERRO_GRAMATICA_VAZIA,
    ERRO_FALTA_SIMBOLO_INICIAL,
    ERRO_REGRA_MAL_FORMADA,
    ERRO_REGRA_VAZIA_INICIAL,
    ERRO_REGRA_VAZIA_INTERMEDIARIA,
    ERRO_FALTA_SEPARADOR_REGRA,
    ERRO_LHS_VAZIO,
    ERRO_LHS_SEM_NAO_TERMINAL,
    ERRO_CARACTERE_INVALIDO_LHS,
    ERRO_CARACTERE_INVALIDO_RHS,
    ERRO_EPSILON_NAO_SOZINHO,
    ERRO_HIFEN_FINAL
} CodigoErro;

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

void inicializar_gramatica(Grammar *gramatica);

char* ler_conteudo_arquivo(const char* nome_arquivo);

CodigoErro analisar_gramatica(const char* definicao_gramatica, Grammar *gramatica);

void imprimir_gramatica(const Grammar *gramatica);

void liberar_gramatica(Grammar *gramatica);

#endif
