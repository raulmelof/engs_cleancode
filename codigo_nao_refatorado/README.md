# Reconhecedor de Gramáticas em C

![Language](https://img.shields.io/badge/Language-C-blue.svg)
![Status](https://img.shields.io/badge/Status-Projeto%20Acadêmico%20Concluído-green.svg)

## 📜 Descrição

Este projeto foi desenvolvido para a disciplina de Linguagens Formais e Autômatos (GRULFAT) no curso de Engenharia de Computação do IFSP - Câmpus Guarulhos. O objetivo é construir um programa em C que reconhece e valida a especificação de uma gramática formal a partir de uma cadeia de entrada, seguindo um conjunto de regras predefinidas.

## ✨ Funcionalidades Principais

- **Leitura de Arquivo:** O programa lê a especificação da gramática a partir de um arquivo de texto passado como argumento na linha de comando.
- **Validação Sintática:** Verifica se a cadeia de entrada segue o formato regular especificado: `<Regra>(-<Regra>)*$`.
- **Identificação de Símbolos:** Diferencia e extrai os conjuntos de símbolos Terminais (letras minúsculas) e Não-Terminais (letras maiúsculas).
- **Reconhecimento do Símbolo Inicial:** O primeiro símbolo não-terminal encontrado na entrada é definido como o símbolo inicial da gramática.
- **Suporte a Produções Vazias:** O caractere `!` é utilizado para representar produções vazias (epsilon/ε).
- **Exibição Estruturada:** Se a gramática for válida, o programa exibe seus componentes de forma organizada: o conjunto de não-terminais (N), terminais (T), o símbolo inicial (S) e as regras de produção (P).

## ✍️ Formato da Gramática de Entrada

A gramática deve ser fornecida em uma única linha, seguindo estas regras:
- O símbolo `>` separa o lado esquerdo (LHS) do lado direito (RHS) de uma produção.
- O símbolo `-` separa regras de produção consecutivas.
- O símbolo `$` indica o fim da gramática.

**Exemplo de um arquivo `gramatica.txt` válido: L>IM-M>sIM-M>!-I>x-I>y-I>z$**

Este exemplo representa a gramática G = ({L, M, I}, {s, x, y, z}, P, L), com as produções P = { L → IM, M → sIM, M → ε, I → x, I → y, I → z }.

## 🚀 Como Compilar e Executar

### Pré-requisitos
- Compilador GCC (GNU Compiler Collection) instalado.

### Passos

1.  **Clone o repositório** ou certifique-se de que os arquivos `main.c`, `reconhecedor_gramatica.c` e `reconhecedor_gramatica.h` estejam na mesma pasta.

2.  **Compile o projeto** através do terminal com o seguinte comando:
    ```bash
    gcc -o reconhecedor main.c reconhecedor_gramatica.c -Wall -Wextra -std=c99
    ```

3.  **Prepare o arquivo da gramática** (ex: `gramatica.txt`) com as regras desejadas.

4.  **Execute o programa**, passando o nome do arquivo como argumento:
    ```bash
    ./reconhecedor gramatica.txt
    ```
