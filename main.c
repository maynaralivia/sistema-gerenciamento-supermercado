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
#define LIMITE_VALOR_PAGO 1000000000.0
#define ARQ_PRODUTOS "produtos.txt"
#define ARQ_VENDAS "vendas.txt"
#define ARQ_TEMP "temp_dados.tmp" 

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
    char nome[TAM_NOME];
    char categoria[TAM_CATEGORIA];
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
// Funcao que troca o arquivo definitivo pelo temporario ja completo.
// No Windows o rename nao sobrescreve, entao remove o antigo e tenta de novo.
int substituirArquivo(const char *temporario, const char *definitivo) {
    if (rename(temporario, definitivo) == 0) {
        return 1;
    }
    remove(definitivo);
    if (rename(temporario, definitivo) == 0) {
        return 1;
    }
    return 0;
}

// Le uma linha do arquivo e tira o \n (e \r) do final. Devolve 0 se acabou o arquivo.
int lerLinhaArquivo(FILE *f, char *destino, int tamanho) {
    if (fgets(destino, tamanho, f) == NULL) {
        return 0;
    }
    destino[strcspn(destino, "\r\n")] = '\0';
    return 1;
}
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
// ============================================================================
// >>> PARTE 3: VALIDACAO DOS DADOS CARREGADOS <<<
// ============================================================================
// ----------------------------------------------------------------------------
// Validacao dos dados lidos do arquivo (o arquivo pode ter sido alterado ou
// danificado fora do programa; um registro invalido faria o sistema travar).
// ----------------------------------------------------------------------------

// Garante que o texto termina em '\0' dentro do vetor (evita ler fora dele)
void garantirFimDeString(char texto[], int tamanho) {
    texto[tamanho - 1] = '\0'; // a ultima posicao sempre vira o terminador
}

// Retorna 1 se os campos do produto fazem sentido (>= funciona como filtro de NaN tambem)
int produtoValido(const produto *p) {
    return p->codigo >= 1 && p->qtd_estoque >= 0 && p->estoque_minimo >= 0 &&
           p->preco_custo >= 0.0 && p->preco_venda >= 0.0;
}

// Retorna 1 se os campos da venda (e de todos os seus itens) fazem sentido
int vendaValida(const venda *v) {
    if (v->codigo < 1 || v->contador_itens < 0 || v->contador_itens > MAX_ITENS) return 0;
    if (v->status != VENDA_CONCLUIDA && v->status != VENDA_CANCELADA) return 0;
    if (!(v->desconto >= 0.0 && v->desconto <= 100.0)) return 0;
    if (!(v->valor_total >= 0.0 && v->valor_pago >= 0.0 && v->troco >= 0.0)) return 0;

    for (int i = 0; i < v->contador_itens; i++) {
        if (v->itens[i].codigo_produto < 1 || v->itens[i].quantidade < 1) return 0;
        if (!(v->itens[i].preco_unitario >= 0.0 && v->itens[i].preco_custo >= 0.0)) return 0;
    }
    return 1;
}

// Valida todos os produtos carregados e confere se nao ha codigos repetidos
int validarProdutosCarregados(produto *produtos, int total_produtos) {
    for (int i = 0; i < total_produtos; i++) {
        garantirFimDeString(produtos[i].nome, TAM_NOME);
        garantirFimDeString(produtos[i].categoria, TAM_CATEGORIA);
        if (!produtoValido(&produtos[i])) return 0;
        for (int j = 0; j < i; j++) { // codigo unico (requisito do trabalho)
            if (produtos[j].codigo == produtos[i].codigo) return 0;
        }
    }
    return 1;
}

// Valida todas as vendas carregadas e confere se nao ha codigos repetidos
int validarVendasCarregadas(venda *vendas, int total_vendas) {
    for (int i = 0; i < total_vendas; i++) {
        for (int j = 0; j < vendas[i].contador_itens && j < MAX_ITENS; j++) {
            garantirFimDeString(vendas[i].itens[j].nome, TAM_NOME);
            garantirFimDeString(vendas[i].itens[j].categoria, TAM_CATEGORIA);
        }
        if (!vendaValida(&vendas[i])) return 0;
        for (int j = 0; j < i; j++) {
            if (vendas[j].codigo == vendas[i].codigo) return 0;
        }
    }
    return 1;
}

// Renomeia um arquivo com problema para "nome.corrompido".
// Assim, o salvamento seguinte nao sobrescreve (e nao perde) os dados originais.
void preservarArquivoCorrompido(const char *nome_arquivo) {
    char nome_novo[80];
    snprintf(nome_novo, sizeof(nome_novo), "%s.corrompido", nome_arquivo);
    remove(nome_novo); // apaga uma copia anterior, se existir
    if (rename(nome_arquivo, nome_novo) == 0) {
        printf("O arquivo original foi guardado como \"%s\".\n", nome_novo);
    }
}
// ARMAZENAMENTO DE PRODUTOS
// Formato de produtos.txt: 1a linha = total; depois, para cada produto:
// codigo / nome / categoria / "custo venda estoque minimo"
int salvarProdutos(const produto *produtos, int total_produtos) {
    FILE *f = fopen(ARQ_TEMP, "w");
    if (f == NULL) {
        printf("Aviso: nao foi possivel salvar os produtos.\n");
        return 0;
    }

    fprintf(f, "%d\n", total_produtos);
    for (int i = 0; i < total_produtos; i++) {
        fprintf(f, "%d\n%s\n%s\n%.6f %.6f %d %d\n",
                produtos[i].codigo, produtos[i].nome, produtos[i].categoria,
                produtos[i].preco_custo, produtos[i].preco_venda,
                produtos[i].qtd_estoque, produtos[i].estoque_minimo);
    }

    // so troca o arquivo antigo se o temporario foi gravado por inteiro
    if (fclose(f) != 0 || !substituirArquivo(ARQ_TEMP, ARQ_PRODUTOS)) {
        printf("Aviso: falha ao gravar o arquivo de produtos.\n");
        return 0;
    }
    return 1;
}

// Le um produto do arquivo. Devolve 1 se leu completo, 0 se faltou algo.
int lerProdutoDoArquivo(FILE *f, produto *p) {
    char linha[256];

    if (!lerLinhaArquivo(f, linha, sizeof(linha)) || sscanf(linha, "%d", &p->codigo) != 1) return 0;
    if (!lerLinhaArquivo(f, linha, sizeof(linha))) return 0;
    snprintf(p->nome, TAM_NOME, "%s", linha);
    if (!lerLinhaArquivo(f, linha, sizeof(linha))) return 0;
    snprintf(p->categoria, TAM_CATEGORIA, "%s", linha);
    if (!lerLinhaArquivo(f, linha, sizeof(linha)) ||
        sscanf(linha, "%lf %lf %d %d", &p->preco_custo, &p->preco_venda,
               &p->qtd_estoque, &p->estoque_minimo) != 4) return 0;
    return 1;
}

// Carrega os produtos ao iniciar. Se o arquivo nao existe (primeira execucao), comeca vazio.
// Devolve 1 se esta tudo certo, 0 se houve problema.
int carregarProdutos(produto **produtos, int *total_produtos, int *capacidade_produtos) {
    FILE *f = fopen(ARQ_PRODUTOS, "r");
    if (f == NULL) {
        return 1;
    }

    char linha[256];
    int qtd;
    if (!lerLinhaArquivo(f, linha, sizeof(linha)) || sscanf(linha, "%d", &qtd) != 1 || qtd < 0) {
        printf("Aviso: arquivo de produtos invalido. Iniciando sem produtos.\n");
        fclose(f);
        preservarArquivoCorrompido(ARQ_PRODUTOS); // PARTE 3: nao deixa o proximo salvamento apagar o original
        return 0;
    }

    // se o arquivo tem mais produtos que a capacidade atual, aumenta o vetor
    if (qtd > *capacidade_produtos) {
        produto *temp = realloc(*produtos, qtd * sizeof(produto));
        if (temp == NULL) {
            printf("Erro: memoria insuficiente para carregar os produtos.\n");
            fclose(f);
            return 0;
        }
        *produtos = temp;
        *capacidade_produtos = qtd;
    }

    int lidos = 0;
    while (lidos < qtd && lerProdutoDoArquivo(f, &(*produtos)[lidos])) {
        lidos++;
    }
    fclose(f);

  if (lidos != qtd) {
        printf("Aviso: arquivo de produtos incompleto (%d de %d lidos).\n", lidos, qtd);
        preservarArquivoCorrompido(ARQ_PRODUTOS); // PARTE 3: guarda o original
        *total_produtos = 0;
        return 0;
    }
    if (!validarProdutosCarregados(*produtos, qtd)) { // PARTE 3: codigos repetidos, valores negativos...
        printf("Aviso: arquivo de produtos com dados invalidos.\n");
        preservarArquivoCorrompido(ARQ_PRODUTOS);
        *total_produtos = 0;
        return 0;
    }
    *total_produtos = qtd;
    return 1;
}
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
    salvarProdutos(*produtos, *total_produtos);   // <- LINHA NOVA

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

    salvarProdutos(produtos, total_produtos);     // <- LINHA NOVA
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
    salvarProdutos(produtos, *total_produtos);    // <- LINHA NOVA
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
    salvarProdutos(produtos, total_produtos);

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
    salvarProdutos(produtos, total_produtos);     

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
// FUNCOES DA PARTE 2
// ARMAZENAMENTO DE VENDAS
// Formato de vendas.txt: 1a linha = total; depois, para cada venda:
// "codigo data itens status" / "desconto total pago troco" /
// e para cada item: "codigo quantidade preco_unit preco_custo" / nome / categoria
int salvarVendas(const venda *vendas, int total_vendas) {
    FILE *f = fopen(ARQ_TEMP, "w");
    if (f == NULL) {
        printf("Aviso: nao foi possivel salvar as vendas.\n");
        return 0;
    }

    fprintf(f, "%d\n", total_vendas);
    for (int i = 0; i < total_vendas; i++) {
        const venda *v = &vendas[i];
        fprintf(f, "%d %lld %d %d\n", v->codigo, (long long)v->data, v->contador_itens, v->status);
        fprintf(f, "%.6f %.6f %.6f %.6f\n", v->desconto, v->valor_total, v->valor_pago, v->troco);
        for (int j = 0; j < v->contador_itens; j++) {
            const item_venda *it = &v->itens[j];
            fprintf(f, "%d %d %.6f %.6f\n%s\n%s\n",
                    it->codigo_produto, it->quantidade, it->preco_unitario, it->preco_custo,
                    it->nome, it->categoria);
        }
    }

    if (fclose(f) != 0 || !substituirArquivo(ARQ_TEMP, ARQ_VENDAS)) {
        printf("Aviso: falha ao gravar o arquivo de vendas.\n");
        return 0;
    }
    return 1;
}

// Le uma venda (com seus itens) do arquivo. Devolve 1 se leu completa, 0 se faltou algo.
int lerVendaDoArquivo(FILE *f, venda *v) {
    char linha[256];
    long long data;
    memset(v, 0, sizeof(venda));

    if (!lerLinhaArquivo(f, linha, sizeof(linha)) ||
        sscanf(linha, "%d %lld %d %d", &v->codigo, &data, &v->contador_itens, &v->status) != 4) return 0;
    v->data = (time_t)data;
    if (v->contador_itens < 0 || v->contador_itens > MAX_ITENS) return 0;

    if (!lerLinhaArquivo(f, linha, sizeof(linha)) ||
        sscanf(linha, "%lf %lf %lf %lf", &v->desconto, &v->valor_total,
               &v->valor_pago, &v->troco) != 4) return 0;

    for (int i = 0; i < v->contador_itens; i++) {
        item_venda *it = &v->itens[i];
        if (!lerLinhaArquivo(f, linha, sizeof(linha)) ||
            sscanf(linha, "%d %d %lf %lf", &it->codigo_produto, &it->quantidade,
                   &it->preco_unitario, &it->preco_custo) != 4) return 0;
        if (!lerLinhaArquivo(f, linha, sizeof(linha))) return 0;
        snprintf(it->nome, TAM_NOME, "%s", linha);
        if (!lerLinhaArquivo(f, linha, sizeof(linha))) return 0;
        snprintf(it->categoria, TAM_CATEGORIA, "%s", linha);
    }
    return 1;
}

// Carrega as vendas ao iniciar. Se o arquivo nao existe (primeira execucao), comeca vazio.
int carregarVendas(venda **vendas, int *total_vendas, int *capacidade_vendas) {
    FILE *f = fopen(ARQ_VENDAS, "r");
    if (f == NULL) {
        return 1;
    }

    char linha[256];
    int qtd;
    if (!lerLinhaArquivo(f, linha, sizeof(linha)) || sscanf(linha, "%d", &qtd) != 1 || qtd < 0) {
        printf("Aviso: arquivo de vendas invalido. Iniciando sem vendas.\n");
        fclose(f);
        preservarArquivoCorrompido(ARQ_VENDAS); // PARTE 3
        return 0;
    }

    if (qtd > *capacidade_vendas) {
        venda *temp = realloc(*vendas, qtd * sizeof(venda));
        if (temp == NULL) {
            printf("Erro: memoria insuficiente para carregar as vendas.\n");
            fclose(f);
            return 0;
        }
        *vendas = temp;
        *capacidade_vendas = qtd;
    }

    int lidas = 0;
    while (lidas < qtd && lerVendaDoArquivo(f, &(*vendas)[lidas])) {
        lidas++;
    }
    fclose(f);

       if (lidas != qtd) {
        printf("Aviso: arquivo de vendas incompleto (%d de %d lidas).\n", lidas, qtd);
        preservarArquivoCorrompido(ARQ_VENDAS); // PARTE 3
        *total_vendas = 0;
        return 0;
    }
    if (!validarVendasCarregadas(*vendas, qtd)) { // PARTE 3
        printf("Aviso: arquivo de vendas com dados invalidos.\n");
        preservarArquivoCorrompido(ARQ_VENDAS);
        *total_vendas = 0;
        return 0;
    }
    *total_vendas = qtd;
    return 1;
}
// Gera o codigo da proxima venda, maior codigo de venda existente + 1 (continua correto depois de carregar de arquivo)
int proximoCodigoVenda(venda *vendas, int total_vendas) {
    int maior = 0;
    for (int i = 0; i < total_vendas; i++) {
        if (vendas[i].codigo > maior) {
            maior = vendas[i].codigo;
        }
    }
    return maior + 1;
}

// Retorna o indice da venda no vetor ou -1 se nao encontrar
int buscarIndiceVendaPorCodigo(venda *vendas, int total_vendas, int codigo) {
    for (int i = 0; i < total_vendas; i++) {
        if (vendas[i].codigo == codigo) {
            return i;
        }
    }
    return -1;
}

// Arredonda para 2 casas decimais (evita erros de ponto flutuante no troco)
double arredondarCentavos(double valor) {
    return (long long)(valor * 100.0 + 0.5) / 100.0;
}

// Soma quantidade * preco de todos os itens (antes do desconto)
double calcularSubtotal(const venda *v) {
    double subtotal = 0.0;
    for (int i = 0; i < v->contador_itens; i++) {
        subtotal += v->itens[i].quantidade * v->itens[i].preco_unitario;
    }
    return subtotal;
}

// Imprime a nota fiscal em qualquer saida (stdout ou arquivo)
void imprimirNotaFiscal(FILE *saida, const venda *v) {
    char data[TAM_DATA];
    char hora[6];
    formatarData(v->data, data);
    struct tm *info = localtime(&v->data);
    strftime(hora, sizeof(hora), "%H:%M", info);

    double subtotal = calcularSubtotal(v);

    fprintf(saida, "==================================================================\n");
    fprintf(saida, "                    NOTA FISCAL SIMPLIFICADA\n");
    fprintf(saida, "==================================================================\n");
    fprintf(saida, "Venda no: %d        Data: %s %s\n", v->codigo, data, hora);
    fprintf(saida, "Status: %s\n", v->status == VENDA_CONCLUIDA ? "CONCLUIDA" : "CANCELADA");
    fprintf(saida, "==================================================================\n");
    fprintf(saida, "%-5s | %-24s | %5s | %10s | %10s\n", "COD", "PRODUTO", "QTD", "UNIT (R$)", "TOTAL (R$)");
    fprintf(saida, "==================================================================\n");

    for (int i = 0; i < v->contador_itens; i++) {
        const item_venda *it = &v->itens[i];
        fprintf(saida, "%-5d | %-24.24s | %5d | %10.2f | %10.2f\n",
                it->codigo_produto, it->nome, it->quantidade,
                it->preco_unitario, it->quantidade * it->preco_unitario);
    }

    fprintf(saida, "==================================================================\n");
    fprintf(saida, "Subtotal:        R$ %10.2f\n", subtotal);
    fprintf(saida, "Desconto (%.2f%%): R$ %10.2f\n", v->desconto, arredondarCentavos(subtotal - v->valor_total));
    fprintf(saida, "TOTAL:           R$ %10.2f\n", v->valor_total);
    fprintf(saida, "Valor pago:      R$ %10.2f\n", v->valor_pago);
    fprintf(saida, "Troco:           R$ %10.2f\n", v->troco);
    fprintf(saida, "==================================================================\n");
}

// Salva a nota em nota_venda_<codigo>.txt
void salvarNotaFiscal(const venda *v) {
    char nome_arquivo[64];
    snprintf(nome_arquivo, sizeof(nome_arquivo), "nota_venda_%d.txt", v->codigo);

    FILE *f = fopen(nome_arquivo, "w");
    if (f == NULL) {
        printf("Aviso: nao foi possivel salvar a nota fiscal em arquivo.\n");
        return;
    }
    imprimirNotaFiscal(f, v);
    fclose(f);
    printf("Nota fiscal salva em \"%s\".\n", nome_arquivo);
}

// ---------- REGISTRAR VENDA ----------
void registrarVenda(venda **vendas, int *total_vendas, int *capacidade_vendas,
                    produto *produtos, int total_produtos) {
    limpaTela();
    desenhaBorda();
    printf("       REGISTRAR VENDA\n");
    desenhaBorda();

    if (total_produtos == 0) {
        printf("Nenhum produto cadastrado no sistema.\n");
        pausar();
        return;
    }

    // Garante espaco no vetor de vendas (dobra a capacidade se estiver cheio)
    if (*total_vendas == *capacidade_vendas) {
        int nova_capacidade = (*capacidade_vendas) * 2;
        venda *temp = realloc(*vendas, nova_capacidade * sizeof(venda));
        if (temp == NULL) {
            printf("Erro: memoria insuficiente para registrar novas vendas.\n");
            pausar();
            return;
        }
        *vendas = temp;
        *capacidade_vendas = nova_capacidade;
    }

    venda nova;
    memset(&nova, 0, sizeof(venda));
    nova.codigo = proximoCodigoVenda(*vendas, *total_vendas);
    nova.contador_itens = 0;

    printf("Venda no %d\n", nova.codigo);

    // 1. Monta o carrinho (o estoque so e alterado na confirmacao)
    while (1) {
        int codigo = lerInteiro("\nCodigo do produto (0 para finalizar itens): ", 0, INT_MAX);
        if (codigo == 0) {
            break;
        }

        int idx = buscarIndiceProdutoPorCodigo(produtos, total_produtos, codigo);
        if (idx == -1) {
            printf("Produto com codigo %d nao encontrado.\n", codigo);
            continue;
        }

        // Verifica se o produto ja esta no carrinho
        int pos_item = -1;
        for (int i = 0; i < nova.contador_itens; i++) {
            if (nova.itens[i].codigo_produto == codigo) {
                pos_item = i;
                break;
            }
        }

        int no_carrinho = (pos_item == -1) ? 0 : nova.itens[pos_item].quantidade;
        int disponivel = produtos[idx].qtd_estoque - no_carrinho;

        printf("Produto: %s | Preco: R$ %.2f | Disponivel: %d\n",
               produtos[idx].nome, produtos[idx].preco_venda, disponivel);

        if (disponivel <= 0) {
            printf("Sem estoque disponivel para este produto.\n");
            continue;
        }
        if (pos_item == -1 && nova.contador_itens >= MAX_ITENS) {
            printf("Limite de %d itens diferentes por venda atingido.\n", MAX_ITENS);
            continue;
        }

        int qtd = lerInteiro("Quantidade: ", 1, disponivel);

        if (pos_item == -1) {
            pos_item = nova.contador_itens;
            nova.contador_itens++;
            nova.itens[pos_item].codigo_produto = codigo;
            nova.itens[pos_item].quantidade = qtd;
            nova.itens[pos_item].preco_unitario = produtos[idx].preco_venda;
            nova.itens[pos_item].preco_custo = produtos[idx].preco_custo;
            snprintf(nova.itens[pos_item].nome, TAM_NOME, "%s", produtos[idx].nome);
            snprintf(nova.itens[pos_item].categoria, TAM_CATEGORIA, "%s", produtos[idx].categoria);
        } else {
            nova.itens[pos_item].quantidade += qtd;
        }

        printf("Adicionado: %d x %s = R$ %.2f\n", qtd, produtos[idx].nome,
               qtd * produtos[idx].preco_venda);
    }

    if (nova.contador_itens == 0) {
        printf("\nNenhum item informado. Venda descartada.\n");
        pausar();
        return;
    }

    // 2. Desconto e total
    double subtotal = calcularSubtotal(&nova);
    printf("\nSubtotal: R$ %.2f\n", subtotal);
    nova.desconto = lerDecimal("Desconto em % (0 a 100): ", 0.0, 100.0);
    nova.valor_total = arredondarCentavos(subtotal * (1.0 - nova.desconto / 100.0));
    printf("TOTAL A PAGAR: R$ %.2f\n", nova.valor_total);

    // 3. Pagamento e troco
    nova.valor_pago = lerDecimal("Valor pago pelo cliente (R$): ", nova.valor_total, LIMITE_VALOR_PAGO);
    nova.troco = arredondarCentavos(nova.valor_pago - nova.valor_total);
    printf("Troco: R$ %.2f\n", nova.troco);

    // 4. Confirmacao
    int confirma = lerInteiro("\nConfirmar venda? (1 - Sim / 2 - Nao): ", 1, 2);
    if (confirma == 2) {
        printf("\nVenda descartada. O estoque nao foi alterado.\n");
        pausar();
        return;
    }

    // 5. Verificacao final da disponibilidade de TODOS os itens antes de dar baixa
    for (int i = 0; i < nova.contador_itens; i++) {
        int idx = buscarIndiceProdutoPorCodigo(produtos, total_produtos, nova.itens[i].codigo_produto);
        if (idx == -1 || produtos[idx].qtd_estoque < nova.itens[i].quantidade) {
            printf("\nErro: estoque insuficiente para \"%s\". Venda nao realizada.\n", nova.itens[i].nome);
            pausar();
            return;
        }
    }

    // 6. Da baixa no estoque, finaliza e guarda no historico
    for (int i = 0; i < nova.contador_itens; i++) {
        int idx = buscarIndiceProdutoPorCodigo(produtos, total_produtos, nova.itens[i].codigo_produto);
        produtos[idx].qtd_estoque -= nova.itens[i].quantidade;
    }

    nova.data = time(NULL);
    nova.status = VENDA_CONCLUIDA;

    (*vendas)[*total_vendas] = nova;
    (*total_vendas)++;
    salvarVendas(*vendas, *total_vendas);         
    salvarProdutos(produtos, total_produtos);     

    printf("\nVenda registrada com sucesso!\n\n");
    imprimirNotaFiscal(stdout, &nova);
    salvarNotaFiscal(&nova);

    // Alertas de estoque dos produtos vendidos
    for (int i = 0; i < nova.contador_itens; i++) {
        int idx = buscarIndiceProdutoPorCodigo(produtos, total_produtos, nova.itens[i].codigo_produto);
        if (produtos[idx].qtd_estoque == 0) {
            printf("ALERTA: \"%s\" ficou com estoque ZERADO!\n", produtos[idx].nome);
        } else if (produtos[idx].qtd_estoque <= produtos[idx].estoque_minimo) {
            printf("ALERTA: \"%s\" ficou com estoque baixo (%d un., minimo %d).\n",
                   produtos[idx].nome, produtos[idx].qtd_estoque, produtos[idx].estoque_minimo);
        }
    }

    pausar();
}

// ---------- HISTORICO ----------
void imprimirCabecalhoVendas() {
    printf("=======================================================================\n");
    printf("%-6s | %-16s | %-6s | %-9s | %-12s | %-10s\n",
           "COD", "DATA/HORA", "ITENS", "DESCONTO", "TOTAL (R$)", "STATUS");
    printf("=======================================================================\n");
}

void imprimirLinhaVenda(const venda *v) {
    char data[TAM_DATA];
    char hora[6];
    formatarData(v->data, data);
    struct tm *info = localtime(&v->data);
    strftime(hora, sizeof(hora), "%H:%M", info);

    char data_hora[20];
    snprintf(data_hora, sizeof(data_hora), "%s %s", data, hora);

    printf("%-6d | %-16s | %-6d | %8.2f%% | %12.2f | %-10s\n",
           v->codigo, data_hora, v->contador_itens, v->desconto, v->valor_total,
           v->status == VENDA_CONCLUIDA ? "CONCLUIDA" : "CANCELADA");
}

void listarVendas(venda *vendas, int total_vendas) {
    limpaTela();
    desenhaBorda();
    printf("     HISTORICO DE VENDAS\n");
    desenhaBorda();

    if (total_vendas == 0) {
        printf("Nenhuma venda registrada.\n");
        pausar();
        return;
    }

    imprimirCabecalhoVendas();
    for (int i = 0; i < total_vendas; i++) {
        imprimirLinhaVenda(&vendas[i]);
    }
    printf("=======================================================================\n");
    printf("Total de vendas: %d\n", total_vendas);
    pausar();
}

int diasNoMes(int mes, int ano) {
    int dias[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int bissexto = (ano % 4 == 0 && ano % 100 != 0) || (ano % 400 == 0);
    if (mes == 2 && bissexto) {
        return 29;
    }
    return dias[mes - 1];
}

// Le uma data DD/MM/AAAA valida e devolve normalizada em destino (tamanho TAM_DATA).
// Retorna 1 se leu, 0 se a entrada acabou.
int lerData(const char *mensagem, char destino[]) {
    char texto[TAM_NOME];
    int d, m, a;
    char extra;

    while (1) {
        if (!lerTexto(mensagem, texto, TAM_NOME)) {
            return 0;
        }
        if (sscanf(texto, "%d/%d/%d %c", &d, &m, &a, &extra) == 3 &&
            a >= 1900 && a <= 9999 && m >= 1 && m <= 12 &&
            d >= 1 && d <= diasNoMes(m, a)) {
            snprintf(destino, TAM_DATA, "%02d/%02d/%04d", d, m, a);
            return 1;
        }
        printf("Erro: data invalida. Use o formato DD/MM/AAAA.\n");
    }
}

void consultarVenda(venda *vendas, int total_vendas) {
    limpaTela();
    desenhaBorda();
    printf("     CONSULTAR VENDA\n");
    desenhaBorda();

    if (total_vendas == 0) {
        printf("Nenhuma venda registrada.\n");
        pausar();
        return;
    }

    printf("1. Consultar por codigo\n");
    printf("2. Consultar por data\n");
    printf("0. Voltar\n");
    desenhaBorda();

    int opcao = lerInteiro("Escolha o tipo de consulta: ", 0, 2);
    if (opcao == 0) return;

    if (opcao == 1) {
        int codigo = lerInteiro("Digite o codigo da venda: ", 1, INT_MAX);
        int idx = buscarIndiceVendaPorCodigo(vendas, total_vendas, codigo);

        if (idx == -1) {
            printf("\nVenda %d nao encontrada.\n", codigo);
        } else {
            printf("\n");
            imprimirNotaFiscal(stdout, &vendas[idx]);
        }
    } else {
        char data_busca[TAM_DATA];
        if (!lerData("Digite a data (DD/MM/AAAA): ", data_busca)) {
            return;
        }

        int encontradas = 0;
        char data_venda[TAM_DATA];

        printf("\nVendas do dia %s:\n", data_busca);
        for (int i = 0; i < total_vendas; i++) {
            formatarData(vendas[i].data, data_venda);
            if (strcmp(data_venda, data_busca) == 0) {
                if (encontradas == 0) {
                    imprimirCabecalhoVendas();
                }
                imprimirLinhaVenda(&vendas[i]);
                encontradas++;
            }
        }

        if (encontradas == 0) {
            printf("Nenhuma venda encontrada nessa data.\n");
        } else {
            printf("=======================================================================\n");
            printf("Total encontrado: %d venda(s).\n", encontradas);
            printf("Use a consulta por codigo para ver a nota completa.\n");
        }
    }

    pausar();
}

//funcao para cancelar venda
void cancelarVenda(venda *vendas, int total_vendas, produto *produtos, int total_produtos) {
    limpaTela();
    desenhaBorda();
    printf("       CANCELAR VENDA\n");
    desenhaBorda();

    if (total_vendas == 0) {
        printf("Nenhuma venda registrada.\n");
        pausar();
        return;
    }

    int codigo = lerInteiro("Digite o codigo da venda a cancelar: ", 1, INT_MAX);
    int idx = buscarIndiceVendaPorCodigo(vendas, total_vendas, codigo);

    if (idx == -1) {
        printf("\nVenda %d nao encontrada.\n", codigo);
        pausar();
        return;
    }

    if (vendas[idx].status == VENDA_CANCELADA) {
        printf("\nEsta venda ja foi cancelada anteriormente.\n");
        pausar();
        return;
    }

    printf("\n");
    imprimirNotaFiscal(stdout, &vendas[idx]);

    int confirma = lerInteiro("Tem certeza que deseja cancelar esta venda? (1 - Sim / 2 - Nao): ", 1, 2);
    if (confirma == 2) {
        printf("\nCancelamento abortado.\n");
        pausar();
        return;
    }

    // Devolve cada item ao estoque
    for (int i = 0; i < vendas[idx].contador_itens; i++) {
        item_venda *it = &vendas[idx].itens[i];
        int p = buscarIndiceProdutoPorCodigo(produtos, total_produtos, it->codigo_produto);

        if (p == -1) {
            printf("Aviso: \"%s\" (cod. %d) foi excluido do cadastro; %d un. nao puderam ser devolvidas.\n",
                   it->nome, it->codigo_produto, it->quantidade);
        } else if (INT_MAX - produtos[p].qtd_estoque < it->quantidade) {
            printf("Aviso: devolver %d un. de \"%s\" estouraria o limite do estoque; item nao devolvido.\n",
                   it->quantidade, it->nome);
        } else {
            produtos[p].qtd_estoque += it->quantidade;
        }
    }

    vendas[idx].status = VENDA_CANCELADA;
    salvarVendas(vendas, total_vendas);
    salvarProdutos(produtos, total_produtos);
    printf("\nVenda cancelada e estoque atualizado!\n");
    pausar();
}

//menu vendas
void menuVendas(venda **vendas, int *total_vendas, int *capacidade_vendas,
                produto *produtos, int total_produtos) {
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

        switch (opcao) {
            case 1: registrarVenda(vendas, total_vendas, capacidade_vendas, produtos, total_produtos); break;
            case 2: listarVendas(*vendas, *total_vendas); break;
            case 3: consultarVenda(*vendas, *total_vendas); break;
            case 4: cancelarVenda(*vendas, *total_vendas, produtos, total_produtos); break;
            case 0: break;
        }
    } while (opcao != 0);
}


// ============================================================================
// >>> INICIO PARTE 3 (B) - RELATORIOS <<<
// Fica DEPOIS de lerData()/diasNoMes() e ANTES do menuRelatorios().
// ============================================================================

// Nomes dos arquivos de relatorio (sobrescritos a cada geracao)
#define ARQ_REL_ESTOQUE "relatorio_estoque.txt"          // relatorio 1
#define ARQ_REL_ESTOQUE_BAIXO "relatorio_estoque_baixo.txt" // relatorio 2
#define ARQ_REL_RANKING "relatorio_ranking.txt"          // relatorio 3
#define ARQ_REL_FATURAMENTO "relatorio_faturamento.txt"  // relatorio 4
#define ARQ_REL_LUCRO "relatorio_lucro.txt"              // relatorio 5
#define ARQ_REL_CATEGORIA "relatorio_categoria_dia.txt"  // relatorio 6

// Uma linha do ranking de mais vendidos (soma das quantidades de um produto)
typedef struct item_ranking{
    int codigo;          // codigo do produto
    char nome[TAM_NOME]; // nome do produto (copiado do item da venda)
    long long quantidade;// total de unidades vendidas (long long evita estouro do int)
}item_ranking;

// Uma linha da tabela "categoria x dia da semana"
typedef struct linha_categoria{
    char categoria[TAM_CATEGORIA]; // nome da categoria
    long long por_dia[7];          // unidades vendidas por dia (0 = domingo ... 6 = sabado)
    long long total;               // total de unidades da categoria
}linha_categoria;

// ============================================================================
// FUNCOES AUXILIARES DOS RELATORIOS
// Cada relatorio e escrito por uma funcao que recebe um FILE *saida.
// Chamando-a com stdout ele aparece na tela; chamando-a com um arquivo ele e salvo.
// ============================================================================

// Desenha uma linha de '=' (na tela ou no arquivo) com a largura informada
void desenhaLinhaRelatorio(FILE *saida, int largura) {
    for (int i = 0; i < largura; i++) {
        fputc('=', saida); // escreve um caractere '=' por vez
    }
    fputc('\n', saida);
}

// Escreve o cabecalho padrao: borda, titulo e data de geracao
void cabecalhoRelatorio(FILE *saida, const char *titulo) {
    char data[TAM_DATA];
    formatarData(time(NULL), data); // time(NULL) = data/hora de agora
    desenhaLinhaRelatorio(saida, 27);
    fprintf(saida, "  %s\n", titulo);
    fprintf(saida, "  Gerado em: %s\n", data);
    desenhaLinhaRelatorio(saida, 27);
}

// Abre o arquivo do relatorio para escrita; mostra erro e devolve NULL se falhar
FILE *abrirArquivoRelatorio(const char *nome_arquivo) {
    FILE *arq = fopen(nome_arquivo, "w"); // "w" = texto, apaga o conteudo anterior
    if (arq == NULL) {
        printf("\nErro: nao foi possivel criar o arquivo \"%s\".\n", nome_arquivo);
    }
    return arq;
}

// Fecha o arquivo do relatorio e informa se foi salvo corretamente
void finalizarRelatorio(FILE *arq, const char *nome_arquivo) {
    int erro = ferror(arq); // houve erro em alguma escrita?
    if (fclose(arq) != 0 || erro) {
        printf("\nErro: falha ao gravar o arquivo \"%s\".\n", nome_arquivo);
    } else {
        printf("\nRelatorio salvo no arquivo: %s\n", nome_arquivo);
    }
}

// Quantos itens de uma venda sao validos (protege contra contador fora do limite)
int contarItensValidos(venda v) {
    if (v.contador_itens < 0) return 0;
    if (v.contador_itens > MAX_ITENS) return MAX_ITENS;
    return v.contador_itens;
}

// Converte uma data "DD/MM/AAAA" (ja validada pela lerData) em time_t.
// fim_do_dia = 0 usa 00:00:00 (inicio do periodo); fim_do_dia = 1 usa 23:59:59.
// Retorna 1 se converteu e 0 se a data esta fora do intervalo que o sistema suporta.
int dataParaTimeT(const char *data, int fim_do_dia, time_t *destino) {
    int dia, mes, ano;
    if (sscanf(data, "%d/%d/%d", &dia, &mes, &ano) != 3) { // separa dia, mes e ano
        return 0;
    }

    struct tm info;
    memset(&info, 0, sizeof(info)); // zera todos os campos da struct
    info.tm_mday = dia;
    info.tm_mon = mes - 1;     // tm_mon vai de 0 a 11
    info.tm_year = ano - 1900; // tm_year conta a partir de 1900
    info.tm_hour = fim_do_dia ? 23 : 0;
    info.tm_min = fim_do_dia ? 59 : 0;
    info.tm_sec = fim_do_dia ? 59 : 0;
    info.tm_isdst = -1;        // deixa o sistema decidir sobre horario de verao

    time_t t = mktime(&info);  // converte struct tm em time_t
    if (t == (time_t)-1) {
        printf("Erro: data fora do intervalo suportado pelo sistema.\n");
        return 0;
    }

    *destino = t;
    return 1;
}

// ============================================================================
// RELATORIO 1 - QUANTIDADE DE PRODUTOS E VALOR TOTAL EM ESTOQUE
// ============================================================================
void escreverRelatorioEstoque(FILE *saida, produto *produtos, int total_produtos) {
    long long total_unidades = 0;  // soma das unidades em estoque
    int com_estoque = 0;           // produtos com pelo menos 1 unidade
    double valor_custo = 0.0;      // estoque avaliado pelo preco de custo
    double valor_venda = 0.0;      // estoque avaliado pelo preco de venda

    for (int i = 0; i < total_produtos; i++) {
        if (produtos[i].qtd_estoque > 0) { // so conta produtos disponiveis
            com_estoque++;
            total_unidades += produtos[i].qtd_estoque;
            valor_custo += produtos[i].qtd_estoque * produtos[i].preco_custo;
            valor_venda += produtos[i].qtd_estoque * produtos[i].preco_venda;
        }
    }

    cabecalhoRelatorio(saida, "RELATORIO DE ESTOQUE");
    fprintf(saida, "Produtos cadastrados:             %d\n", total_produtos);
    fprintf(saida, "Produtos disponiveis em estoque:  %d\n", com_estoque);
    fprintf(saida, "Total de unidades em estoque:     %lld\n", total_unidades);
    fprintf(saida, "Valor do estoque (preco custo):   R$ %.2f\n", valor_custo);
    fprintf(saida, "Valor do estoque (preco venda):   R$ %.2f\n", valor_venda);
    desenhaLinhaRelatorio(saida, 27);
}

void relatorioEstoque(produto *produtos, int total_produtos) {
    limpaTela();
    escreverRelatorioEstoque(stdout, produtos, total_produtos); // mostra na tela

    FILE *arq = abrirArquivoRelatorio(ARQ_REL_ESTOQUE);
    if (arq != NULL) {
        escreverRelatorioEstoque(arq, produtos, total_produtos); // salva no arquivo
        finalizarRelatorio(arq, ARQ_REL_ESTOQUE);
    }
    pausar();
}

// ============================================================================
// RELATORIO 2 - PRODUTOS COM ESTOQUE BAIXO OU ZERADO
// ============================================================================
void escreverRelatorioEstoqueBaixo(FILE *saida, produto *produtos, int total_produtos) {
    cabecalhoRelatorio(saida, "ESTOQUE BAIXO OU ZERADO");
    int encontrados = 0;

    fprintf(saida, "===========================================================================\n");
    fprintf(saida, "%-6s | %-25s | %-15s | %-7s | %-7s | %-8s\n",
            "COD", "NOME", "CATEGORIA", "ATUAL", "MINIMO", "SITUACAO");
    fprintf(saida, "===========================================================================\n");

    for (int i = 0; i < total_produtos; i++) {
        if (produtos[i].qtd_estoque <= produtos[i].estoque_minimo) { // inferior ou igual ao minimo
            const char *situacao = (produtos[i].qtd_estoque == 0) ? "ZERADO" : "BAIXO";
            fprintf(saida, "%-6d | %-25.25s | %-15.15s | %7d | %7d | %-8s\n",
                    produtos[i].codigo, produtos[i].nome, produtos[i].categoria,
                    produtos[i].qtd_estoque, produtos[i].estoque_minimo, situacao);
            encontrados++;
        }
    }

    fprintf(saida, "===========================================================================\n");
    if (encontrados == 0) {
        fprintf(saida, "Nenhum produto esta com estoque baixo ou zerado.\n");
    } else {
        fprintf(saida, "Total de produtos precisando de reposicao: %d\n", encontrados);
    }
}

void relatorioEstoqueBaixo(produto *produtos, int total_produtos) {
    limpaTela();
    escreverRelatorioEstoqueBaixo(stdout, produtos, total_produtos);

    FILE *arq = abrirArquivoRelatorio(ARQ_REL_ESTOQUE_BAIXO);
    if (arq != NULL) {
        escreverRelatorioEstoqueBaixo(arq, produtos, total_produtos);
        finalizarRelatorio(arq, ARQ_REL_ESTOQUE_BAIXO);
    }
    pausar();
}

// ============================================================================
// RELATORIO 3 - RANKING DOS PRODUTOS MAIS VENDIDOS (ORDEM DECRESCENTE)
// ============================================================================
// Soma as quantidades vendidas de cada produto (apenas vendas concluidas) e ordena.
// Usa os dados guardados nos itens da venda, entao funciona mesmo se o produto
// tiver sido excluido do cadastro depois. Retorna 0 se faltar memoria.
int montarRanking(venda *vendas, int total_vendas, item_ranking **ranking, int *total_ranking) {
    *ranking = NULL;
    *total_ranking = 0;

    long long total_itens = 0; // maximo de linhas distintas possiveis
    for (int i = 0; i < total_vendas; i++) {
        if (vendas[i].status == VENDA_CONCLUIDA) {
            total_itens += contarItensValidos(vendas[i]);
        }
    }
    if (total_itens == 0) {
        return 1; // nada vendido ainda: ranking vazio (nao e erro)
    }

    item_ranking *lista = malloc((size_t)total_itens * sizeof(item_ranking));
    if (lista == NULL) {
        return 0; // sem memoria
    }
    int quantidade_lista = 0;

    for (int i = 0; i < total_vendas; i++) {
        if (vendas[i].status != VENDA_CONCLUIDA) continue; // cancelada nao conta

        for (int j = 0; j < contarItensValidos(vendas[i]); j++) {
            item_venda item = vendas[i].itens[j];

            int pos = -1; // procura o produto na lista
            for (int k = 0; k < quantidade_lista; k++) {
                if (lista[k].codigo == item.codigo_produto) {
                    pos = k;
                    break;
                }
            }
            if (pos == -1) { // primeira vez que aparece: cria a linha
                pos = quantidade_lista;
                lista[pos].codigo = item.codigo_produto;
                snprintf(lista[pos].nome, TAM_NOME, "%s", item.nome);
                lista[pos].quantidade = 0;
                quantidade_lista++;
            }
            lista[pos].quantidade += item.quantidade;
        }
    }

    // Selection Sort decrescente por quantidade (sem funcoes prontas de ordenacao)
    for (int i = 0; i < quantidade_lista - 1; i++) {
        int indice_maior = i;
        for (int j = i + 1; j < quantidade_lista; j++) {
            if (lista[j].quantidade > lista[indice_maior].quantidade ||
                (lista[j].quantidade == lista[indice_maior].quantidade &&
                 lista[j].codigo < lista[indice_maior].codigo)) { // empate: menor codigo primeiro
                indice_maior = j;
            }
        }
        if (indice_maior != i) {
            item_ranking temp = lista[i];
            lista[i] = lista[indice_maior];
            lista[indice_maior] = temp;
        }
    }

    *ranking = lista;
    *total_ranking = quantidade_lista;
    return 1;
}

void escreverRelatorioRanking(FILE *saida, item_ranking *ranking, int total_ranking) {
    cabecalhoRelatorio(saida, "RANKING MAIS VENDIDOS");

    if (total_ranking == 0) {
        fprintf(saida, "Nenhuma venda concluida registrada.\n");
        return;
    }

    fprintf(saida, "=====================================================\n");
    fprintf(saida, "%-4s | %-6s | %-25s | %-10s\n", "POS", "COD", "NOME", "QTD VENDIDA");
    fprintf(saida, "=====================================================\n");
    for (int i = 0; i < total_ranking; i++) {
        fprintf(saida, "%-4d | %-6d | %-25.25s | %11lld\n",
                i + 1, ranking[i].codigo, ranking[i].nome, ranking[i].quantidade);
    }
    fprintf(saida, "=====================================================\n");
}

void relatorioRanking(venda *vendas, int total_vendas) {
    limpaTela();
    item_ranking *ranking = NULL;
    int total_ranking = 0;

    if (!montarRanking(vendas, total_vendas, &ranking, &total_ranking)) {
        printf("Erro: memoria insuficiente para gerar o ranking.\n");
        pausar();
        return;
    }

    escreverRelatorioRanking(stdout, ranking, total_ranking);

    FILE *arq = abrirArquivoRelatorio(ARQ_REL_RANKING);
    if (arq != NULL) {
        escreverRelatorioRanking(arq, ranking, total_ranking);
        finalizarRelatorio(arq, ARQ_REL_RANKING);
    }

    free(ranking); // libera a memoria (free(NULL) e permitido)
    pausar();
}

// ============================================================================
// RELATORIO 4 - TOTAL DE VENDAS E FATURAMENTO POR DIA / PERIODO
// ============================================================================
void escreverRelatorioFaturamento(FILE *saida, venda *vendas, int total_vendas,
                                  time_t inicio, time_t fim) {
    char data_inicial[TAM_DATA], data_final[TAM_DATA], data_dia[TAM_DATA];
    formatarData(inicio, data_inicial);
    formatarData(fim, data_final);

    cabecalhoRelatorio(saida, "FATURAMENTO POR PERIODO");
    fprintf(saida, "Periodo: %s a %s\n", data_inicial, data_final);
    fprintf(saida, "=====================================================\n");
    fprintf(saida, "%-12s | %-8s | %-14s\n", "DATA", "VENDAS", "FATURAMENTO");
    fprintf(saida, "=====================================================\n");

    int total_concluidas = 0, total_canceladas = 0; // contadores do periodo
    double faturamento_total = 0.0;

    // Percorre o periodo dia a dia, comecando em 00:00:00 do dia inicial
    struct tm dia_info = *localtime(&inicio); // copia a struct (localtime reaproveita o mesmo espaco)
    dia_info.tm_hour = 0;
    dia_info.tm_min = 0;
    dia_info.tm_sec = 0;
    dia_info.tm_isdst = -1;
    time_t inicio_dia = mktime(&dia_info);

    while (inicio_dia != (time_t)-1 && inicio_dia <= fim) {
        struct tm prox_info = dia_info;
        prox_info.tm_mday += 1;  // dia seguinte (mktime ajusta fim de mes/ano sozinho)
        prox_info.tm_isdst = -1;
        time_t prox_dia = mktime(&prox_info);
        if (prox_dia == (time_t)-1) break; // protege contra erro de conversao

        int concluidas_dia = 0;
        double faturamento_dia = 0.0;

        for (int i = 0; i < total_vendas; i++) {
            if (vendas[i].data >= inicio_dia && vendas[i].data < prox_dia) { // venda caiu neste dia
                if (vendas[i].status == VENDA_CONCLUIDA) {
                    concluidas_dia++;
                    faturamento_dia += vendas[i].valor_total;
                } else {
                    total_canceladas++;
                }
            }
        }

        if (concluidas_dia > 0) { // so imprime dias que tiveram venda
            formatarData(inicio_dia, data_dia);
            fprintf(saida, "%-12s | %8d | R$ %11.2f\n", data_dia, concluidas_dia, faturamento_dia);
        }
        total_concluidas += concluidas_dia;
        faturamento_total += faturamento_dia;

        dia_info = prox_info; // avanca para o proximo dia
        inicio_dia = prox_dia;
    }

    fprintf(saida, "=====================================================\n");
    if (total_concluidas == 0) {
        fprintf(saida, "Nenhuma venda concluida no periodo.\n");
    }
    fprintf(saida, "Vendas concluidas:  %d\n", total_concluidas);
    fprintf(saida, "Vendas canceladas:  %d\n", total_canceladas);
    fprintf(saida, "Faturamento total:  R$ %.2f\n", faturamento_total);
    if (total_concluidas > 0) {
        fprintf(saida, "Ticket medio:       R$ %.2f\n", faturamento_total / total_concluidas);
    }
    fprintf(saida, "=====================================================\n");
}

void relatorioFaturamento(venda *vendas, int total_vendas) {
    limpaTela();
    desenhaBorda();
    printf("  FATURAMENTO POR PERIODO\n");
    desenhaBorda();
    printf("Para ver um unico dia, use a mesma data nas duas perguntas.\n\n");

    char texto_data[TAM_DATA]; // lerData devolve a data como texto "DD/MM/AAAA"
    time_t inicio, fim;

    do { // repete ate a data inicial ser convertida com sucesso
        if (!lerData("Data inicial (DD/MM/AAAA): ", texto_data)) return;
    } while (!dataParaTimeT(texto_data, 0, &inicio));

    int periodo_ok;
    do { // repete ate a data final ser valida e igual ou posterior a inicial
        periodo_ok = 0;
        if (!lerData("Data final   (DD/MM/AAAA): ", texto_data)) return;
        if (dataParaTimeT(texto_data, 1, &fim)) {
            if (fim < inicio) {
                printf("Erro: a data final nao pode ser anterior a data inicial.\n");
            } else {
                periodo_ok = 1;
            }
        }
    } while (!periodo_ok);

    limpaTela();
    escreverRelatorioFaturamento(stdout, vendas, total_vendas, inicio, fim);

    FILE *arq = abrirArquivoRelatorio(ARQ_REL_FATURAMENTO);
    if (arq != NULL) {
        escreverRelatorioFaturamento(arq, vendas, total_vendas, inicio, fim);
        finalizarRelatorio(arq, ARQ_REL_FATURAMENTO);
    }
    pausar();
}

// ============================================================================
// RELATORIO 5 - LUCRO ESTIMADO DAS VENDAS
// lucro = (soma qtd x preco de venda) - desconto% - (soma qtd x preco de custo)
// ============================================================================
void escreverRelatorioLucro(FILE *saida, venda *vendas, int total_vendas) {
    cabecalhoRelatorio(saida, "LUCRO ESTIMADO DAS VENDAS");

    int total_concluidas = 0;
    double soma_bruto = 0.0, soma_desconto = 0.0, soma_custo = 0.0, soma_lucro = 0.0;
    char data[TAM_DATA];

    fprintf(saida, "=================================================================\n");
    fprintf(saida, "%-6s | %-10s | %10s | %10s | %10s | %10s\n",
            "VENDA", "DATA", "BRUTO", "DESCONTO", "CUSTO", "LUCRO");
    fprintf(saida, "=================================================================\n");

    for (int i = 0; i < total_vendas; i++) {
        if (vendas[i].status != VENDA_CONCLUIDA) continue; // cancelada nao gera lucro

        double bruto = 0.0, custo = 0.0;
        for (int j = 0; j < contarItensValidos(vendas[i]); j++) {
            bruto += vendas[i].itens[j].quantidade * vendas[i].itens[j].preco_unitario;
            custo += vendas[i].itens[j].quantidade * vendas[i].itens[j].preco_custo;
        }
        double valor_desconto = bruto * vendas[i].desconto / 100.0; // desconto e percentual
        double lucro = bruto - valor_desconto - custo;

        formatarData(vendas[i].data, data);
        fprintf(saida, "%-6d | %-10s | %10.2f | %10.2f | %10.2f | %10.2f\n",
                vendas[i].codigo, data, bruto, valor_desconto, custo, lucro);

        total_concluidas++;
        soma_bruto += bruto;
        soma_desconto += valor_desconto;
        soma_custo += custo;
        soma_lucro += lucro;
    }

    fprintf(saida, "=================================================================\n");
    if (total_concluidas == 0) {
        fprintf(saida, "Nenhuma venda concluida registrada.\n");
        return;
    }
    fprintf(saida, "Vendas consideradas: %d\n", total_concluidas);
    fprintf(saida, "Receita bruta:       R$ %.2f\n", soma_bruto);
    fprintf(saida, "Descontos:           R$ %.2f\n", soma_desconto);
    fprintf(saida, "Custo dos produtos:  R$ %.2f\n", soma_custo);
    fprintf(saida, "Lucro estimado:      R$ %.2f\n", soma_lucro);
    if (soma_bruto > 0.0) {
        fprintf(saida, "Margem sobre a receita bruta: %.2f%%\n", soma_lucro / soma_bruto * 100.0); // %% imprime '%'
    }
}

void relatorioLucro(venda *vendas, int total_vendas) {
    limpaTela();
    escreverRelatorioLucro(stdout, vendas, total_vendas);

    FILE *arq = abrirArquivoRelatorio(ARQ_REL_LUCRO);
    if (arq != NULL) {
        escreverRelatorioLucro(arq, vendas, total_vendas);
        finalizarRelatorio(arq, ARQ_REL_LUCRO);
    }
    pausar();
}

// ============================================================================
// RELATORIO 6 - UNIDADES VENDIDAS POR CATEGORIA E DIA DA SEMANA
// + CATEGORIA COM MAIOR QUANTIDADE DE UNIDADES VENDIDAS
// ============================================================================
// Monta a tabela (apenas vendas concluidas), ordenada por total decrescente.
// Retorna 0 se faltar memoria.
int montarTabelaCategorias(venda *vendas, int total_vendas, linha_categoria **tabela, int *total_categorias) {
    *tabela = NULL;
    *total_categorias = 0;

    long long total_itens = 0;
    for (int i = 0; i < total_vendas; i++) {
        if (vendas[i].status == VENDA_CONCLUIDA) {
            total_itens += contarItensValidos(vendas[i]);
        }
    }
    if (total_itens == 0) {
        return 1;
    }

    linha_categoria *lista = malloc((size_t)total_itens * sizeof(linha_categoria));
    if (lista == NULL) {
        return 0;
    }
    int quantidade_lista = 0;

    for (int i = 0; i < total_vendas; i++) {
        if (vendas[i].status != VENDA_CONCLUIDA) continue;

        int dia = diaDaSemana(vendas[i].data); // 0 a 6
        if (dia < 0 || dia > 6) continue;      // protecao extra

        for (int j = 0; j < contarItensValidos(vendas[i]); j++) {
            item_venda item = vendas[i].itens[j];

            int pos = -1; // procura a categoria (ignorando maiusculas/minusculas)
            for (int k = 0; k < quantidade_lista; k++) {
                if (compararStringsSemCase(lista[k].categoria, item.categoria) == 0) {
                    pos = k;
                    break;
                }
            }
            if (pos == -1) { // categoria nova: cria a linha zerada
                pos = quantidade_lista;
                snprintf(lista[pos].categoria, TAM_CATEGORIA, "%s", item.categoria);
                for (int d = 0; d < 7; d++) {
                    lista[pos].por_dia[d] = 0;
                }
                lista[pos].total = 0;
                quantidade_lista++;
            }
            lista[pos].por_dia[dia] += item.quantidade;
            lista[pos].total += item.quantidade;
        }
    }

    // Selection Sort decrescente por total (a 1a linha sera a categoria lider)
    for (int i = 0; i < quantidade_lista - 1; i++) {
        int indice_maior = i;
        for (int j = i + 1; j < quantidade_lista; j++) {
            if (lista[j].total > lista[indice_maior].total) {
                indice_maior = j;
            }
        }
        if (indice_maior != i) {
            linha_categoria temp = lista[i];
            lista[i] = lista[indice_maior];
            lista[indice_maior] = temp;
        }
    }

    *tabela = lista;
    *total_categorias = quantidade_lista;
    return 1;
}

void escreverRelatorioCategoria(FILE *saida, linha_categoria *tabela, int total_categorias) {
    const char *nomes_dias[7] = {"DOM", "SEG", "TER", "QUA", "QUI", "SEX", "SAB"};

    cabecalhoRelatorio(saida, "VENDAS POR CATEGORIA/DIA");

    if (total_categorias == 0) {
        fprintf(saida, "Nenhuma venda concluida registrada.\n");
        return;
    }

    fprintf(saida, "=========================================================================\n");
    fprintf(saida, "%-20s", "CATEGORIA");
    for (int d = 0; d < 7; d++) {
        fprintf(saida, " | %5s", nomes_dias[d]);
    }
    fprintf(saida, " | %7s\n", "TOTAL");
    fprintf(saida, "=========================================================================\n");

    for (int i = 0; i < total_categorias; i++) {
        fprintf(saida, "%-20.20s", tabela[i].categoria);
        for (int d = 0; d < 7; d++) {
            fprintf(saida, " | %5lld", tabela[i].por_dia[d]);
        }
        fprintf(saida, " | %7lld\n", tabela[i].total);
    }
    fprintf(saida, "=========================================================================\n");

    // Como a tabela esta ordenada, a 1a linha e a lider; varre para detectar empates
    long long maior_total = tabela[0].total;
    int lideres = 0;
    for (int i = 0; i < total_categorias; i++) {
        if (tabela[i].total == maior_total) lideres++;
    }
    if (lideres == 1) {
        fprintf(saida, "Categoria mais vendida: %s (%lld unidades)\n", tabela[0].categoria, maior_total);
    } else {
        fprintf(saida, "Empate entre %d categorias com %lld unidades cada:\n", lideres, maior_total);
        for (int i = 0; i < total_categorias; i++) {
            if (tabela[i].total == maior_total) fprintf(saida, " - %s\n", tabela[i].categoria);
        }
    }
}

void relatorioCategoriaDiaSemana(venda *vendas, int total_vendas) {
    limpaTela();
    linha_categoria *tabela = NULL;
    int total_categorias = 0;

    if (!montarTabelaCategorias(vendas, total_vendas, &tabela, &total_categorias)) {
        printf("Erro: memoria insuficiente para gerar o relatorio.\n");
        pausar();
        return;
    }

    escreverRelatorioCategoria(stdout, tabela, total_categorias);

    FILE *arq = abrirArquivoRelatorio(ARQ_REL_CATEGORIA);
    if (arq != NULL) {
        escreverRelatorioCategoria(arq, tabela, total_categorias);
        finalizarRelatorio(arq, ARQ_REL_CATEGORIA);
    }

    free(tabela);
    pausar();
}

// ----------------------------------------------------------------------------
// MENU DE RELATORIOS (substitui o menuRelatorios() simplificado)
// Agora recebe os vetores de produtos e vendas.
// ----------------------------------------------------------------------------
void menuRelatorios(produto *produtos, int total_produtos, venda *vendas, int total_vendas) {
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

        switch (opcao) {
            case 1: relatorioEstoque(produtos, total_produtos); break;
            case 2: relatorioEstoqueBaixo(produtos, total_produtos); break;
            case 3: relatorioRanking(vendas, total_vendas); break;
            case 4: relatorioFaturamento(vendas, total_vendas); break;
            case 5: relatorioLucro(vendas, total_vendas); break;
            case 6: relatorioCategoriaDiaSemana(vendas, total_vendas); break;
            case 0: break;
        }
    } while (opcao != 0);
}

// ============================================================================
// >>> FIM PARTE 3 (B) <<<
// ============================================================================

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
    int ok_produtos = carregarProdutos(&produtos, &total_produtos, &capacidade_produtos);
    int ok_vendas = carregarVendas(&vendas, &total_vendas, &capacidade_vendas);
    if (!ok_produtos || !ok_vendas) {
        pausar();   // para o aviso nao sumir quando o menu limpar a tela
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
            case 3: menuVendas(&vendas, &total_vendas, &capacidade_vendas, produtos, total_produtos); break;
            case 4: menuRelatorios(produtos, total_produtos, vendas, total_vendas); break;
            case 5: printf("\nEncerrando o sistema...\n");
                    salvarProdutos(produtos, total_produtos);
                    salvarVendas(vendas, total_vendas);
            break;
        }
    } while (opcao != 5);

    free(produtos);
    produtos = NULL;
    free(vendas);
    vendas = NULL;

    return 0;
}
