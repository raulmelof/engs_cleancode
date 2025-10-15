#include <stdio.h>
#include <stdlib.h>
#include "reconhecedor_gramatica.h"

#ifdef _WIN32
#include <windows.h>
#endif //correção de caracteres especiais como ã e ε

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
    char *string_gramatica = ler_conteudo_arquivo(nome_arquivo);

    if (!string_gramatica) {
        return 1;
    }

    Grammar gramatica_obj;
    gramatica_obj.rules = NULL;
    gramatica_obj.non_terminals = NULL;
    gramatica_obj.terminals = NULL;


    if (analisar_gramatica(string_gramatica, &gramatica_obj)) {
        imprimir_gramatica(&gramatica_obj);
    } else {
        fprintf(stderr, "Falha ao analisar a gramática.\n");
    }

    liberar_gramatica(&gramatica_obj);
    free(string_gramatica);

    return 0;
}
