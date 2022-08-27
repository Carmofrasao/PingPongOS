// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.4 -- Janeiro de 2022

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <ucontext.h>		// biblioteca POSIX de trocas de contexto

// Estrutura que define um Task Control Block (TCB)
typedef struct task_t
{
  struct task_t *prev, *next ;        // Ponteiros para usar em filas
  int id ;				                    // Identificador da tarefa
  ucontext_t context ;			          // Contexto armazenado da tarefa
  short status ;			                // Pronta, rodando, suspensa, ...
  short preemptable ;			            // Pode ser preemptada?
  short prio_e ;                      // Prioridade estatica da tarefa
  short prio_d ;                      // Prioridade dinamica da tarefa
  short TaskUser ;                    // Tarefa de usuario
  int time_init ;                     // Momento em que a tarefa entrou para a fila de tarefas prontas
  int time_exit ;                     // Momento em que a tarefa saiu da fila de tarefas prontas
  int time_exec ;                     // Tempo de execução da tarefa
  int n_ativa ;                       // Numero de ativações da tarefa
  struct task_t * tarefas_suspensas ; // Fila de tarefas suspensas
  int ec ;                            // Exit code
  int sleep_time;                     // Tempo do sleep
  int type;                           // tipo de pedido
  int block;                          // Bloco para leitura/escrita da tarefa (caso seja operação de disco)
  void *buffer;                       // Endereço do buffer de dados
  // ... (outros campos serão adicionados mais tarde)
} task_t ;

// estrutura que define um semáforo
typedef struct
{
  int counter ;                       // contador do semafaro
  task_t * queue ;                    // fila do semafaro
  // preencher quando necessário
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
  void *buffer;     // buffer de dados
  void *buffer_fim; // fim do buffer de dados
  size_t max;       // número máximo de itens no buffer
  size_t count;     // número de itens no buffer
  size_t size;      // tamanho de cada item no buffer
  void *inicio;     // ponteiro para o inicio
  void *fim;        // ponteiro para o fim
  semaphore_t s_buffer, s_item, s_vaga ;  // samafaro da fila
  // preencher quando necessário
} mqueue_t ;

#endif