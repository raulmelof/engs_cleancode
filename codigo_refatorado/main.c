#include <stdio.h>
#include <stdlib.h>
#include "reconhecedor_gramatica.h"

#ifdef _WIN32
#include <windows.h>
#endif //correção de caracteres especiais como ã e ε

static void imprimir_mensagem_erro(CodigoErro codigo) {
    fprintf(stderr, "Falha ao analisar a gramática: ");
    switch (codigo) {
        case ERRO_ALOCACAO_MEMORIA:
            fprintf(stderr, "Não foi possível alocar memória.\n");
            break;
        case ERRO_FALTA_CIFRAO_FINAL:
            fprintf(stderr, "A definição da gramática deve terminar com o caractere '$'.\n");
            break;
        case ERRO_GRAMATICA_VAZIA:
            fprintf(stderr, "A entrada não pode conter apenas o caractere '$'.\n");
            break;
        case ERRO_FALTA_SIMBOLO_INICIAL:
            fprintf(stderr, "Nenhum símbolo não-terminal (A-Z) encontrado para ser o símbolo inicial.\n");
            break;
        case ERRO_REGRA_MAL_FORMADA:
            fprintf(stderr, "Estrutura de regra malformada ou faltando '$' no final.\n");
            break;
        case ERRO_REGRA_VAZIA_INICIAL:
            fprintf(stderr, "A gramática não pode começar com uma regra vazia antes de '$'.\n");
            break;
        case ERRO_REGRA_VAZIA_INTERMEDIARIA:
            fprintf(stderr, "Regra vazia encontrada entre hífens ('--').\n");
            break;
        case ERRO_FALTA_SEPARADOR_REGRA:
            fprintf(stderr, "Uma regra não contém o separador '>' ou está malformada.\n");
            break;
        case ERRO_LHS_VAZIO:
            fprintf(stderr, "O lado esquerdo (LHS) de uma regra não pode ser vazio.\n");
            break;
        case ERRO_LHS_SEM_NAO_TERMINAL:
            fprintf(stderr, "O lado esquerdo (LHS) de uma regra deve conter pelo menos um não-terminal (A-Z).\n");
            break;
        case ERRO_CARACTERE_INVALIDO_LHS:
            fprintf(stderr, "Caractere inválido encontrado no lado esquerdo (LHS) de uma regra.\n");
            break;
        case ERRO_CARACTERE_INVALIDO_RHS:
            fprintf(stderr, "Caractere inválido encontrado no lado direito (RHS) de uma regra.\n");
            break;
        case ERRO_EPSILON_NAO_SOZINHO:
            fprintf(stderr, "O símbolo de épsilon ('%c') só pode aparecer sozinho no lado direito (RHS).\n", EPSILON_INPUT_CHAR);
            break;
        case ERRO_HIFEN_FINAL:
            fprintf(stderr, "A definição da gramática não pode terminar com um hífen ('-') antes do '$'.\n");
            break;
        default:
            fprintf(stderr, "Ocorreu um erro desconhecido.\n");
            break;
    }
}

int main(int argc, char *argv[]) {
    #ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    #endif

    if (argc != 2) {
        fprintf(stderr, "Uso: %s <gramatica.txt>\n", argv[0]);
        fprintf(stderr, "Exemplo: %s gramatica.txt\n", argv[0]);
        return 1;
    }

    const char *nome_arquivo = argv[1];
    char *definicao_gramatica = ler_conteudo_arquivo(nome_arquivo);

    if (!definicao_gramatica) {
        return 1;
    }

    Grammar gramatica;
    inicializar_gramatica(&gramatica);

    CodigoErro status_analise = analisar_gramatica(definicao_gramatica, &gramatica);

    if (status_analise == SUCESSO_ANALISE) {
        imprimir_gramatica(&gramatica);
    } else {
        imprimir_mensagem_erro(status_analise);
    }

    liberar_gramatica(&gramatica);
    free(definicao_gramatica);

    return (status_analise == SUCESSO_ANALISE) ? 0 : 1;
}
