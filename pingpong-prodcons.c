// GRR20204069 Anderson Aparecido do Carmo Frasão

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ppos.h"
#include "queue.h"

int         item = 0;
int         buffer[5];
task_t      p1, p2, p3, c1, c2;
semaphore_t s_buffer, s_item, s_vaga ;

void produtor(void * arg)
{
   while (1)
   {
        task_sleep (1000);
        item = rand() % 99;

        sem_down (&s_vaga);

        sem_down (&s_buffer);
        for (int i = 0; i < 5; i++){
            if(buffer[i] == -1){
                buffer[i] = item;
                break;
            }
        }
        sem_up (&s_buffer);

        sem_up (&s_item);

        printf("%s%d\n", (char *) arg, item);
   }
}

void consumidor(void * arg)
{
   while (1)
   {
        sem_down (&s_item);

        sem_down (&s_buffer);
        int aux = buffer[0];
        for (int i = 0; i < 4; i++){
            buffer[i] = buffer[i+1];
        }
        buffer[4] = -1;
        printf("%s%d\n", (char *) arg, aux);
        sem_up (&s_buffer);

        sem_up (&s_vaga);

        task_sleep (1000);
   }
}

int main(int argc, char *argv[]){

    ppos_init () ;
    memset(buffer, -1, sizeof(buffer));

    sem_create (&s_buffer, 1) ;
    sem_create (&s_item, 0) ;
    sem_create (&s_vaga, 5) ;
    
    task_create(&p1, produtor, "p1 produziu ");
    task_create(&p2, produtor, "p2 produziu ");
    task_create(&p3, produtor, "p3 produziu ");
    task_create(&c1, consumidor, "                             c1 consumiu ");
    task_create(&c2, consumidor, "                             c2 consumiu ");

    task_join (&p1) ;
    task_join (&p2) ;
    task_join (&p3) ;
    task_join (&c1) ;
    task_join (&c2) ;

    sem_destroy (&s_buffer) ;
    sem_destroy (&s_item) ;
    sem_destroy (&s_vaga) ;

    task_exit (0) ;
    exit(0);
}