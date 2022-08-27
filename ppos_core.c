// GRR20204069 Anderson Aparecido do Carmo Frasão

#include "ppos.h"
#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
task_t ContextDrive;        // Tarefa do disco
task_t *TarefasProntas;     // Tarefas prontas
task_t *Dormitorio;         // Tarefas suspensas
task_t *TarefasDeDisco;     // Tarefas de disco
int userTasks;              // Numero de tarefas de usuario
short quantum ;             // Tempo de vida da tarefa
unsigned int time ;         // Tempo do sistema

int lock_u = 0 ;
int lock_d = 0 ;

// tratador do sinal do disco
void tratador_disk (int signum){
    task_switch(&ContextDrive);
}

void diskDriverBody (void * args)
{
   while (1) 
   {
      // obtém o semáforo de acesso ao disco
 
      // se foi acordado devido a um sinal do disco
      if (disco gerou um sinal)
      {
         // acorda a tarefa cujo pedido foi atendido
      }
 
      // se o disco estiver livre e houver pedidos de E/S na fila
      if (disco_livre && (fila_disco != NULL))
      {
         // escolhe na fila o pedido a ser atendido, usando FCFS
         // solicita ao disco a operação de E/S, usando disk_cmd()
      }
 
      // libera o semáforo de acesso ao disco
 
      // suspende a tarefa corrente (retorna ao dispatcher)
   }
}
 
void enter_cs (int *lock)
{
  // atomic OR (Intel macro for GCC)
  while (__sync_fetch_and_or (lock, 1)) ;   // busy waiting
}
 
void leave_cs (int *lock)
{
  (*lock) = 0 ;
}

// define a tarefa mais prioritaria
task_t* scheduler(){
    task_t* aux = TarefasProntas;
    if(aux != NULL){
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
    }
    return aux;
}

void dispatcher(){
    int td_1 = systime();
    // enquanto houverem tarefas de usuário
    while(userTasks > 0){
        // escolhe a próxima tarefa a executar
        task_t *proxima = scheduler();
        
        // acordando tarefas em sleep
        task_t *aux = Dormitorio;
        if(aux != NULL){
            task_t *aux1 = Dormitorio->next;
            while (aux1 != Dormitorio){
                if(systime() >= aux->sleep_time){
                    task_resume(aux, &Dormitorio);
                }
                aux = aux1;
                aux1 = aux->next;
            };
            if(systime() >= aux->sleep_time){
                task_resume(aux, &Dormitorio);
            }
        }

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
    int td_2 = systime();
    ContextDispatcher.time_exec += (td_2 - td_1);

    // encerra a tarefa dispatcher
    task_exit(0);
}

// tratador do sinal
void tratador (int signum){
    time++;
    if(tarefaAtual->TaskUser == 1){
        if (quantum > 0){
            quantum--;
            return;
        }
        else if (quantum == 0)
            task_yield();
    }
}

// Inicializa o sistema operacional; deve ser chamada no inicio do main()
void ppos_init (){
    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf (stdout, 0, _IONBF, 0);
    time = 0;
    ContadorDeTarefas = 0;
    userTasks = 0;
    TarefasProntas = NULL;
    
    task_create(&ContextMain, NULL, NULL);

    tarefaAtual = &ContextMain;

    task_create(&ContextDispatcher, dispatcher, NULL);
    queue_remove((queue_t **)&TarefasProntas, (queue_t *)&ContextDispatcher);
    userTasks--;
    ContextDispatcher.TaskUser = 0;

    task_create(&ContextDrive, diskDriverBody, NULL);
    queue_remove((queue_t **)&TarefasProntas, (queue_t *)&ContextDrive);
    userTasks--;
    ContextDrive.TaskUser = 0;

    // registra a ação para o sinal de timer SIGALRM
    action.sa_handler = tratador ;
    sigemptyset (&action.sa_mask) ;
    action.sa_flags = 0 ;
    if (sigaction (SIGALRM, &action, 0) < 0)
    {
        perror ("Erro em sigaction: ") ;
        exit (1) ;
    }

    // registra a ação para o sinal de timer SIGUSR1
    action.sa_handler = tratador_disk ;
    sigemptyset (&action.sa_mask) ;
    action.sa_flags = 0 ;
    if (sigaction (SIGUSR1, &action, 0) < 0)
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
    task_yield ();
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

    task->id = ContadorDeTarefas;
    ContadorDeTarefas++;

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
    task->tarefas_suspensas = NULL;

    return task->id;
}

// Termina a tarefa corrente, indicando um valor de status encerramento
void task_exit (int exit_code){
    tarefaAtual->time_exit = systime();
    printf("Task %d exit: execution time %d ms, processor time %d ms, %d activations\n", tarefaAtual->id, tarefaAtual->time_exit-tarefaAtual->time_init, tarefaAtual->time_exec, tarefaAtual->n_ativa);
    tarefaAtual->status = TERMINADA;
    task_t *aux = tarefaAtual->tarefas_suspensas;
    if(aux != NULL){
        do{
            task_resume(aux, &tarefaAtual->tarefas_suspensas);
            aux->ec = exit_code;
            aux = aux->next;
        }while (tarefaAtual->tarefas_suspensas != NULL);
    } 

    if(tarefaAtual != &ContextDispatcher)
        task_switch(&ContextDispatcher);
    else{
        free((*tarefaAtual).context.uc_stack.ss_sp);
        exit(0);
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

// suspende a tarefa atual na fila "queue"
void task_suspend (task_t **queue){
    queue_remove((queue_t **)&TarefasProntas, (queue_t *)tarefaAtual);
    tarefaAtual->status = SUSPENSA;
    queue_append((queue_t **)queue, (queue_t *)tarefaAtual);
    task_yield();
}

// acorda a tarefa indicada, que está suspensa na fila indicada
void task_resume (task_t *task, task_t **queue){
    queue_remove((queue_t **)queue, (queue_t *)task);
    task->status = PRONTA;
    queue_append((queue_t **)&TarefasProntas, (queue_t *)task);
}

// libera o processador para a próxima tarefa, retornando à fila de tarefas
// prontas ("ready queue")
void task_yield (){
    task_switch(&ContextDispatcher);
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

int task_join (task_t *task){
    if(task->status == TERMINADA || task == NULL){
        return -1;
    }
    task_suspend(&task->tarefas_suspensas);
    return tarefaAtual->ec;
}

// suspende a tarefa corrente por t milissegundos
void task_sleep (int t){
    tarefaAtual->sleep_time = systime() + t;
    task_suspend(&Dormitorio);
}

// retorna o relógio atual (em milisegundos)
unsigned int systime (){
    return time;
}

// cria um semáforo com valor inicial "value"
int sem_create (semaphore_t *s, int value){
    s->counter = value;
    s->queue = NULL;
    if (s == NULL){
        return -1;
    }
    return 0;
}

// requisita o semáforo
int sem_down (semaphore_t *s){
    enter_cs (&lock_d) ;
    if(s == NULL){
        leave_cs (&lock_d) ;
        return -1;
    }
    s->counter--;
    if(s->counter < 0){
        leave_cs (&lock_d) ;
        task_suspend(&s->queue);
        return 0;
    }
    leave_cs (&lock_d) ;
    return 0;
}

// libera o semáforo
int sem_up (semaphore_t *s){
    enter_cs (&lock_u) ;
    if(s == NULL){
        leave_cs (&lock_u) ;
        return -1;
    }
    
    s->counter++;
    if(s->counter <= 0){
        task_resume(s->queue, &s->queue);
        leave_cs (&lock_u) ;
        return 0;
    }
    leave_cs (&lock_u) ;
    return -1;
}

// destroi o semáforo, liberando as tarefas bloqueadas
int sem_destroy (semaphore_t *s){
    if(s == NULL){
        return -1;
    }
    task_t *aux = s->queue;
    if(aux != NULL){
        do{
            sem_up(s);
            aux = aux->next;
        }while (s->queue != NULL);
        s->queue = NULL;
        s = NULL;
    } 
    return 0;
}

// cria uma fila para até max mensagens de size bytes cada
int mqueue_create (mqueue_t *queue, int max, int size){
    
    queue->buffer = malloc(max * size);
    if (!queue->buffer)
        return -1;

    queue->buffer_fim = (char*)queue->buffer + max * size;
    queue->max = max;
    queue->count = 0;
    queue->size = size;
    queue->inicio = queue->buffer;
    queue->fim = queue->buffer;

    if(sem_create (&queue->s_buffer, 1) == -1 || sem_create (&queue->s_item, 0) == -1 || sem_create (&queue->s_vaga, max) == -1)
        return -1;

    return 0;
}

// envia uma mensagem para a fila
int mqueue_send (mqueue_t *queue, void *msg){
    if(queue == NULL || !queue->buffer || queue->fim == NULL || queue->buffer_fim == NULL){
        sem_up (&queue->s_buffer);
        sem_up (&queue->s_vaga);
        return -1;
    }

    sem_down (&queue->s_vaga);

    sem_down (&queue->s_buffer);
    
    memcpy(queue->inicio, msg, queue->size);
    queue->inicio = (char*)queue->inicio + queue->size;
    if(queue->inicio == queue->buffer_fim)
        queue->inicio = queue->buffer;
    queue->count++;
    
    sem_up (&queue->s_buffer);

    sem_up (&queue->s_item);
    return 0;
}

// recebe uma mensagem da fila
int mqueue_recv (mqueue_t *queue, void *msg){
    
    sem_down (&queue->s_item);

    sem_down (&queue->s_buffer);
    if(queue == NULL || !queue->buffer || queue->fim == NULL || queue->buffer_fim == NULL){
        sem_up (&queue->s_buffer);
        sem_up (&queue->s_item);
        return -1;
    }
    memcpy(msg, queue->fim, queue->size);
    queue->fim = (char*)queue->fim + queue->size;
    if(queue->fim == queue->buffer_fim)
        queue->fim = queue->buffer;
    queue->count--;
    sem_up (&queue->s_buffer);

    sem_up (&queue->s_vaga);
    return 0;
}

// destroi a fila, liberando as tarefas bloqueadas
int mqueue_destroy (mqueue_t *queue){
    queue->buffer = NULL;
    free(queue->buffer);

    queue->buffer_fim = NULL;
    queue->inicio = NULL;
    queue->fim = NULL;

    sem_destroy (&queue->s_buffer) ;
    sem_destroy (&queue->s_item) ;
    sem_destroy (&queue->s_vaga) ;

    queue = NULL;
    return 0;
}

// informa o número de mensagens atualmente na fila
int mqueue_msgs (mqueue_t *queue){
    return queue->count;
}