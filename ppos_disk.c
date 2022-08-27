// GRR 20204069 Anderson Aparecido do Carmo Frasão

#include "ppos_disk.h"

// inicializacao do gerente de disco
// retorna -1 em erro ou 0 em sucesso
// numBlocks: tamanho do disco, em blocos
// blockSize: tamanho de cada bloco do disco, em bytes
int disk_mgr_init (int *numBlocks, int *blockSize){

}

// leitura de um bloco, do disco para o buffer
int disk_block_read (int block, void *buffer){
    // obtém o semáforo de acesso ao disco
 
    // inclui o pedido na fila_disco
 
    if (gerente de disco está dormindo)
    {
        // acorda o gerente de disco (põe ele na fila de prontas)
    }
    
    // libera semáforo de acesso ao disco
    
    // suspende a tarefa corrente (retorna ao dispatcher)
}

// escrita de um bloco, do buffer para o disco
int disk_block_write (int block, void *buffer){
    // obtém o semáforo de acesso ao disco
 
    // inclui o pedido na fila_disco
 
    if (gerente de disco está dormindo)
    {
        // acorda o gerente de disco (põe ele na fila de prontas)
    }
    
    // libera semáforo de acesso ao disco
    
    // suspende a tarefa corrente (retorna ao dispatcher)
}