#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "reconhecedor_gramatica.h"

static void adicionar_char_unico(char **str, int *current_size, int *capacity, char c) {
    if (*str == NULL || strchr(*str, c) == NULL) {
        if (*current_size + 1 >= *capacity) {
            *capacity = (*capacity == 0) ? 16 : *capacity * 2;
            char *new_str = (char*)realloc(*str, *capacity);
            if (!new_str) {
                fprintf(stderr, "Erro: Falha ao realocar memória para string de símbolos únicos.\n");
                if (*str) free(*str);
                *str = NULL;
                return;
            }
            *str = new_str;
        }
        (*str)[(*current_size)++] = c;
        (*str)[*current_size] = '\0';
    }
}

static void processar_simbolo(char c, Grammar *g, int is_lhs_char, int *lhs_has_nt) {
    if (isupper(c)) {
        adicionar_char_unico(&(g->non_terminals), &(g->nt_count), &(g->max_nt_size), c);
        if (is_lhs_char) {
            *lhs_has_nt = 1;
        }
    } else if (islower(c)) {
        adicionar_char_unico(&(g->terminals), &(g->t_count), &(g->max_t_size), c);
    }
}


char* ler_conteudo_arquivo(const char* filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Erro ao abrir arquivo");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char*)malloc(length + 1);
    if (!buffer) {
        fprintf(stderr, "Erro de alocação de memória\n");
        fclose(file);
        return NULL;
    }

    if (fread(buffer, 1, length, file) != (size_t)length) {
        fprintf(stderr, "Erro ao ler arquivo\n");
        fclose(file);
        free(buffer);
        return NULL;
    }
    buffer[length] = '\0';
    fclose(file);

    long actual_len = strlen(buffer);
    while (actual_len > 0 && (buffer[actual_len - 1] == '\n' || buffer[actual_len - 1] == '\r')) {
        buffer[actual_len - 1] = '\0';
        actual_len--;
    }
    return buffer;
}

int analisar_gramatica(const char* input_str, Grammar *g) {
    g->rules = NULL;
    g->num_rules = 0;
    g->max_rules = 0;
    g->non_terminals = (char*)malloc(1); g->non_terminals[0] = '\0';
    g->nt_count = 0; g->max_nt_size = 1;
    g->terminals = (char*)malloc(1); g->terminals[0] = '\0';
    g->t_count = 0; g->max_t_size = 1;
    g->start_symbol = '\0';

    if (!g->non_terminals || !g->terminals) {
        fprintf(stderr, "Erro: Falha na alocação inicial para terminais/não terminais.\n");
        free(g->non_terminals); free(g->terminals);
        return 0;
    }

    size_t len_input = strlen(input_str);
    if (len_input == 0 || input_str[len_input - 1] != '$') {
        fprintf(stderr, "Erro de Validação: A entrada deve terminar com '$'.\n");
        return 0;
    }
    if (len_input == 1 && input_str[0] == '$') {
        fprintf(stderr, "Erro de Validação: Entrada contém apenas '$'. Pelo menos uma regra é necessária.\n");
        return 0;
    }

    for (size_t i = 0; i < len_input && input_str[i] != '$'; ++i) {
        if (isupper(input_str[i])) {
            g->start_symbol = input_str[i];
            break;
        }
    }
    int tem_conteudo_util = 0;
    for(size_t i=0; i < len_input -1; ++i) {
        if(input_str[i] != '-' && input_str[i] != '>') {
            tem_conteudo_util = 1;
            break;
        }
    }
    if (g->start_symbol == '\0' && tem_conteudo_util) {
        fprintf(stderr, "Erro de Validação: Nenhum símbolo não terminal (A-Z) encontrado para ser o símbolo inicial.\n");
        return 0;
    }


    const char *pos_atual = input_str;
    const char *fim_regra_ptr;

    while (*pos_atual != '$' && *pos_atual != '\0') {
        fim_regra_ptr = strchr(pos_atual, '-');
        const char *fim_gramatica_ptr = strchr(pos_atual, '$');

        if (fim_regra_ptr == NULL || (fim_gramatica_ptr != NULL && fim_regra_ptr > fim_gramatica_ptr)) {
            fim_regra_ptr = fim_gramatica_ptr;
        }

        if (fim_regra_ptr == NULL) {
            fprintf(stderr, "Erro de Validação: Estrutura de regra malformada ou faltando '$'.\n");
            return 0;
        }
        if (pos_atual == fim_regra_ptr && *fim_regra_ptr == '$' && g->num_rules == 0) {
             fprintf(stderr, "Erro de Validação: Regra vazia antes de '$' e nenhuma regra anterior.\n");
             return 0;
        }
        if (pos_atual == fim_regra_ptr && *fim_regra_ptr == '-') {
            fprintf(stderr, "Erro de Validação: Regra vazia entre '-' (ex: rule1--rule2).\n");
            return 0;
        }


        const char *lhs_rhs_sep = strchr(pos_atual, '>');
        if (lhs_rhs_sep == NULL || lhs_rhs_sep >= fim_regra_ptr) {
            fprintf(stderr, "Erro de Validação: Regra '%.*s' não contém '>' ou está malformada.\n", (int)(fim_regra_ptr - pos_atual), pos_atual);
            return 0;
        }

        if (g->num_rules >= g->max_rules) {
            g->max_rules = (g->max_rules == 0) ? 4 : g->max_rules * 2;
            ProductionRule *novas_regras = (ProductionRule*)realloc(g->rules, g->max_rules * sizeof(ProductionRule));
            if (!novas_regras) {
                fprintf(stderr, "Erro: Falha ao realocar memória para regras.\n");
                return 0;
            }
            g->rules = novas_regras;
        }
        ProductionRule *regra_atual = &g->rules[g->num_rules];
        regra_atual->is_epsilon_production = 0;
        regra_atual->lhs = NULL;
        regra_atual->rhs = NULL;


        size_t lhs_len = lhs_rhs_sep - pos_atual;
        if (lhs_len == 0) {
            fprintf(stderr, "Erro de Validação: Lado esquerdo (LHS) não pode ser vazio na regra '%.*s'.\n", (int)(fim_regra_ptr - pos_atual), pos_atual);
            return 0;
        }
        regra_atual->lhs = (char*)malloc(lhs_len + 1);
        if (!regra_atual->lhs) { fprintf(stderr, "Malloc falhou para LHS\n"); return 0; }
        strncpy(regra_atual->lhs, pos_atual, lhs_len);
        regra_atual->lhs[lhs_len] = '\0';

        int lhs_tem_nao_terminal = 0;
        for (size_t i = 0; i < lhs_len; ++i) {
            char c = regra_atual->lhs[i];
            if (!isupper(c) && !islower(c)) {
                fprintf(stderr, "Erro de Validação: Caractere inválido '%c' no LHS '%s'.\n", c, regra_atual->lhs);
                return 0;
            }
            processar_simbolo(c, g, 1, &lhs_tem_nao_terminal);
        }
        if (!lhs_tem_nao_terminal) {
            fprintf(stderr, "Erro de Validação: LHS '%s' deve conter pelo menos um não terminal (A-Z).\n", regra_atual->lhs);
            return 0;
        }

        const char* rhs_inicio = lhs_rhs_sep + 1;
        size_t rhs_len = fim_regra_ptr - rhs_inicio;
        regra_atual->rhs = (char*)malloc(rhs_len + 1);
        if(!regra_atual->rhs) {
            fprintf(stderr, "Malloc falhou para RHS\n");
            return 0;
        }
        strncpy(regra_atual->rhs, rhs_inicio, rhs_len);
        regra_atual->rhs[rhs_len] = '\0';

        if (rhs_len == 1 && regra_atual->rhs[0] == EPSILON_INPUT_CHAR) {
            regra_atual->is_epsilon_production = 1;
        } else {
            for (size_t i = 0; i < rhs_len; ++i) {
                char c = regra_atual->rhs[i];
                if (!isupper(c) && !islower(c) && c != EPSILON_INPUT_CHAR) {
                    fprintf(stderr, "Erro de Validação: Caractere inválido '%c' no RHS '%s'.\n", c, regra_atual->rhs);
                    return 0;
                }
                 if (c == EPSILON_INPUT_CHAR && rhs_len > 1) {
                    fprintf(stderr, "Erro de Validação: Símbolo Epsilon ('%c') em RHS '%s' deve aparecer sozinho.\n", EPSILON_INPUT_CHAR, regra_atual->rhs);
                    return 0;
                 }
                int dummy_lhs_tem_nt = 0;
                processar_simbolo(c, g, 0, &dummy_lhs_tem_nt);
            }
        }
        g->num_rules++;

        if (*fim_regra_ptr == '$') {
            pos_atual = fim_regra_ptr;
        } else {
            pos_atual = fim_regra_ptr + 1;
            if (*pos_atual == '$' || *pos_atual == '\0') {
                 fprintf(stderr, "Erro de Validação: '-' final antes de '$' ou fim da string.\n");
                 return 0;
            }
        }
    }
    if (g->num_rules == 0 && tem_conteudo_util) {
        fprintf(stderr, "Erro de Validação: Nenhuma regra foi analisada, mas havia conteúdo.\n");
        return 0;
    }

    return 1;
}

void imprimir_gramatica(const Grammar *g) {
    if (!g) return;

    printf("Gramática Analisada com Sucesso:\n");

    printf("  Não Terminais (N): { ");
    for (int i = 0; i < g->nt_count; ++i) {
        printf("%c%s", g->non_terminals[i], (i == g->nt_count - 1) ? "" : ", ");
    }
    printf(" }\n");

    printf("  Terminais (T):     { ");
    if (g->t_count == 0) {
        printf(" ");
    } else {
        for (int i = 0; i < g->t_count; ++i) {
            printf("%c%s", g->terminals[i], (i == g->t_count - 1) ? "" : ", ");
        }
    }
    printf(" }\n");

    printf("  Símbolo Inicial (S): %c\n", g->start_symbol ? g->start_symbol : ' ');

    printf("  Regras de Produção (P):\n");
    for (int i = 0; i < g->num_rules; ++i) {
        printf("    %s -> ", g->rules[i].lhs);
        if (g->rules[i].is_epsilon_production) {
            printf("ε\n", EPSILON_INPUT_CHAR);
        } else if (strlen(g->rules[i].rhs) == 0 && !g->rules[i].is_epsilon_production) {
            printf("ε (RHS vazio na entrada)\n");
        } else {
            printf("%s\n", g->rules[i].rhs);
        }
    }
}

void liberar_gramatica(Grammar *g) {
    if (!g) return;
    if (g->rules) {
        for (int i = 0; i < g->num_rules; ++i) {
            free(g->rules[i].lhs);
            free(g->rules[i].rhs);
        }
        free(g->rules);
    }
    free(g->non_terminals);
    free(g->terminals);

    g->rules = NULL; g->num_rules = 0; g->max_rules = 0;
    g->non_terminals = NULL; g->nt_count = 0; g->max_nt_size = 0;
    g->terminals = NULL; g->t_count = 0; g->max_t_size = 0;
    g->start_symbol = '\0';
}
