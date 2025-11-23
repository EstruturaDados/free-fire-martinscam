#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

/* ===== Config ===== */
#define MAX_COMP 10

/* ===== Tipos ===== */
typedef struct {
    char nome[30];
    char tipo[20];
    int  prioridade; /* 1..10 */
} Componente;

/* ===== Utils ===== */
static void limparEntrada(void){
    int c; while((c=getchar())!='\n' && c!=EOF){}
}
static void lerString(const char* prompt, char* dst, size_t tam){
    printf("%s", prompt);
    if(!fgets(dst, (int)tam, stdin)){ dst[0]='\0'; return; }
    size_t n=strlen(dst);
    if(n>0 && dst[n-1]=='\n') dst[n-1]='\0';
}
static int lerIntFaixa(const char* prompt, int min, int max){
    int v;
    for(;;){
        printf("%s", prompt);
        if(scanf("%d",&v)!=1){ printf("Entrada invalida. Digite numero inteiro.\n"); limparEntrada(); continue; }
        limparEntrada();
        if(v<min || v>max){ printf("Valor fora da faixa [%d..%d].\n", min, max); continue; }
        return v;
    }
}
static int cmp_ci(const char* a, const char* b){
    while(*a && *b){
        int ca=tolower((unsigned char)*a);
        int cb=tolower((unsigned char)*b);
        if(ca!=cb) return ca-cb;
        a++; b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

/* ===== View ===== */
static void header(int n, int ordenadoPorNome){
    printf("============================================================\n");
    printf("  PLANO DE FUGA - CODIGO DA ILHA (NIVEL MESTRE)\n");
    printf("============================================================\n");
    printf("Itens na Mochila: %d/%d\n", n, MAX_COMP);
    printf("Status da Ordenacao por Nome: %s\n", ordenadoPorNome ? "ORDENADO" : "NAO ORDENADO");
    printf("------------------------------------------------------------\n");
}
static void menuPrincipal(void){
    printf("1. Adicionar Componente\n");
    printf("2. Descartar Componente\n");
    printf("3. Listar Componentes (Inventario)\n");
    printf("4. Organizar Mochila (Ordenar Componentes)\n");
    printf("5. Busca Binaria por Componente-Chave (por nome)\n");
    printf("0. ATIVAR TORRE DE FUGA (Sair)\n");
    printf("------------------------------------------------------------\n");
    printf("Opcao: ");
}
static void mostrarComponentes(const Componente v[], int n){
    printf("\n%-3s | %-28s | %-18s | %-7s\n", "#", "Nome", "Tipo", "Prior");
    printf("----+------------------------------+--------------------+---------\n");
    for(int i=0;i<n;i++){
        printf("%-3d | %-28s | %-18s | %-7d\n",
               i+1, v[i].nome, v[i].tipo, v[i].prioridade);
    }
    if(n==0) printf("(vazio)\n");
}

/* ===== Ordenacoes (com comparacoes) ===== */
static long bubbleSortNome(Componente v[], int n){
    long comps=0;
    for(int pass=0; pass<n-1; pass++){
        int trocou=0;
        for(int j=0; j<n-1-pass; j++){
            comps++;
            if(cmp_ci(v[j].nome, v[j+1].nome) > 0){
                Componente t=v[j]; v[j]=v[j+1]; v[j+1]=t; trocou=1;
            }
        }
        if(!trocou) break;
    }
    return comps;
}
static long insertionSortTipo(Componente v[], int n){
    long comps=0;
    for(int i=1;i<n;i++){
        Componente chave=v[i];
        int j=i-1;
        while(j>=0){
            comps++;
            if(cmp_ci(v[j].tipo, chave.tipo) > 0){ v[j+1]=v[j]; j--; }
            else break;
        }
        v[j+1]=chave;
    }
    return comps;
}
static long selectionSortPrioridade(Componente v[], int n){
    long comps=0;
    for(int i=0;i<n-1;i++){
        int min=i;
        for(int j=i+1;j<n;j++){
            comps++;
            if(v[j].prioridade < v[min].prioridade) min=j;
        }
        if(min!=i){ Componente t=v[i]; v[i]=v[min]; v[min]=t; }
    }
    return comps;
}

/* ===== Busca binaria por NOME ===== */
static int buscaBinariaPorNome(Componente v[], int n, const char* nome, long* comps){
    int ini=0, fim=n-1;
    if(comps) *comps=0;
    while(ini<=fim){
        int m=(ini+fim)/2;
        if(comps) (*comps)++;
        int c=cmp_ci(v[m].nome, nome);
        if(c==0) return m;
        else if(c<0) ini=m+1; else fim=m-1;
    }
    return -1;
}

/* ===== Auxiliares ===== */
static void copiar(const Componente src[], Componente dst[], int n){
    for(int i=0;i<n;i++) dst[i]=src[i];
}
static int cadastrar(Componente v[], int n){
    if(n>=MAX_COMP){ printf("Limite atingido (%d).\n", MAX_COMP); return n; }
    Componente c;
    lerString("Nome do componente: ", c.nome, sizeof(c.nome));
    lerString("Tipo do componente (ex: controle/suporte/propulsao): ", c.tipo, sizeof(c.tipo));
    c.prioridade = lerIntFaixa("Prioridade (1..10): ", 1, 10);
    v[n]=c;
    printf("Componente cadastrado.\n");
    return n+1;
}
static int descartar(Componente v[], int n){
    if(n==0){ printf("Inventario vazio.\n"); return n; }
    char alvo[30]; lerString("Nome do componente a descartar: ", alvo, sizeof(alvo));
    for(int i=0;i<n;i++){
        if(cmp_ci(v[i].nome, alvo)==0){
            for(int j=i;j<n-1;j++) v[j]=v[j+1];
            printf("Componente descartado.\n");
            return n-1;
        }
    }
    printf("Componente nao encontrado.\n");
    return n;
}

/* ===== Submenu de ordenacao ===== */
static int submenuOrdenar(Componente base[], int n, int* ordenadoPorNome){
    if(n==0){ printf("Nada a ordenar. Inventario vazio.\n"); return *ordenadoPorNome; }

    printf("\n--- Estrategias de Ordenacao ---\n");
    printf("1) Bubble Sort (por NOME)\n");
    printf("2) Insertion Sort (por TIPO)\n");
    printf("3) Selection Sort (por PRIORIDADE)\n");
    printf("Opcao: ");
    int op; if(scanf("%d",&op)!=1){ limparEntrada(); return *ordenadoPorNome; }
    limparEntrada();

    Componente work[MAX_COMP]; copiar(base, work, n);
    clock_t t0=clock(); long comps=0; clock_t t1; double secs;

    switch(op){
        case 1:
            comps = bubbleSortNome(work, n);
            t1=clock(); secs=(double)(t1-t0)/CLOCKS_PER_SEC;
            printf("Ordenado por NOME (Bubble). Comparacoes: %ld | Tempo: %.6fs\n", comps, secs);
            mostrarComponentes(work, n);
            copiar(work, base, n);
            *ordenadoPorNome = 1; /* habilita busca binaria */
            break;
        case 2:
            comps = insertionSortTipo(work, n);
            t1=clock(); secs=(double)(t1-t0)/CLOCKS_PER_SEC;
            printf("Ordenado por TIPO (Insertion). Comparacoes: %ld | Tempo: %.6fs\n", comps, secs);
            mostrarComponentes(work, n);
            copiar(work, base, n);
            *ordenadoPorNome = 0;
            break;
        case 3:
            comps = selectionSortPrioridade(work, n);
            t1=clock(); secs=(double)(t1-t0)/CLOCKS_PER_SEC;
            printf("Ordenado por PRIORIDADE (Selection). Comparacoes: %ld | Tempo: %.6fs\n", comps, secs);
            mostrarComponentes(work, n);
            copiar(work, base, n);
            *ordenadoPorNome = 0;
            break;
        default:
            printf("Opcao invalida.\n");
    }
    return *ordenadoPorNome;
}

/* ===== Main ===== */
int main(void){
    Componente base[MAX_COMP]; int n=0;
    int ordenadoPorNome = 0;

    int op;
    do{
        header(n, ordenadoPorNome);
        menuPrincipal();
        if(scanf("%d",&op)!=1){ limparEntrada(); continue; }
        limparEntrada();

        switch(op){
            case 1: n = cadastrar(base, n); break;
            case 2: n = descartar(base, n); break;
            case 3: mostrarComponentes(base, n); break;
            case 4: submenuOrdenar(base, n, &ordenadoPorNome); break;
            case 5: {
                if(!ordenadoPorNome){
                    printf("Para busca binaria por NOME, ordene primeiro por NOME (opcao 4 -> Bubble).\n");
                    break;
                }
                char chave[30];
                lerString("Componente-chave (nome): ", chave, sizeof(chave));
                long comps=0;
                int idx = buscaBinariaPorNome(base, n, chave, &comps);
                if(idx==-1){
                    printf("'%s' NAO encontrado. Comparacoes: %ld\n", chave, comps);
                } else {
                    printf("'%s' encontrado (pos %d). Comparacoes: %ld\n",
                           base[idx].nome, idx+1, comps);
                    printf(" - Tipo: %s | Prioridade: %d\n",
                           base[idx].tipo, base[idx].prioridade);
                }
                break;
            }
            case 0:
                printf("Ativando a torre de fuga... Boa sorte!\n");
                break;
            default:
                printf("Opcao invalida.\n");
        }
    }while(op!=0);

    return 0;
}