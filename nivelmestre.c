#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_STR 128
#define HASH_SIZE 101

typedef struct Sala {
    char nome[MAX_STR];
    char pista[MAX_STR];
    struct Sala *esquerda;
    struct Sala *direita;
} Sala;

typedef struct PistaNode {
    char pista[MAX_STR];
    struct PistaNode *esquerda;
    struct PistaNode *direita;
} PistaNode;

typedef struct HashEntry {
    char pista[MAX_STR];
    char suspeito[MAX_STR];
    struct HashEntry *prox;
} HashEntry;

/* ---------- utilitário: comparação case-insensitive ---------- */
int stricmp_local(const char *a, const char *b) {
    while (*a && *b) {
        char ca = tolower((unsigned char)*a);
        char cb = tolower((unsigned char)*b);
        if (ca != cb) return (ca < cb) ? -1 : 1;
        a++; b++;
    }
    if (*a) return 1;
    if (*b) return -1;
    return 0;
}

/* ---------- criar sala ---------- */
Sala* criarSala(const char *nome, const char *pista) {
    Sala *nova = (Sala*) malloc(sizeof(Sala));
    if (!nova) { fprintf(stderr, "Erro de memória.\n"); exit(1); }
    strncpy(nova->nome, nome, MAX_STR-1); nova->nome[MAX_STR-1] = '\0';
    if (pista) { strncpy(nova->pista, pista, MAX_STR-1); nova->pista[MAX_STR-1] = '\0'; }
    else nova->pista[0] = '\0';
    nova->esquerda = nova->direita = NULL;
    return nova;
}

/* ---------- BST de pistas ---------- */
PistaNode* criarNoPista(const char *pista) {
    PistaNode *n = (PistaNode*) malloc(sizeof(PistaNode));
    if (!n) { fprintf(stderr, "Erro de memória.\n"); exit(1); }
    strncpy(n->pista, pista, MAX_STR-1); n->pista[MAX_STR-1] = '\0';
    n->esquerda = n->direita = NULL;
    return n;
}

PistaNode* inserirPista(PistaNode *raiz, const char *pista) {
    if (!pista || strlen(pista) == 0) return raiz;
    if (!raiz) return criarNoPista(pista);
    int cmp = stricmp_local(pista, raiz->pista);
    if (cmp < 0) raiz->esquerda = inserirPista(raiz->esquerda, pista);
    else if (cmp > 0) raiz->direita = inserirPista(raiz->direita, pista);
    return raiz;
}

void exibirPistas(PistaNode *raiz) {
    if (!raiz) return;
    exibirPistas(raiz->esquerda);
    printf(" - %s\n", raiz->pista);
    exibirPistas(raiz->direita);
}

/* ---------- tabela hash ---------- */
unsigned long hashFunc(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = (unsigned char)*str++)) hash = ((hash << 5) + hash) + c;
    return hash % HASH_SIZE;
}

void inserirNaHash(HashEntry *tabela[], const char *pista, const char *suspeito) {
    unsigned long idx = hashFunc(pista);
    HashEntry *cur = tabela[idx];
    while (cur) {
        if (strcmp(cur->pista, pista) == 0) {
            strncpy(cur->suspeito, suspeito, MAX_STR-1); cur->suspeito[MAX_STR-1] = '\0';
            return;
        }
        cur = cur->prox;
    }
    HashEntry *novo = (HashEntry*) malloc(sizeof(HashEntry));
    if (!novo) { fprintf(stderr, "Erro de memória.\n"); exit(1); }
    strncpy(novo->pista, pista, MAX_STR-1); novo->pista[MAX_STR-1] = '\0';
    strncpy(novo->suspeito, suspeito, MAX_STR-1); novo->suspeito[MAX_STR-1] = '\0';
    novo->prox = tabela[idx];
    tabela[idx] = novo;
}

const char* encontrarSuspeito(HashEntry *tabela[], const char *pista) {
    unsigned long idx = hashFunc(pista);
    HashEntry *cur = tabela[idx];
    while (cur) {
        if (strcmp(cur->pista, pista) == 0) return cur->suspeito;
        cur = cur->prox;
    }
    return NULL;
}

/* ---------- exploração ---------- */
void explorarSalas(Sala *atual, PistaNode **raizPistas) {
    char opcao;
    while (atual != NULL) {
        printf("\nVocê está na sala: %s\n", atual->nome);
        if (strlen(atual->pista) > 0) {
            printf("Você encontrou a pista: \"%s\"\n", atual->pista);
            *raizPistas = inserirPista(*raizPistas, atual->pista);
        } else {
            printf("Nenhuma pista nesta sala.\n");
        }

        printf("\nOpções:\n");
        if (atual->esquerda) printf(" (e) Ir para %s (esquerda)\n", atual->esquerda->nome);
        if (atual->direita)  printf(" (d) Ir para %s (direita)\n", atual->direita->nome);
        printf(" (s) Sair da exploração\n");
        printf("Escolha: ");
        scanf(" %c", &opcao);

        if (opcao == 'e' && atual->esquerda) atual = atual->esquerda;
        else if (opcao == 'd' && atual->direita) atual = atual->direita;
        else if (opcao == 's') {
            printf("Exploração encerrada.\n");
            break;
        } else {
            printf("Opção inválida ou caminho inexistente. Tente novamente.\n");
        }
    }
}

/* ---------- contagem de pistas por suspeito (percorre BST) ---------- */
int contarPistasPorSuspeito(PistaNode *raiz, HashEntry *tabela[], const char *suspeitoAlvo) {
    if (!raiz) return 0;
    int count = 0;
    const char *sus = encontrarSuspeito(tabela, raiz->pista);
    if (sus && strcmp(sus, suspeitoAlvo) == 0) count = 1;
    return count + contarPistasPorSuspeito(raiz->esquerda, tabela, suspeitoAlvo)
                 + contarPistasPorSuspeito(raiz->direita, tabela, suspeitoAlvo);
}

/* ---------- julgamento final ---------- */
void verificarSuspeitoFinal(PistaNode *raizPistas, HashEntry *tabela[]) {
    printf("\n===== PISTAS COLETADAS =====\n");
    if (!raizPistas) {
        printf("Nenhuma pista coletada. Não há base para acusar.\n");
        return;
    }
    exibirPistas(raizPistas);

    char acusado[MAX_STR];
    printf("\nDigite o nome do suspeito que deseja acusar: ");
    getchar(); // consome newline pendente
    if (!fgets(assunto_buffer := acusado, MAX_STR, stdin)) {
        acusado[0] = '\0';
    } else {
        // remove newline
        size_t L = strlen(acusado);
        if (L > 0 && acusado[L-1] == '\n') acusado[L-1] = '\0';
    }

    // Se usuário não digitou nada
    if (strlen(acusado) == 0) {
        printf("Nenhum suspeito informado.\n");
        return;
    }

    int qtd = contarPistasPorSuspeito(raizPistas, tabela, acusado);
    printf("\nPistas que apontam para %s: %d\n", acusado, qtd);
    if (qtd >= 2) {
        printf("Veredito: Acusação bem fundamentada! O culpado provavelmente é %s.\n", acusado);
    } else {
        printf("Veredito: Pistas insuficientes. A acusação contra %s não se sustenta.\n", acusado);
    }
}

/* ---------- limpeza de memória da hash ---------- */
void liberarHash(HashEntry *tabela[]) {
    for (int i = 0; i < HASH_SIZE; i++) {
        HashEntry *cur = tabela[i];
        while (cur) {
            HashEntry *aux = cur;
            cur = cur->prox;
            free(aux);
        }
        tabela[i] = NULL;
    }
}

/* ---------- liberar BST de pistas ---------- */
void liberarPistas(PistaNode *raiz) {
    if (!raiz) return;
    liberarPistas(raiz->esquerda);
    liberarPistas(raiz->direita);
    free(raiz);
}

/* ---------- liberar salas (árvore da mansão) ---------- */
void liberarSalas(Sala *raiz) {
    if (!raiz) return;
    liberarSalas(raiz->esquerda);
    liberarSalas(raiz->direita);
    free(raiz);
}

/* ---------- main: monta mapa, popula hash e inicia exploração ---------- */
int main() {
    // cria salas (mapa fixo)
    Sala *hall = criarSala("Hall de Entrada", "Chave antiga sobre a mesa");
    Sala *salaEstar = criarSala("Sala de Estar", "Pegada de lama no tapete");
    Sala *cozinha = criarSala("Cozinha", "Copo quebrado na pia");
    Sala *biblioteca = criarSala("Biblioteca", "Livro fora do lugar");
    Sala *jardim = criarSala("Jardim", "");
    Sala *escritorio = criarSala("Escritório", "Envelope com selo rasgado");
    Sala *porao = criarSala("Porão Misterioso", "Relógio parado às 10h15");

    // monta ligações
    hall->esquerda = salaEstar; hall->direita = cozinha;
    salaEstar->esquerda = biblioteca; salaEstar->direita = jardim;
    cozinha->direita = escritorio; escritorio->esquerda = porao;

    // inicializa BST de pistas coletadas (vazia)
    PistaNode *pistasColetadas = NULL;

    // inicializa tabela hash e popula associações pista -> suspeito
    HashEntry *tabela[HASH_SIZE];
    for (int i = 0; i < HASH_SIZE; i++) tabela[i] = NULL;

    // associações estáticas (definidas pelo designer do jogo)
    inserirNaHash(tabela, "Chave antiga sobre a mesa", "Sr. Black");
    inserirNaHash(tabela, "Pegada de lama no tapete", "Sra. White");
    inserirNaHash(tabela, "Copo quebrado na pia", "Sr. Green");
    inserirNaHash(tabela, "Livro fora do lugar", "Sra. White");
    inserirNaHash(tabela, "Envelope com selo rasgado", "Sr. Black");
    inserirNaHash(tabela, "Relógio parado às 10h15", "Sr. Green");
    // (pistas sem associação ficam sem entrada na hash)

    printf("=== Detective Quest: Mistério na Mansão (Modo Mestre) ===\n");
    printf("Comece a investigação a partir do Hall de Entrada.\n");

    // exploração
    explorarSalas(hall, &pistasColetadas);

    // ao final, julgamento
    verificarSuspeitoFinal(pistasColetadas, tabela);

    // liberar memória
    liberarPistas(pistasColetadas);
    liberarHash(tabela);
    liberarSalas(hall); // libera toda árvore (recursivamente)

    return 0;
}