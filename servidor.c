//
// Created by linux-lex on 29/02/24.
//
#include "servidor.h"
#include "structures.h"
#include <pthread.h>
#include <mqueue.h>
#include <stdbool.h>
#include <stdio.h>

pthread_mutex_t sync_mutex;
bool sync_copied = false;
pthread_cond_t sync_cond;

int main ( int argc, char *argv[] )
{
    // inicializar estructuras de peticion y variables
    struct peticion p;
    struct respuesta r;
    int prio;
    int contador = 0;
    // inicializar hilos
    pthread_attr_t attr;
    pthread_t thid[MAX];
    // inicializar mutex y varaibles de condicion
    pthread_mutex_init(&sync_mutex, NULL);
    pthread_cond_init(&sync_cond, NULL);
    pthread_attr_init(&attr) ;
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    // abrir cola de lectura
    struct mq_attr attr_read; 
    // modificar atributos cola
    mqd_t queue_read = mq_open("/SERVIDOR", O_CREAT | O_RDONLY, 0700, &attr_read);
    if (-1 == queue_read){
        fprintf(stderr, "Error al crear la cola de mensajes.\n");
        return -1;
    }
    // inicializar mutex
    while (1) {
        mq_recv(queue_read, &p, sizeof(struct peticion), &prio) ;
        // hay que pasarle un struct con todos los valores que se vayan a usar
        // &p esta mal     
        pthread_create(&thid[contador], &attr, tratar_petición, (void *)&p);
        pthread_mutex_lock(&sync_mutex) ;
        while (sync_copied == false) {
            pthread_cond_wait(&sync_cond, &sync_mutex) ;
        }
        sync_copied = false;
        pthread_mutex_unlock(&sync_mutex) ;
        contador++;
    }
}

void tratar_petición ( struct peticion * p )
{
    struct peticion p_local ;
    pthread_mutex_lock(&sync_mutex) ;
    p_local = *p ;
    sync_copied = true ;
    pthread_cond_signal(&sync_cond) ;
    pthread_mutex_unlock(&sync_mutex) ;
    switch (p_local.op)
    {
        case 0: r.status = s_init();
            break;
        case 1: r.status = s_get_value();
            break;
        case 2: r.status = s_set_value();
            break;
        case 3: r.status = s_modify_value();
            break;
        case 4: r.status = s_delete_key();
            break;
        case 5: r.status = s_exist();
            break;
    }
    mqd_t queue_read = mq_open(p->q_name, O_CREAT | O_WRONLY, 0700, NULL);
    mq_send(queue_read, &r, sizeof(struct respuesta), prio) ;
    mq_close(queue_read);
    pthread_exit(NULL) ;
}