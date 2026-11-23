//bibliotecas que serao utilizadas
#include<stdio.h> //entrada e saida
#include<stdlib.h> //entrada e saida
#include<string.h> //manipulacao de strings
#include<ctype.h> //manipulacao de strings
#include<locale.h>//formatacao regional
#include<time.h>//captura e formatacao de datas
#include<float.h>//constantes
#include<limits.h>//constantes
#define TAM_NOME 50
#define TAM_CATEGORIA 50
#define CAPACIDADE_INICIAL 100
#define MAX_ITENS 100
#define VENDA_CONCLUIDA 1
#define VENDA_CANCELADA 0
#define TAM_DATA 11   // "DD/MM/AAAA" = 10 caracteres + '\0'

typedef struct produto{ 
    int codigo;
    char nome[TAM_NOME];
    char categoria[TAM_CATEGORIA];
    double preco_custo;
    double preco_venda;
    int qtd_estoque;
    int estoque_minimo;
}produto;  

typedef struct item_venda{
    int codigo_produto;
    int quantidade;
    double preco_unitario;
    double preco_custo;
}item_venda; 

typedef struct venda{
    int codigo;
    time_t data;
    item_venda itens[MAX_ITENS];//vetor de itens usando a constante MAX_ITENS
    int contador_itens;
    double desconto;//percentual(0 a 100)
    double valor_total;
    int status; // 0 - cancelada, 1 - concluída
    double valor_pago;
    double troco;
}venda;  

//função que formata a data para o formato DD/MM/AAAA
void formatarData(time_t t, char destino[]) {
    struct tm *info = localtime(&t);/*converte o time_t (segundos contados desde 1970) em uma struct tm(dia, mes, ano separados) usando localtime*/
    strftime(destino, TAM_DATA, "%d/%m/%Y", info);//converte em uma string formatafa
}
//função que retorna o dia da semana (0 = domingo, 1 = segunda-feira, ..., 6 = sábado)
int diaDaSemana(time_t t) {
    struct tm *info = localtime(&t); //obtem o endereco de memoria onde a variavel t está guardada na biblioteca time.h
    return info->tm_wday;//ponteiro para uma estrutura que usa o operador para acessar o dia da semana
}

// Desenha uma linha para separar partes do menu
void desenhaBorda() {
    printf("===========================\n");
}

// Limpa o terminal (Windows usa "cls", Linux e Mac usam "clear")
void limpaTela() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// Descarta tudo o que sobrou digitado até o Enter
void limparBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Para o programa até o usuário apertar Enter
void pausar() {
    printf("\nPressione Enter para continuar...");
    limparBuffer();
}

//funcao para ler inteiros com tratamento de erro
// Lê um inteiro entre minimo e maximo, repetindo até o usuário digitar um valor válido
int lerInteiro(const char *mensagem, int minimo, int maximo) {//recebe a mensagem a ser exibida, o valor minimo e o valor maximo
    int valor; //variavel que vai armazenar o valor digitado pelo usuario
    int leu; //variavel que vai armazenar quantos valores foram lidos com sucesso

    do {
        printf("%s", mensagem); //exibe a mensagem para o usuário
        leu = scanf("%d", &valor); //devolve quantos valores foram lidos com sucesso    
        limparBuffer(); //descarta o que sobrou no buffer de entrada

        if (leu != 1) { // Se não leu um inteiro, exibe mensagem de erro
            printf("Erro: digite apenas numeros.\n"); 
        } else if (valor < minimo || valor > maximo) { // Se o valor estiver fora do intervalo, exibe mensagem de erro
            printf("Erro: digite um valor entre %d e %d.\n", minimo, maximo); //exibe a mensagem de erro com o intervalo permitido
        }
    } while (leu != 1 || valor < minimo || valor > maximo); //repete enquanto não for digitado um valor válido

    return valor; //retorna o valor digitado pelo usuário
}

// Lê um decimal entre minimo e maximo, repetindo até o usuário digitar um valor válido
double lerDecimal(const char *mensagem, double minimo, double maximo) {
    double valor; //variavel para guardar o valor digitado pelo usuario
    int leu; //varavel para guardar o valor digitado pelo usuario

    do {
        printf("%s", mensagem);
        leu = scanf("%lf", &valor);
        limparBuffer();

        if (leu != 1) {
            printf("Erro: digite apenas numeros.\n");
        } else if (valor < minimo || valor > maximo) {
            printf("Erro: digite um valor entre %.2f e %.2f.\n", minimo, maximo);
        }
    } while (leu != 1 || valor < minimo || valor > maximo);

    return valor;
}

// Lê um texto (com espaços) de até tamanho-1 caracteres, repetindo até ser válido.
// Devolve 1 se leu, 0 se a entrada acabou (fim do arquivo).
int lerTexto(const char *mensagem, char destino[], int tamanho) { 
    int valido;

    do {
        valido = 1; //a funcao assume inicialmente que a entrada eh valida e muda para 0  caso alguma regra falhe
        printf("%s", mensagem);

        //le no maximo tamanho-1 caracteres do teclado e adiciona \0 no final
        if (fgets(destino, tamanho, stdin) == NULL) {
            destino[0] = '\0';
            return 0;
        }
        //  para remover o enter 
        size_t pos = strcspn(destino, "\n");
        if (destino[pos] == '\n') {
            destino[pos] = '\0';             // tirou o Enter que o fgets guardou
        } else {
            int c = getchar();               // não veio Enter: olha o próximo caractere
            if (c != '\n' && c != EOF) {     // sobrou texto, então foi digitado demais
                limparBuffer(); //para destacar orestante da linha
                printf("Erro: texto muito longo (maximo %d caracteres).\n", tamanho - 1);
                valido = 0;
            }
        }

        if (valido) { //so exetuca a checagem se o texto não tiver estourado o limite de caracteres
            int temConteudo = 0;
            for (int i = 0; destino[i] != '\0'; i++) {
                if (!isspace((unsigned char)destino[i])) { //verifica se cada caractere eh "' '", '\t', '\n'
                    temConteudo = 1; //se encontrar, marca 1 e interrompe 
                    break;
                }
            }
            if (!temConteudo) {
                printf("Erro: o texto nao pode ser vazio.\n");
                valido = 0;
            }
        }
    } while (!valido);

    return 1;
}

// FUNCOES COMPARTILHADAS ENTRE AS PARTES
// Retorna o indice do produto no vetor ou -1 se nao encontrar
int buscarIndiceProdutoPorCodigo(produto *produtos, int total_produtos, int codigo) {
    for (int i = 0; i < total_produtos; i++) {
        if (produtos[i].codigo == codigo) {
            return i;
        }
    }
    return -1;
}
//funcoes auxiliares da parte 1 
// Exibe os dados completos de um unico produto
void exibirDetalhesProduto(produto p) {
    desenhaBorda();
    printf("Codigo:         %d\n", p.codigo);
    printf("Nome:           %s\n", p.nome);
    printf("Categoria:      %s\n", p.categoria);
    printf("Preco de custo: R$ %.2f\n", p.preco_custo);
    printf("Preco de venda: R$ %.2f\n", p.preco_venda);
    printf("Estoque atual:  %d unidades\n", p.qtd_estoque);
    printf("Estoque minimo: %d unidades\n", p.estoque_minimo);
    desenhaBorda();
}
// Verifica se 'busca' aparece dentro de 'texto' ignorando maiusculas/minusculas
int contemTextoSemCase(const char *texto, const char *busca) {
    for (int i = 0; texto[i] != '\0'; i++) {
        int j = 0;
        while (busca[j] != '\0' && texto[i + j] != '\0' &&
               tolower((unsigned char)texto[i + j]) == tolower((unsigned char)busca[j])) {
            j++;
        }
        if (busca[j] == '\0') {
            return 1; // Encontrou a substring completa
        }
    }
    return 0;
}
//funcoes auxiliares de ordenacao 
// Compara duas strings ignorando maiusculas/minusculas (funciona como strcmp)
// Retorna < 0 se s1 < s2, 0 se forem iguais, > 0 se s1 > s2
int compararStringsSemCase(const char *s1, const char *s2) {
    int i = 0;
    while (s1[i] != '\0' && s2[i] != '\0') {
        char c1 = (char)tolower((unsigned char)s1[i]);
        char c2 = (char)tolower((unsigned char)s2[i]);
        if (c1 != c2) {
            return c1 - c2;
        }
        i++;
    }
    return (unsigned char)s1[i] - (unsigned char)s2[i];
}

// Ordena o vetor manualmente usando Selection Sort
// criterio: 1 = codigo, 2 = nome, 3 = preco de venda
void ordenarProdutos(produto *produtos, int total_produtos, int criterio) {
    for (int i = 0; i < total_produtos - 1; i++) {
        int indice_menor = i;

        for (int j = i + 1; j < total_produtos; j++) {
            int deve_trocar = 0;

            if (criterio == 1) {
                if (produtos[j].codigo < produtos[indice_menor].codigo) {
                    deve_trocar = 1;
                }
            } else if (criterio == 2) {
                if (compararStringsSemCase(produtos[j].nome, produtos[indice_menor].nome) < 0) {
                    deve_trocar = 1;
                }
            } else if (criterio == 3) {
                if (produtos[j].preco_venda < produtos[indice_menor].preco_venda) {
                    deve_trocar = 1;
                }
            }

            if (deve_trocar) {
                indice_menor = j;
            }
        }

        // Faz a troca apenas uma vez ao final da varredura
        if (indice_menor != i) {
            produto temp = produtos[i];
            produtos[i] = produtos[indice_menor];
            produtos[indice_menor] = temp;
        }
    }
}
// PARTE 1 - PRODUTOS E ESTOQUE (PLACEHOLDERS)
//funcao responsavel pelo cadastro de produtos no sistema
void cadastrarProduto(produto **produtos, int *total_produtos, int *capacidade_produtos) {
    limpaTela();
    desenhaBorda();
    printf("     CADASTRAR PRODUTO\n");
    desenhaBorda();

    // 1. Verifica se o vetor atingiu a capacidade maxima e dobra o tamanho
    if (*total_produtos == *capacidade_produtos) {
        int nova_capacidade = (*capacidade_produtos) * 2;
        produto *temp = realloc(*produtos, nova_capacidade * sizeof(produto));

        if (temp == NULL) {
            printf("Erro: memoria insuficiente para cadastrar novos produtos.\n");
            pausar();
            return;
        }

        *produtos = temp;
        *capacidade_produtos = nova_capacidade;
    }

    produto novo;

    // 2. Garante que o codigo seja unico (requisito obrigatorio)
    int codigo_existente;
    do {
        novo.codigo = lerInteiro("Codigo do produto: ", 1, INT_MAX);
        codigo_existente = buscarIndiceProdutoPorCodigo(*produtos, *total_produtos, novo.codigo);
        if (codigo_existente != -1) {
            printf("Erro: ja existe um produto cadastrado com o codigo %d.\n", novo.codigo);
        }
    } while (codigo_existente != -1);

    // 3. Leitura validada dos demais campos
    lerTexto("Nome do produto: ", novo.nome, TAM_NOME);
    lerTexto("Categoria: ", novo.categoria, TAM_CATEGORIA);
    novo.preco_custo = lerDecimal("Preco de custo (R$): ", 0.01, DBL_MAX);
    novo.preco_venda = lerDecimal("Preco de venda (R$): ", 0.01, DBL_MAX);

    if (novo.preco_venda < novo.preco_custo) {
        printf("Aviso: o preco de venda esta menor que o preco de custo.\n");
    }

    novo.qtd_estoque = lerInteiro("Quantidade inicial em estoque: ", 0, INT_MAX);
    novo.estoque_minimo = lerInteiro("Estoque minimo: ", 0, INT_MAX);

    // 4. Salva na proxima posicao livre e incrementa o total
    (*produtos)[*total_produtos] = novo;
    (*total_produtos)++;

    printf("\nProduto cadastrado com sucesso!\n");
    pausar();
}
void listarProdutos(produto *produtos, int total_produtos) {
    limpaTela();
    desenhaBorda();
    printf("       LISTAR PRODUTOS\n");
    desenhaBorda();

    if (total_produtos == 0) {
        printf("Nenhum produto cadastrado no sistema.\n");
        pausar();
        return;
    }

    printf("Ordenar listagem por:\n");
    printf("1. Codigo (crescente)\n");
    printf("2. Nome (ordem alfabetica)\n");
    printf("3. Preco de venda (menor para maior)\n");
    printf("0. Voltar\n");
    desenhaBorda();

    int criterio = lerInteiro("Escolha uma opcao: ", 0, 3);
    if (criterio == 0) return;

    ordenarProdutos(produtos, total_produtos, criterio);

    limpaTela();
    printf("==================================================================================\n");
    printf("%-6s | %-24s | %-14s | %-9s | %-9s | %-7s\n",
           "COD", "NOME", "CATEGORIA", "CUSTO", "VENDA", "ESTOQUE");
    printf("==================================================================================\n");

    for (int i = 0; i < total_produtos; i++) {
        printf("%-6d | %-24.24s | %-14.14s | %9.2f | %9.2f | %7d\n",
               produtos[i].codigo,
               produtos[i].nome,
               produtos[i].categoria,
               produtos[i].preco_custo,
               produtos[i].preco_venda,
               produtos[i].qtd_estoque);
    }

    printf("==================================================================================\n");
    printf("Total de produtos listados: %d\n", total_produtos);
    pausar();
}
//funcao para buscar o produto com codigo ou parte do nome 
void buscarProduto(produto *produtos, int total_produtos) {
    limpaTela();
    desenhaBorda();
    printf("       BUSCAR PRODUTO\n");
    desenhaBorda();

    if (total_produtos == 0) {
        printf("Nenhum produto cadastrado no sistema.\n");
        pausar();
        return;
    }

    printf("1. Buscar por codigo\n");
    printf("2. Buscar por parte do nome\n");
    printf("0. Voltar\n");
    desenhaBorda();

    int opcao = lerInteiro("Escolha o tipo de busca: ", 0, 2);
    if (opcao == 0) return;

    if (opcao == 1) {
        int codigo = lerInteiro("Digite o codigo do produto: ", 1, INT_MAX);
        int idx = buscarIndiceProdutoPorCodigo(produtos, total_produtos, codigo);

        if (idx == -1) {
            printf("\nProduto com codigo %d nao encontrado.\n", codigo);
        } else {
            printf("\nProduto encontrado:\n");
            exibirDetalhesProduto(produtos[idx]);
        }
    } else {
        char termo[TAM_NOME];
        lerTexto("Digite o nome (ou parte dele): ", termo, TAM_NOME);

        int encontrados = 0;
        printf("\nResultados da busca por \"%s\":\n", termo);

        for (int i = 0; i < total_produtos; i++) {
            if (contemTextoSemCase(produtos[i].nome, termo)) {
                exibirDetalhesProduto(produtos[i]);
                encontrados++;
            }
        }

        if (encontrados == 0) {
            printf("Nenhum produto encontrado com esse termo.\n");
        } else {
            printf("Total encontrado: %d produto(s).\n", encontrados);
        }
    }

    pausar();
}
//funcao para alterar o cadastro do produto
void alterarProduto(produto *produtos, int total_produtos) {
    limpaTela();
    desenhaBorda();
    printf("   ALTERAR CADASTRO DE PRODUTO\n");
    desenhaBorda();

    if (total_produtos == 0) {
        printf("Nenhum produto cadastrado no sistema.\n");
        pausar();
        return;
    }

    int codigo = lerInteiro("Digite o codigo do produto a alterar: ", 1, INT_MAX);
    int idx = buscarIndiceProdutoPorCodigo(produtos, total_produtos, codigo);

    if (idx == -1) {
        printf("\nProduto com codigo %d nao encontrado.\n", codigo);
        pausar();
        return;
    }

    int opcao;
    do {
        limpaTela();
        printf("Dados atuais do produto:\n");
        exibirDetalhesProduto(produtos[idx]);

        printf("Qual campo deseja alterar?\n");
        printf("1. Nome\n");
        printf("2. Categoria\n");
        printf("3. Preco de custo\n");
        printf("4. Preco de venda\n");
        printf("5. Estoque minimo\n");
        printf("0. Concluir alteracoes\n");
        desenhaBorda();

        opcao = lerInteiro("Escolha uma opcao: ", 0, 5);

        switch (opcao) {
            case 1:
                lerTexto("Novo nome: ", produtos[idx].nome, TAM_NOME);
                printf("Nome atualizado!\n");
                break;
            case 2:
                lerTexto("Nova categoria: ", produtos[idx].categoria, TAM_CATEGORIA);
                printf("Categoria atualizada!\n");
                break;
            case 3:
                produtos[idx].preco_custo = lerDecimal("Novo preco de custo (R$): ", 0.01, DBL_MAX);
                printf("Preco de custo atualizado!\n");
                break;
            case 4:
                produtos[idx].preco_venda = lerDecimal("Novo preco de venda (R$): ", 0.01, DBL_MAX);
                printf("Preco de venda atualizado!\n");
                break;
            case 5:
                produtos[idx].estoque_minimo = lerInteiro("Novo estoque minimo: ", 0, INT_MAX);
                printf("Estoque minimo atualizado!\n");
                break;
            case 0:
                break;
        }
    } while (opcao != 0);

    printf("\nAlteracoes finalizadas.\n");
    pausar();
}

//funcao para excluir produto 
void excluirProduto(produto *produtos, int *total_produtos) {
    limpaTela();
    desenhaBorda();
    printf("       EXCLUIR PRODUTO\n");
    desenhaBorda();

    if (*total_produtos == 0) {
        printf("Nenhum produto cadastrado no sistema.\n");
        pausar();
        return;
    }

    int codigo = lerInteiro("Digite o codigo do produto a excluir: ", 1, INT_MAX);
    int idx = buscarIndiceProdutoPorCodigo(produtos, *total_produtos, codigo);

    if (idx == -1) {
        printf("\nProduto com codigo %d nao encontrado.\n", codigo);
        pausar();
        return;
    }

    printf("\nProduto localizado:\n");
    exibirDetalhesProduto(produtos[idx]);

    int confirma = lerInteiro("Tem certeza que deseja excluir este produto? (1 - Sim / 2 - Nao): ", 1, 2);
    if (confirma == 2) {
        printf("\nExclusao cancelada.\n");
        pausar();
        return;
    }

    // Desloca todos os elementos posteriores uma posicao para a esquerda
    for (int i = idx; i < (*total_produtos) - 1; i++) {
        produtos[i] = produtos[i + 1];
    }

    (*total_produtos)--;
    printf("\nProduto excluido com sucesso!\n");
    pausar();
}
//funcao para registrar entrada de mercadoria no estoque 
void entradaEstoque(produto *produtos, int total_produtos) {
    limpaTela();
    desenhaBorda();
    printf("     ENTRADA DE ESTOQUE\n");
    desenhaBorda();

    if (total_produtos == 0) {
        printf("Nenhum produto cadastrado no sistema.\n");
        pausar();
        return;
    }

    int codigo = lerInteiro("Digite o codigo do produto: ", 1, INT_MAX);
    int idx = buscarIndiceProdutoPorCodigo(produtos, total_produtos, codigo);

    if (idx == -1) {
        printf("\nProduto com codigo %d nao encontrado.\n", codigo);
        pausar();
        return;
    }

    printf("\nProduto: %s\n", produtos[idx].nome);
    printf("Estoque atual: %d unidades\n", produtos[idx].qtd_estoque);
    desenhaBorda();

    // Evita estouro do tipo int e garante que maximo >= 1
    int espaco_disponivel = INT_MAX - produtos[idx].qtd_estoque;
    if (espaco_disponivel <= 0) {
        printf("Erro: o estoque deste produto ja atingiu o limite maximo.\n");
        pausar();
        return;
    }

    int qtd_entrada = lerInteiro("Quantidade a adicionar: ", 1, espaco_disponivel);
    produtos[idx].qtd_estoque += qtd_entrada;

    printf("\nEntrada registrada com sucesso!\n");
    printf("Novo saldo em estoque: %d unidades\n", produtos[idx].qtd_estoque);
    pausar();
}
//funcao para retirar mercadoria do estoque 
void baixaEstoque(produto *produtos, int total_produtos) {
    limpaTela();
    desenhaBorda();
    printf("   BAIXA / RETIRADA DE ESTOQUE\n");
    desenhaBorda();

    if (total_produtos == 0) {
        printf("Nenhum produto cadastrado no sistema.\n");
        pausar();
        return;
    }

    int codigo = lerInteiro("Digite o codigo do produto: ", 1, INT_MAX);
    int idx = buscarIndiceProdutoPorCodigo(produtos, total_produtos, codigo);

    if (idx == -1) {
        printf("\nProduto com codigo %d nao encontrado.\n", codigo);
        pausar();
        return;
    }

    printf("\nProduto: %s\n", produtos[idx].nome);
    printf("Estoque atual: %d unidades\n", produtos[idx].qtd_estoque);
    desenhaBorda();

    // Se o estoque estiver zerado, bloqueia antes de chamar lerInteiro
    if (produtos[idx].qtd_estoque == 0) {
        printf("Operacao cancelada: este produto esta com estoque zerado.\n");
        pausar();
        return;
    }

    // Passar qtd_estoque como maximo impede retirar mais do que ha disponivel
    int qtd_baixa = lerInteiro("Quantidade a retirar: ", 1, produtos[idx].qtd_estoque);
    produtos[idx].qtd_estoque -= qtd_baixa;

    printf("\nBaixa registrada com sucesso!\n");
    printf("Novo saldo em estoque: %d unidades\n", produtos[idx].qtd_estoque);

    // Alerta imediato caso a retirada tenha deixado o produto em nivel critico
    if (produtos[idx].qtd_estoque == 0) {
        printf("ALERTA: O estoque deste produto agora esta ZERADO!\n");
    } else if (produtos[idx].qtd_estoque <= produtos[idx].estoque_minimo) {
        printf("ALERTA: Estoque abaixo ou igual ao minimo (%d unidades)!\n",
               produtos[idx].estoque_minimo);
    }

    pausar();
}
//funcao para listar produtos com estoque baixo ou zerado
void listarEstoqueBaixo(produto *produtos, int total_produtos) {
    limpaTela();
    desenhaBorda();
    printf("  PRODUTOS COM ESTOQUE BAIXO OU ZERADO\n");
    desenhaBorda();

    if (total_produtos == 0) {
        printf("Nenhum produto cadastrado no sistema.\n");
        pausar();
        return;
    }

    int encontrados = 0;

    printf("===========================================================================\n");
    printf("%-6s | %-25s | %-15s | %-7s | %-7s | %-8s\n",
           "COD", "NOME", "CATEGORIA", "ATUAL", "MINIMO", "SITUACAO");
    printf("===========================================================================\n");

    for (int i = 0; i < total_produtos; i++) {
        if (produtos[i].qtd_estoque <= produtos[i].estoque_minimo) {
            const char *situacao = (produtos[i].qtd_estoque == 0) ? "ZERADO" : "BAIXO";

            printf("%-6d | %-25.25s | %-15.15s | %7d | %7d | %-8s\n",
                   produtos[i].codigo,
                   produtos[i].nome,
                   produtos[i].categoria,
                   produtos[i].qtd_estoque,
                   produtos[i].estoque_minimo,
                   situacao);
            encontrados++;
        }
    }

    printf("===========================================================================\n");

    if (encontrados == 0) {
        printf("Tudo certo! Nenhum produto esta com estoque baixo ou zerado.\n");
    } else {
        printf("Total de produtos precisando de reposicao: %d\n", encontrados);
    }

    pausar();
}

// SUBMENUS
void menuProdutos(produto **produtos, int *total_produtos, int *capacidade_produtos) {
    int opcao;
    do {
        limpaTela();
        desenhaBorda();
        printf("      MENU DE PRODUTOS\n");
        desenhaBorda();
        printf("1. Cadastrar produto\n");
        printf("2. Listar produtos\n");
        printf("3. Buscar produto\n");
        printf("4. Alterar cadastro\n");
        printf("5. Excluir produto\n");
        printf("0. Voltar\n");
        desenhaBorda();

        opcao = lerInteiro("Escolha uma opcao: ", 0, 5);

        switch (opcao) {
            case 1: cadastrarProduto(produtos, total_produtos, capacidade_produtos); break;
            case 2: listarProdutos(*produtos, *total_produtos); break;
            case 3: buscarProduto(*produtos, *total_produtos); break;
            case 4: alterarProduto(*produtos, *total_produtos); break;
            case 5: excluirProduto(*produtos, total_produtos); break;
            case 0: break;
        }
    } while (opcao != 0);
}

void menuEstoque(produto *produtos, int total_produtos) {
    int opcao;
    do {
        limpaTela();
        desenhaBorda();
        printf("      MENU DE ESTOQUE\n");
        desenhaBorda();
        printf("1. Entrada de mercadoria\n");
        printf("2. Baixa/retirada de estoque\n");
        printf("3. Produtos com estoque baixo ou zerado\n");
        printf("0. Voltar\n");
        desenhaBorda();

        opcao = lerInteiro("Escolha uma opcao: ", 0, 3);

        switch (opcao) {
            case 1: entradaEstoque(produtos, total_produtos); break;
            case 2: baixaEstoque(produtos, total_produtos); break;
            case 3: listarEstoqueBaixo(produtos, total_produtos); break;
            case 0: break;
        }
    } while (opcao != 0);
}
// SUBMENUS DA PARTE 2 E 3 (SIMPLIFICADOS)
void menuVendas() {
    int opcao;
    do {
        limpaTela();
        desenhaBorda();
        printf("       MENU DE VENDAS\n");
        desenhaBorda();
        printf("1. Registrar venda\n");
        printf("2. Listar todas as vendas\n");
        printf("3. Consultar venda (codigo ou data)\n");
        printf("4. Cancelar venda\n");
        printf("0. Voltar\n");
        desenhaBorda();

        opcao = lerInteiro("Escolha uma opcao: ", 0, 4);

        if (opcao != 0) {
            printf("Em desenvolvimento\n");
            pausar();
        }
    } while (opcao != 0);
}

void menuRelatorios() {
    int opcao;
    do {
        limpaTela();
        desenhaBorda();
        printf("     MENU DE RELATORIOS\n");
        desenhaBorda();
        printf("1. Quantidade e valor total em estoque\n");
        printf("2. Produtos com estoque baixo ou zerado\n");
        printf("3. Ranking de produtos mais vendidos\n");
        printf("4. Vendas e faturamento por dia/periodo\n");
        printf("5. Lucro estimado das vendas\n");
        printf("6. Vendas por categoria e dia da semana\n");
        printf("0. Voltar\n");
        desenhaBorda();

        opcao = lerInteiro("Escolha uma opcao: ", 0, 6);

        if (opcao != 0) {
            printf("Em desenvolvimento\n");
            pausar();
        }
    } while (opcao != 0);
}
// FUNCAO PRINCIPAL
int main() {
    int total_produtos = 0;
    int capacidade_produtos = CAPACIDADE_INICIAL;
    produto *produtos = malloc(capacidade_produtos * sizeof(produto));

    if (produtos == NULL) {
        printf("Erro critico: nao foi possivel alocar memoria para produtos.\n");
        return 1;
    }

    int total_vendas = 0;
    int capacidade_vendas = CAPACIDADE_INICIAL;
    venda *vendas = malloc(capacidade_vendas * sizeof(venda));

    if (vendas == NULL) {
        printf("Erro critico: nao foi possivel alocar memoria para vendas.\n");
        free(produtos);
        return 1;
    }

    int opcao;
    do {
        limpaTela();
        desenhaBorda();
        printf("   SISTEMA DE SUPERMERCADO\n");
        desenhaBorda();
        printf("1. Produtos\n");
        printf("2. Estoque\n");
        printf("3. Vendas\n");
        printf("4. Relatorios\n");
        printf("5. Sair\n");
        desenhaBorda();

        opcao = lerInteiro("Escolha uma opcao: ", 1, 5);

        switch (opcao) {
            case 1: menuProdutos(&produtos, &total_produtos, &capacidade_produtos); break;
            case 2: menuEstoque(produtos, total_produtos); break;
            case 3: menuVendas(); break;
            case 4: menuRelatorios(); break;
            case 5: printf("\nEncerrando o sistema...\n"); break;
        }
    } while (opcao != 5);

    free(produtos);
    produtos = NULL;
    free(vendas);
    vendas = NULL;

    return 0;
}
