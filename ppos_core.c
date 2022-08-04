// GRR20204069 Anderson Aparecido do Carmo Frasão

#include "ppos.h"
#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include <string.h>

enum estados {
    TERMINADA = 0,
    PRONTA,  
    SUSPENSA 
} status;

#define STACKSIZE 64*1024
int cont;
task_t *tarefaAtual;
task_t ContextMain;
task_t ContextDispatcher;
task_t *TarefasProntas;
int userTasks;

void task_setprio (task_t *task, int prio){
    if (task == NULL){
        tarefaAtual->prio_e = prio;
        tarefaAtual->prio_d = tarefaAtual->prio_e;
        return;
    }
    task->prio_e = prio;
    task->prio_d = task->prio_e;
}

int task_getprio (task_t *task){
    if (task == NULL){
        return tarefaAtual->prio_e;
    }
    return task->prio_e;
}

task_t* scheduler(){
    task_t* aux = TarefasProntas;
    task_t* aux2 = aux->next;
    int tam_fila = queue_size((queue_t*)TarefasProntas);
    short verificador[tam_fila];
    memset(verificador, 0, tam_fila*sizeof(short));
    for(int i = 0; i < tam_fila; i++){
        for(int l = 0; l < tam_fila; l++){
            if (aux2->prio_d < aux->prio_d)
                aux = aux2;
            else if (verificador[l] == 0){
                verificador[l] = 1;
                aux2->prio_d--;
            }

            aux2 = aux2->next;
        }
    }
    
    aux->prio_d = aux->prio_e;
    return aux;
}

void dispatcher(){
    // enquanto houverem tarefas de usuário
    while(userTasks > 0){
        // escolhe a próxima tarefa a executar
        task_t *proxima = scheduler();
        
        // escalonador escolheu uma tarefa?      
        if(proxima != NULL){

            // transfere controle para a próxima tarefa
            task_switch (proxima);
            
            // voltando ao dispatcher, trata a tarefa de acordo com seu estado
            // caso o estado da tarefa "próxima" seja:
            switch ( proxima->status ){
                case PRONTA: 

                    break;
                case TERMINADA:
                    userTasks--;
                    free((*proxima).context.uc_stack.ss_sp);
                    queue_remove((queue_t **)&TarefasProntas, (queue_t *)proxima);
                    break; 
                case SUSPENSA:

                    break;
            }      
        }
    }

    // encerra a tarefa dispatcher
    task_exit(0);
}

// Inicializa o sistema operacional; deve ser chamada no inicio do main()
void ppos_init (){
    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf (stdout, 0, _IONBF, 0);
    
    cont = 0;
    userTasks = 0;
    
    ContextMain.id = cont;
    ContextMain.prev = NULL;
    ContextMain.next = NULL;
    tarefaAtual = &ContextMain;
    
    TarefasProntas = NULL;
    task_create(&ContextDispatcher, dispatcher, NULL);
    queue_remove((queue_t **)&TarefasProntas, (queue_t *)&ContextDispatcher);
    userTasks--;
}

// Cria uma nova tarefa. Retorna um ID> 0 ou erro.
int task_create (task_t *task,		    // descritor da nova tarefa
                 void (*start_func)(void *),	// funcao corpo da tarefa
                 void *arg){         	// argumentos para a tarefa
    getcontext (&task->context);
    char *stack;

    stack = malloc (STACKSIZE);

    if (stack){
        (*task).context.uc_stack.ss_sp = stack ;
        (*task).context.uc_stack.ss_size = STACKSIZE ;
        (*task).context.uc_stack.ss_flags = 0 ;
        (*task).context.uc_link = 0 ;
    }
    else{
        perror ("Erro na criação da pilha: ") ;
        return -1;
    }

    cont++;
    task->id = cont;

    task->next = NULL;
    task->prev = NULL;
    task->preemptable = 1;
    task->prio_e = 0;
    task->prio_d = task->prio_e;

    makecontext (&task->context, (void*)(*start_func), 1, arg) ;

    task->status = PRONTA;

    queue_append((queue_t **)&TarefasProntas, (queue_t *)task);
    userTasks++;
    return task->id;
}

// Termina a tarefa corrente, indicando um valor de status encerramento
void task_exit (int exit_code){
    tarefaAtual->status = TERMINADA;
    if(tarefaAtual != &ContextDispatcher)
        task_switch(&ContextDispatcher);
    else{
        free((*tarefaAtual).context.uc_stack.ss_sp);
        task_switch(&ContextMain);
    }
}

// alterna a execução para a tarefa indicada
int task_switch (task_t *task){
    task_t * aux = tarefaAtual;
    tarefaAtual = task;
    swapcontext(&aux->context, &task->context);
    return 0;
}

// retorna o identificador da tarefa corrente (main deve ser 0)
int task_id (){
    return tarefaAtual->id;
}

// libera o processador para a próxima tarefa, retornando à fila de tarefas
// prontas ("ready queue")
void task_yield (){
    task_switch(&ContextDispatcher);
}