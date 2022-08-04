// GRR20204069 Anderson Aparecido do Carmo Frasão

#include "ppos.h"
#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include <signal.h>
#include <sys/time.h>

// estrutura que define um tratador de sinal (deve ser global ou static)
struct sigaction action ;

// estrutura de inicialização to timer
struct itimerval timer;

// possiveis estados de cada tarefa
enum estados {
    TERMINADA = 0,
    PRONTA,  
    SUSPENSA 
} status;

#define STACKSIZE 64*1024   // Tamanho da stack de cada tarefa
int ContadorDeTarefas;      // Marcador para definir os id's das tarefas
task_t *tarefaAtual;        // Tarefa que esta no processador no momento
task_t ContextMain;         // Tarefa da main
task_t ContextDispatcher;   // Tarefa do dispatcher
task_t *TarefasProntas;     // Vertor de tarefas prontas
int userTasks;              // Numero de tarefas de usuario
short quantum ;             // Tempo de vida da tarefa
unsigned int time ;         // Tempo do sistema

// libera o processador para a próxima tarefa, retornando à fila de tarefas
// prontas ("ready queue")
void task_yield (){
    task_switch(&ContextDispatcher);
}

// tratador do sinal
void tratador (int signum){
    
    if(tarefaAtual->TaskUser == 1){
        if (quantum > 0){
            time++;
            quantum--;
            return;
        }
        else if (quantum == 0)
            task_yield();
    }
}

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

// define a tarefa mais prioritaria
task_t* scheduler(){
    task_t* aux = TarefasProntas;
    task_t* aux2 = aux->next;
    // percorre todas as tarefas
    while (aux2 != TarefasProntas){
        if (aux2->prio_d < aux->prio_d){
            aux->prio_d--;
            aux = aux2;
        }
        else
            aux2->prio_d--;
        
        aux2 = aux2->next;
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
            quantum = 20;
            int t1 = systime();
            // transfere controle para a próxima tarefa
            task_switch (proxima);
            int t2 = systime();
            proxima->time_exec += (t2 - t1);
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
    time = 0;
    ContadorDeTarefas = 0;
    userTasks = 0;
    
    ContextMain.id = ContadorDeTarefas;
    ContextMain.prev = NULL;
    ContextMain.next = NULL;
    ContextMain.TaskUser = 0;
    tarefaAtual = &ContextMain;
    
    TarefasProntas = NULL;
    task_create(&ContextDispatcher, dispatcher, NULL);
    queue_remove((queue_t **)&TarefasProntas, (queue_t *)&ContextDispatcher);
    userTasks--;
    ContextDispatcher.TaskUser = 0;

    // registra a ação para o sinal de timer SIGALRM
    action.sa_handler = tratador ;
    sigemptyset (&action.sa_mask) ;
    action.sa_flags = 0 ;
    if (sigaction (SIGALRM, &action, 0) < 0)
    {
        perror ("Erro em sigaction: ") ;
        exit (1) ;
    }

    // ajusta valores do temporizador
    timer.it_value.tv_usec = 1000 ;      // primeiro disparo, em micro-segundos
    timer.it_value.tv_sec  = 0 ;      // primeiro disparo, em segundos
    timer.it_interval.tv_usec = 1000 ;   // disparos subsequentes, em micro-segundos
    timer.it_interval.tv_sec  = 0 ;   // disparos subsequentes, em segundos

    // arma o temporizador ITIMER_REAL (vide man setitimer)
    if (setitimer (ITIMER_REAL, &timer, 0) < 0)
    {
        perror ("Erro em setitimer: ") ;
        exit (1) ;
    }
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

    ContadorDeTarefas++;
    task->id = ContadorDeTarefas;

    task->next = NULL;
    task->prev = NULL;
    task->preemptable = 1;
    task->prio_e = 0;
    task->prio_d = task->prio_e;

    makecontext (&task->context, (void*)(*start_func), 1, arg) ;

    task->status = PRONTA;

    queue_append((queue_t **)&TarefasProntas, (queue_t *)task);
    userTasks++;

    task->TaskUser = 1;

    task->time_init = systime();
    task->time_exec = 0;
    task->n_ativa = 0;

    return task->id;
}

// Termina a tarefa corrente, indicando um valor de status encerramento
void task_exit (int exit_code){
    tarefaAtual->time_exit = systime();
    printf("Task %d exit: execution time %d ms, processor time %d ms, %d activations\n", tarefaAtual->id, tarefaAtual->time_exit-tarefaAtual->time_init, tarefaAtual->time_exec, tarefaAtual->n_ativa);
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
    task->n_ativa++;
    task_t * aux = tarefaAtual;
    tarefaAtual = task;
    swapcontext(&aux->context, &task->context);
    return 0;
}

// retorna o identificador da tarefa corrente (main deve ser 0)
int task_id (){
    return tarefaAtual->id;
}

// retorna o relógio atual (em milisegundos)
unsigned int systime (){
    return time;
}