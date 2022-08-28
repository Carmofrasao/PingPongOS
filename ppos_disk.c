// GRR 20204069 Anderson Aparecido do Carmo Frasão

#include "ppos_disk.h"
#include "ppos_data.h"
#include "ppos.h"
#include "disk.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

// inicializacao do gerente de disco
// retorna -1 em erro ou 0 em sucesso
// numBlocks: tamanho do disco, em blocos
// blockSize: tamanho de cada bloco do disco, em bytes
int disk_mgr_init (int *numBlocks, int *blockSize){
    if(disk_cmd(DISK_CMD_INIT, 0, 0) < 0)
        return -1;

    *numBlocks = disk_cmd (DISK_CMD_DISKSIZE, 0, 0);
    if(*numBlocks < 0)
        return -1;

    *blockSize = disk_cmd (DISK_CMD_BLOCKSIZE, 0, 0);
    if(*blockSize < 0)
        return -1;

    return 0;
}

// leitura de um bloco, do disco para o buffer
int disk_block_read (int block, void *buffer){
    if(buffer == NULL)
        return -1;
    // obtém o semáforo de acesso ao disco
    sem_down(&Disk.sem_disk);

    pedido *aux = malloc(sizeof(pedido));
    aux->prev = NULL; 
    aux->next = NULL;
    aux->id = tarefaAtual->id;
    aux->block = block;
    aux->buffer = buffer;
    aux->type = DISK_CMD_READ;

    // inclui o pedido na fila_disco
    if(queue_append((queue_t **)&Disk.fila_disco, (queue_t *)aux) < 0){
        return -1;
    }
    
    if (ContextDrive.status == SUSPENSA)
    {
        // acorda o gerente de disco (põe ele na fila de prontas)
        task_resume(&ContextDrive, &Dormitorio);
    }
    
    // libera semáforo de acesso ao disco
    sem_up(&Disk.sem_disk);
    
    // suspende a tarefa corrente (retorna ao dispatcher)
    /*
        O ERRO ESTA AQUI (linha 64)
        A TAREFA QUE CHAMA disk_block_read NÃO ESTA SUSPENDENDO!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
    */
    task_suspend(&Disk.Dormitorio_Disk);
    return 0;
}

// escrita de um bloco, do buffer para o disco
int disk_block_write (int block, void *buffer){
    if(buffer == NULL)
        return -1;
    // obtém o semáforo de acesso ao disco
    sem_down(&Disk.sem_disk);

    pedido *aux = malloc(sizeof(pedido));
    aux->prev = NULL; 
    aux->next = NULL;
    aux->id = tarefaAtual->id;
    aux->block = block;
    aux->buffer = buffer;
    aux->type = DISK_CMD_WRITE;
 
    // inclui o pedido na fila_disco
    if(queue_append((queue_t **)&Disk.fila_disco, (queue_t *)aux) < 0){
        return -1;
    }
 
    if (ContextDrive.status == SUSPENSA)
    {
        // acorda o gerente de disco (põe ele na fila de prontas)
        task_resume(&ContextDrive, &Dormitorio);
    }
    
    // libera semáforo de acesso ao disco
    sem_up(&Disk.sem_disk);
    
    // suspende a tarefa corrente (retorna ao dispatcher)
    /*
        O ERRO ESTA AQUI (linha 101)
        A TAREFA QUE CHAMA disk_block_write NÃO ESTA SUSPENDENDO!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
    */
    task_suspend(&Disk.Dormitorio_Disk);

    return 0;
}