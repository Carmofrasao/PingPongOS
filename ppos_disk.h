// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.4 -- Janeiro de 2022

// interface do gerente de disco rígido (block device driver)

#ifndef __DISK_MGR__
#define __DISK_MGR__

#include "ppos_data.h"

// estruturas de dados e rotinas de inicializacao e acesso
// a um dispositivo de entrada/saida orientado a blocos,
// tipicamente um disco rigido.

typedef struct pedido
{
  struct pedido *prev, *next ;    // Ponteiros para usar em filas
  int id;                         // tarefa que fez o pedido
  int type;                       // tipo de pedido
  int block;                      // Bloco para leitura/escrita da tarefa (caso seja operação de disco)
  void *buffer;                   // Endereço do buffer de dados
} pedido;


// estrutura que representa um disco no sistema operacional
typedef struct
{
  semaphore_t sem_disk;       // Semaforo do disco
  pedido *fila_disco;         // Tarefas de disco
  task_t *Dormitorio_Disk;    // Tarefas suspensas pelo disco
  short sinal;                // sinal de ativação do disco
  // completar com os campos necessarios
} disk_t ;

disk_t Disk;                  // Disco

// inicializacao do gerente de disco
// retorna -1 em erro ou 0 em sucesso
// numBlocks: tamanho do disco, em blocos
// blockSize: tamanho de cada bloco do disco, em bytes
int disk_mgr_init (int *numBlocks, int *blockSize) ;

// leitura de um bloco, do disco para o buffer
int disk_block_read (int block, void *buffer) ;

// escrita de um bloco, do buffer para o disco
int disk_block_write (int block, void *buffer) ;

#endif
