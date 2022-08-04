// GRR 20204069 Anderson Aparecido do Carmo Frasão

#include "queue.h"
#include<stdio.h>

//------------------------------------------------------------------------------
// Conta o numero de elementos na fila
// Retorno: numero de elementos na fila

int queue_size (queue_t *queue){

    if (queue == NULL){
        return 0;
    }

    queue_t *aux1 = queue;
    queue_t *aux2 = queue->next;
    int i = 1;

    // percorrendo toda a fila ate o ultimo elemento acrescendo i de 1 em 1
    while (aux2 != queue){
        aux1 = aux2;
        aux2 = aux1->next;
        i++;
    }

    return i;
}

//------------------------------------------------------------------------------
// Percorre a fila e imprime na tela seu conteúdo. A impressão de cada
// elemento é feita por uma função externa, definida pelo programa que
// usa a biblioteca. Essa função deve ter o seguinte protótipo:
//
// void print_elem (void *ptr) ; // ptr aponta para o elemento a imprimir

void queue_print (char *name, queue_t *queue, void print_elem (void*) ){

    printf("%s: [", name);

    // caso a lista esteja vazia
    if(queue == NULL){
        printf("]\n");
        return;
    }

    queue_t *aux1 = queue;
    queue_t *aux2 = queue->next;

    // percorrendo a lista ate a ultima posição
    while (aux2 != queue){
        print_elem(aux1);
        aux1 = aux2;
        aux2 = aux1->next;
        printf(" ");
    }

    // ultimo elemento
    print_elem(aux1);
    printf("]\n");
    return;
}

//------------------------------------------------------------------------------
// Insere um elemento no final da fila.
// Condicoes a verificar, gerando msgs de erro:
// - a fila deve existir
// - o elemento deve existir
// - o elemento nao deve estar em outra fila
// Retorno: 0 se sucesso, <0 se ocorreu algum erro

int queue_append (queue_t **queue, queue_t *elem){

    // testando condições de verificação
    if (queue == NULL || elem == NULL){
        return -1;
    }
    if (elem->next != NULL || elem->prev != NULL){
        return -1;
    }

    // caso a lista seja vazia
    if(*queue == NULL){
        *queue = elem;
        (*queue)->next = elem;
        (*queue)->prev = elem;
        return 0;
    }

    queue_t *aux1 = *queue;
    queue_t *aux2 = (*queue)->next;

    // percorrendo a lista ate a ultima posição
    while (aux2 != *queue){
        aux1 = aux2;
        aux2 = aux1->next;
    }

    // inserindo elemento
    aux1->next = elem;
    elem->prev = aux1;
    elem->next = *queue;
    (*queue)->prev = elem;;

    return 0;
}

//------------------------------------------------------------------------------
// Remove o elemento indicado da fila, sem o destruir.
// Condicoes a verificar, gerando msgs de erro:
// - a fila deve existir
// - a fila nao deve estar vazia
// - o elemento deve existir
// - o elemento deve pertencer a fila indicada
// Retorno: 0 se sucesso, <0 se ocorreu algum erro

int queue_remove (queue_t **queue, queue_t *elem){

    // condições de verificação
    if (queue == NULL || *queue == NULL || elem == NULL){
        return -1;
    }

    // caso a fila tenha somente 1 elemento e esse elemento seja da fila
    if ((*queue)->next == *queue && (*queue)->prev == *queue && elem == *queue){
        *queue = NULL;
        elem->next = NULL;
        elem->prev = NULL;
        elem = NULL;
        return 0;
    }

    queue_t *aux1 = *queue;
    queue_t *aux2 = (*queue)->next;

    // caso o elemento esteja em primeiro
    if (*queue == elem){
        
        // remove o elemento
        aux2 = (*queue)->prev;
        (*queue) = (*queue)->next;
        (*queue)->prev = aux2;
        aux2->next = (*queue);

        elem->next = NULL;
        elem->prev = NULL;
        elem = NULL;
        return 0;
    }

    // percorrendo a lista ate o ultimo elemento 
    // ou ate encontrar o elemento a ser removido
    while (aux2 != *queue){
        if(aux1 == elem){
            break;
        }
        aux1 = aux2;
        aux2 = aux1->next;
    }

    // caso o elemento não seja da lista
    if (aux1 != elem){
        return -1;
    }
    
    // remove o elemento
    aux1 = aux1->prev;
    aux1->next = aux2;
    aux2->prev = aux1;

    elem->next = NULL;
    elem->prev = NULL;
    elem = NULL;
    return 0;
}