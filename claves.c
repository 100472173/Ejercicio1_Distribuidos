//
// Created by linux-lex on 29/02/24.
//

#include "claves.h"
#include <unistd.h>
#include "structures.h"
#include <stdio.h>
#include <mqueue.h>

mqd_t queue_servidor, queue_cliente;
struct mq_attr attr_servidor, attr_cliente;

int init(){
    // crear peticion
    struct peticion pet = {.op = 0};
    // mandar peticion
    int err = open_server();
    if (-1 == err){
        fprintf(stderr, "Error al abrir la cola de mensajes.\n");
        return -1;
    }
    
}

int set_value(int key, char *value1, int N_value2, double *V_value2){

}

int get_value(int key, char *value1, int *N_value2, double *V_value2){

}

int modify_value(int key, char *value1, int N_value2, double *V_value2){

}
int delete_key(int key){

}

int exist(int key){

}

int open_client(){
    // crear nombre de la cola
    char queue_name_c[1024];
    sprintf(queue_name_c, "%s%d", "/CLIENTE_", getpid());
    // abrir cola cliente
    queue_cliente = mq_open(queue_name_c, O_CREAT | O_RDONLY, 0700, &attr_cliente);
    return queue_cliente;
}

int open_server(){
    // crear nombre cola
    const static char queue_name_s[1024] = "/SERVIDOR";
    // abrir cola servidor
    queue_servidor = mq_open(queue_name_s, O_WRONLY, 0700, &attr_servidor);
    return queue_servidor;
}