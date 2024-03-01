//
// Created by linux-lex on 29/02/24.
//
#include "servidor.h"
#include "structures.h"
#include <pthread.h>
#include <mqueue.h>
#include <stdbool.h>

pthread_mutex_t sync_mutex;
bool sync_copied = FALSE;
pthread_cond_t sync_cond;



int main ( int argc, char *argv[] )
{
    struct peticion p;
    struct respuesta r;
    int prio;
    int contador = 0;
    pthread_attr_t attr;
    pthread_t thid[MAX];
    pthread_mutex_init(&sync_mutex, NULL);
    pthread_cond_init(&sync_cond, NULL);
    pthread_attr_init(&attr) ;
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) ;
    int qs = mq_open("/SERVIDOR", O_CREAT | O_RDONLY, 0700, NULL) ;
    if (qs == -1) { return -1 ; }
    while (1) {
        mq_recv(qs, &p, sizeof(struct peticion), &prio) ;
        pthread_create(&thid[contador], &attr, tratar_petición, (void *)&p) ;
        pthread_mutex_lock(&sync_mutex) ;
        while (sync_copied == FALSE) {
            pthread_cond_wait(&sync_cond, &sync_mutex) ;
        }
        sync_copied = FALSE ;
        pthread_mutex_unlock(&sync_mutex) ;
        contador++;
    }
}

void tratar_petición ( struct peticion * p )
{
    struct peticion p_local ;
    pthread_mutex_lock(&sync_mutex) ;
    p_local = *p ;
    sync_copied = TRUE ;
    pthread_cond_signal(&sync_cond) ;
    pthread_mutex_unlock(&sync_mutex) ;
    switch (p.op)
    {
        case 0: r.status = init() ;
            break;
        case 1: r.status = get_value() ;
            break;
        case 2: r.status = set_value() ;
            break;
        case 3: r.status = modify_value() ;
            break;
        case 4: r.status = delete_key() ;
            break;
        case 5: r.status = exist() ;
            break;
    }
    int qs = mq_open(p->q_name, O_CREAT|O_WRONLY, 0700, NULL) ;
    mq_send(qr, &r, sizeof(struct respuesta), prio) ;
    mq_close(qr);
    pthread_exit(NULL) ;
}