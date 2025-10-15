#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "reconhecedor_gramatica.h"

static int adicionar_char_unico(char **str, int *current_size, int *capacity, char c) {
    if (*str == NULL || strchr(*str, c) == NULL) {
        if (*current_size + 1 >= *capacity) {
            *capacity = (*capacity == 0) ? 16 : *capacity * 2;
            char *new_str = (char*)realloc(*str, *capacity);
            if (!new_str) {
                if (*str) free(*str);
                *str = NULL;
                return 0;
            }
            *str = new_str;
        }
        (*str)[(*current_size)++] = c;
        (*str)[*current_size] = '\0';
    }
    return 1;
}

char* ler_conteudo_arquivo(const char* nome_arquivo) {
    FILE *file = fopen(nome_arquivo, "r");
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

void inicializar_gramatica(Grammar *gramatica) {
    gramatica->rules = NULL;
    gramatica->num_rules = 0;
    gramatica->max_rules = 0;

    gramatica->non_terminals = (char*)malloc(1);
    if (gramatica->non_terminals) gramatica->non_terminals[0] = '\0';
    gramatica->nt_count = 0;
    gramatica->max_nt_size = gramatica->non_terminals ? 1 : 0;

    gramatica->terminals = (char*)malloc(1);
    if (gramatica->terminals) gramatica->terminals[0] = '\0';
    gramatica->t_count = 0;
    gramatica->max_t_size = gramatica->terminals ? 1 : 0;

    gramatica->start_symbol = '\0';
}

static CodigoErro validar_entrada_e_definir_simbolo_inicial(const char* definicao_gramatica, Grammar *gramatica) {
    size_t len_input = strlen(definicao_gramatica);
    if (len_input == 0 || definicao_gramatica[len_input - 1] != '$') {
        return ERRO_FALTA_CIFRAO_FINAL;
    }
    if (len_input == 1 && definicao_gramatica[0] == '$') {
        return ERRO_GRAMATICA_VAZIA;
    }

    int tem_conteudo_util = 0;
    for (size_t i = 0; i < len_input - 1; ++i) {
        if (!isspace(definicao_gramatica[i])) { // Melhoria: ignorar espaços em branco
            tem_conteudo_util = 1;
        }
        if (gramatica->start_symbol == '\0' && isupper(definicao_gramatica[i])) {
            gramatica->start_symbol = definicao_gramatica[i];
        }
    }

    if (gramatica->start_symbol == '\0' && tem_conteudo_util) {
        return ERRO_FALTA_SIMBOLO_INICIAL;
    }

    return SUCESSO_ANALISE;
}

static CodigoErro garantir_espaco_para_regra(Grammar *gramatica) {
    if (gramatica->num_rules >= gramatica->max_rules) {
        gramatica->max_rules = (gramatica->max_rules == 0) ? 4 : gramatica->max_rules * 2;
        ProductionRule *novas_regras = (ProductionRule*)realloc(gramatica->rules, gramatica->max_rules * sizeof(ProductionRule));
        if (!novas_regras) {
            return ERRO_ALOCACAO_MEMORIA;
        }
        gramatica->rules = novas_regras;
    }
    return SUCESSO_ANALISE;
}

static CodigoErro adicionar_simbolo_a_gramatica(char c, Grammar *gramatica) {
    int sucesso = 1;
    if (isupper(c)) {
        // A função 'adicionar_char_unico' faz a verificação de unicidade e o realloc
        sucesso = adicionar_char_unico(&(gramatica->non_terminals), &(gramatica->nt_count), &(gramatica->max_nt_size), c);
    } else if (islower(c)) {
        sucesso = adicionar_char_unico(&(gramatica->terminals), &(gramatica->t_count), &(gramatica->max_t_size), c);
    }
    // Ignora outros caracteres (como Epsilon)
    return sucesso ? SUCESSO_ANALISE : ERRO_ALOCACAO_MEMORIA;
}

// Processa e valida os símbolos do Lado Esquerdo (LHS)
static CodigoErro processar_lhs(const char* lhs_str, size_t lhs_len, Grammar* gramatica) {
    int lhs_tem_nao_terminal = 0;
    for (size_t i = 0; i < lhs_len; ++i) {
        char c = lhs_str[i];
        if (!isupper(c) && !islower(c)) {
            return ERRO_CARACTERE_INVALIDO_LHS;
        }
        if (isupper(c)) {
            lhs_tem_nao_terminal = 1;
        }
        if (adicionar_simbolo_a_gramatica(c, gramatica) != SUCESSO_ANALISE) {
            return ERRO_ALOCACAO_MEMORIA;
        }
    }
    if (!lhs_tem_nao_terminal) {
        return ERRO_LHS_SEM_NAO_TERMINAL;
    }
    return SUCESSO_ANALISE;
}

// Processa e valida os símbolos do Lado Direito (RHS)
static CodigoErro processar_rhs(const char* rhs_str, size_t rhs_len, Grammar* gramatica) {
    if (rhs_len == 1 && rhs_str[0] == EPSILON_INPUT_CHAR) {
        return SUCESSO_ANALISE;
    }

    for (size_t i = 0; i < rhs_len; ++i) {
        char c = rhs_str[i];
        if (!isupper(c) && !islower(c) && c != EPSILON_INPUT_CHAR) {
            return ERRO_CARACTERE_INVALIDO_RHS;
        }
        if (c == EPSILON_INPUT_CHAR && rhs_len > 1) {
            return ERRO_EPSILON_NAO_SOZINHO;
        }
        if (adicionar_simbolo_a_gramatica(c, gramatica) != SUCESSO_ANALISE) {
            return ERRO_ALOCACAO_MEMORIA;
        }
    }
    return SUCESSO_ANALISE;
}

static CodigoErro analisar_regra_unica(const char** cursor_ptr, Grammar* gramatica) {
    const char* cursor = *cursor_ptr;

    // 1. Encontrar delimitadores da regra
    const char *fim_regra_ptr = strchr(cursor, '-');
    const char *fim_gramatica_ptr = strchr(cursor, '$');

    if (fim_regra_ptr == NULL || (fim_gramatica_ptr != NULL && fim_regra_ptr > fim_gramatica_ptr)) {
        fim_regra_ptr = fim_gramatica_ptr;
    }

    if (fim_regra_ptr == NULL) {
        return ERRO_REGRA_MAL_FORMADA;
    }

    // 2. Validar regra vazia
    if (cursor == fim_regra_ptr) {
        if (*fim_regra_ptr == '$' && gramatica->num_rules == 0) return ERRO_REGRA_VAZIA_INICIAL;
        if (*fim_regra_ptr == '-') return ERRO_REGRA_VAZIA_INTERMEDIARIA;
        if (*fim_regra_ptr == '$') {
            *cursor_ptr = fim_regra_ptr;
            return SUCESSO_ANALISE;
        }
    }

    // 3. Encontrar separador LHS/RHS
    const char *lhs_rhs_sep = strchr(cursor, '>');
    if (lhs_rhs_sep == NULL || lhs_rhs_sep >= fim_regra_ptr) {
        return ERRO_FALTA_SEPARADOR_REGRA;
    }

    // 4. Garantir espaço para a nova regra
    CodigoErro status = garantir_espaco_para_regra(gramatica);
    if (status != SUCESSO_ANALISE) return status;

    ProductionRule *regra_atual = &gramatica->rules[gramatica->num_rules];
    regra_atual->lhs = NULL;
    regra_atual->rhs = NULL;
    regra_atual->is_epsilon_production = 0;

    // 5. Processar LHS
    size_t lhs_len = lhs_rhs_sep - cursor;
    if (lhs_len == 0) return ERRO_LHS_VAZIO;

    status = processar_lhs(cursor, lhs_len, gramatica);
    if (status != SUCESSO_ANALISE) return status;

    regra_atual->lhs = (char*)malloc(lhs_len + 1);
    if (!regra_atual->lhs) return ERRO_ALOCACAO_MEMORIA;
    strncpy(regra_atual->lhs, cursor, lhs_len);
    regra_atual->lhs[lhs_len] = '\0';

    // 6. Processar RHS
    const char* rhs_inicio = lhs_rhs_sep + 1;
    size_t rhs_len = fim_regra_ptr - rhs_inicio;

    status = processar_rhs(rhs_inicio, rhs_len, gramatica);
    if (status != SUCESSO_ANALISE) {
        free(regra_atual->lhs);
        regra_atual->lhs = NULL;
        return status;
    }

    regra_atual->rhs = (char*)malloc(rhs_len + 1);
    if (!regra_atual->rhs) {
        free(regra_atual->lhs);
        regra_atual->lhs = NULL;
        return ERRO_ALOCACAO_MEMORIA;
    }
    strncpy(regra_atual->rhs, rhs_inicio, rhs_len);
    regra_atual->rhs[rhs_len] = '\0';

    if (rhs_len == 1 && regra_atual->rhs[0] == EPSILON_INPUT_CHAR) {
        regra_atual->is_epsilon_production = 1;
    }

    gramatica->num_rules++;

    if (*fim_regra_ptr == '$') {
        *cursor_ptr = fim_regra_ptr;
    } else {
        *cursor_ptr = fim_regra_ptr + 1;
        if (**cursor_ptr == '$' || **cursor_ptr == '\0') {
            return ERRO_HIFEN_FINAL;
        }
    }

    return SUCESSO_ANALISE;
}

CodigoErro analisar_gramatica(const char* definicao_gramatica, Grammar *gramatica) {
    CodigoErro status = validar_entrada_e_definir_simbolo_inicial(definicao_gramatica, gramatica);
    if (status != SUCESSO_ANALISE) {
        return status;
    }
    const char *cursor = definicao_gramatica;
    while (*cursor != '$' && *cursor != '\0') {
        status = analisar_regra_unica(&cursor, gramatica);
        if (status != SUCESSO_ANALISE) {
            return status;
        }
    }
    return SUCESSO_ANALISE;
}

void imprimir_gramatica(const Grammar *gramatica) {
    if (!gramatica) return;

    printf("Gramática Analisada com Sucesso:\n");

    printf("  Não Terminais (N): { ");
    for (int i = 0; i < gramatica->nt_count; ++i) {
        printf("%c%s", gramatica->non_terminals[i], (i == gramatica->nt_count - 1) ? "" : ", ");
    }
    printf(" }\n");

    printf("  Terminais (T):     { ");
    if (gramatica->t_count == 0) {
        printf(" ");
    } else {
        for (int i = 0; i < gramatica->t_count; ++i) {
            printf("%c%s", gramatica->terminals[i], (i == gramatica->t_count - 1) ? "" : ", ");
        }
    }
    printf(" }\n");

    printf("  Símbolo Inicial (S): %c\n", gramatica->start_symbol ? gramatica->start_symbol : ' ');

    printf("  Regras de Produção (P):\n");
    for (int i = 0; i < gramatica->num_rules; ++i) {
        printf("    %s -> ", gramatica->rules[i].lhs);
        if (gramatica->rules[i].is_epsilon_production) {
            printf("ε\n", EPSILON_INPUT_CHAR);
        } else if (strlen(gramatica->rules[i].rhs) == 0 && !gramatica->rules[i].is_epsilon_production) {
            printf("ε (RHS vazio na entrada)\n");
        } else {
            printf("%s\n", gramatica->rules[i].rhs);
        }
    }
}

void liberar_gramatica(Grammar *gramatica) {
    if (!gramatica) return;
    if (gramatica->rules) {
        for (int i = 0; i < gramatica->num_rules; ++i) {
            free(gramatica->rules[i].lhs);
            free(gramatica->rules[i].rhs);
        }
        free(gramatica->rules);
    }
    free(gramatica->non_terminals);
    free(gramatica->terminals);

    gramatica->rules = NULL; gramatica->num_rules = 0; gramatica->max_rules = 0;
    gramatica->non_terminals = NULL; gramatica->nt_count = 0; gramatica->max_nt_size = 0;
    gramatica->terminals = NULL; gramatica->t_count = 0; gramatica->max_t_size = 0;
    gramatica->start_symbol = '\0';
}
