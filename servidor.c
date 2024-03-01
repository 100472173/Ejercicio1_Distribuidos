//
// Created by linux-lex on 29/02/24.
//
#include "servidor.h"
#include "structures.h"
#include <mqueue.h>
#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

pthread_mutex_t sync_mutex;
bool sync_copied = false;
pthread_cond_t sync_cond;
mqd_t queue_servidor;

int main ( int argc, char *argv[] )
{
    // Inicializamos peticion y variables
    struct peticion p;
    unsigned int prio;
    int contador = 0;
    // Inicializamos los atributos de la cola
    struct mq_attr attr_read;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(struct peticion);
    // Inicializamos los hilos
    pthread_attr_t attr;
    pthread_t thid[MAX];
    pthread_attr_init(&attr) ;
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    // Inicializamos mutex y variable condicion
    pthread_mutex_init(&sync_mutex, NULL);
    pthread_cond_init(&sync_cond, NULL);
    // Abrimos la cola para lectura
    queue_servidor = mq_open("/SERVIDOR", O_CREAT | O_RDONLY, 0700, &attr_read);
    if (queue_servidor == -1){
        fprintf(stderr, "Error al crear la cola de mensajes.\n");
        return -1;
    }
    while (1) {
        if (mq_receive(queue_servidor, (char *)&p, sizeof(struct peticion), &prio) < 0) {
            fprintf(stderr, "Error al recibir la petición de la cola de mensajes.\n");
            return -1;
        }
        if (pthread_create(&thid[contador], &attr, (void*)tratar_peticion, &p) != 0) {
            fprintf(stderr, "Error al crear el hilo.\n");
            return -1;
        }
        // Hacemos lock al mutex hasta que se copie la peticion en el hilo
        pthread_mutex_lock(&sync_mutex) ;
        while (sync_copied == false) {
            pthread_cond_wait(&sync_cond, &sync_mutex) ;
        }
        sync_copied = false;
        pthread_mutex_unlock(&sync_mutex) ;
        // Contador para los hilos
        contador++;
    }
    return 0;
}

void tratar_peticion (struct peticion* p)
{
    // Creamos la peticion local y la respuesta
    struct peticion p_local;
    struct respuesta resp;
    // Adquirimos el mutex para copiar la peticion pasada por parametro
    pthread_mutex_lock(&sync_mutex);
    p_local = *p;
    sync_copied = true;
    pthread_cond_signal(&sync_cond);
    pthread_mutex_unlock(&sync_mutex);
    // En funcion de la operacion especificada en la operacion hacemos una u otra operacion
    switch (p_local.op)
    {
        case 0: resp.status = s_init();
            break;
        case 1: resp.status = s_get_value();
            break;
        case 2: resp.status = s_set_value();
            break;
        case 3: resp.status = s_modify_value();
            break;
        case 4: resp.status = s_delete_key();
            break;
        case 5: resp.status = s_exist();
            break;
    }
    // Abrimos la cola del cliente
    mqd_t queue_cliente = mq_open(p->q_name, O_CREAT | O_WRONLY, 0700, NULL);
    if (queue_cliente == -1) {
        fprintf(stderr, "Error al abrir la cola de mensajes del cliente.\n");
        mq_close(queue_servidor);
        mq_unlink("/SERVIDOR");
    }
    else {
        // Mandamos el mensaje al cliente
        if (mq_send(queue_cliente, (char*)&resp, sizeof(struct respuesta), 0) < 0) {
            fprintf(stderr, "Error al enviar el mensaje al cliente.\n");
            mq_close(queue_servidor);
            mq_unlink("/SERVIDOR");
            mq_close(queue_cliente);
        }
    }
    pthread_exit(0) ;
}