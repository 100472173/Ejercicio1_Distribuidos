//
// Created by linux-lex on 29/02/24.
//

#include "claves.h"
#include <unistd.h>
#include "structures.h"
#include <stdio.h>
#include <mqueue.h>
#include <stdlib.h>
#include <string.h>

/*
// Puede haber problemas de concurrencia con mqd_t y struct mq_attr
mqd_t queue_servidor, queue_cliente;
struct mq_attr attr_servidor, attr_cliente;
// nombre peticion
char queue_name_c[MAX];
// valores por defecto de todas las peticiones.
size_t size_pet = sizeof(struct peticion);
unsigned int prio_p = 0;
size_t size_resp = sizeof(struct respuesta);
unsigned int prio_r = 0;
*/

int init(){
    struct peticion p;
    struct respuesta r;
    mqd_t queue_servidor;
    mqd_t queue_cliente;
    char client_name[MAX];
    // Diria que no hace falta hacer queue attr, es medio lioso ahora mismo, no entiendo muy bien

    queue_servidor = mq_open("/SERVIDOR", O_CREAT|O_WRONLY, 0700, NULL) ;
    if (queue_servidor == -1) {
        fprintf(stderr, "Error al abrir la cola del servidor en el cliente.\n");
        return -1;
    }

    sprintf(client_name, "%s%d", "/CLIENTE_", getpid()) ;
    queue_cliente = mq_open(client_name, O_CREAT|O_RDONLY) ;
    if (queue_cliente == -1) {
        fprintf(stderr, "Error al abrir la cola del cliente en el cliente.\n");
        mq_close(queue_servidor);
        return -1;
    }

    // Rellenar la peticion
    memset(&p, 0, sizeof(struct peticion));
    p.op = 0;
    strcpy(p.q_name, client_name);

    if (mq_send(queue_servidor, (const char *)&p, sizeof(struct peticion), 0) < 0){
        fprintf(stderr, "Error al enviar el mensaje al servidor.\n");
        perror("mq_send");
        return -1;
    }
    if (mq_receive(queue_cliente, (char *)&r, sizeof(struct respuesta), 0) < 0){
        fprintf(stderr, "Error al recibir el mensaje del servidor.\n");
        perror("mq_recv");
        return -1;
    }

    mq_close(queue_servidor);
    mq_close(queue_cliente);
    mq_unlink(client_name);
    // No sé si hay que hacer cast de r a struct respuesta
    return r.status;
    /*
    // abirir cola servidor
    int open_s = open_server();
    // crear peticion
    struct peticion pet = {.op = 0};
    strcpy(pet.q_name, queue_name_c);
    // mandar peticion
    int send = send_server((char *)&pet, size_pet, prio_p);
    // abrir cola cliente
    int open_c = open_client();
    // recibir respuesta
    struct respuesta res;
    int rec = receive_client((char*)&res, size_resp, &prio_r);
    int exit_value = check_errors(open_c, open_s, send, rec);
    return exit_value;
     */
}

int set_value(int key, char *value1, int N_value2, double *V_value2){
    struct peticion p;
    struct respuesta r;
    mqd_t queue_servidor;
    mqd_t queue_cliente;
    char client_name[MAX];
    // Diria que no hace falta hacer queue attr, es medio lioso ahora mismo, no entiendo muy bien

    queue_servidor = mq_open("/SERVIDOR", O_CREAT|O_WRONLY, 0700, NULL) ;
    if (queue_servidor == -1) {
        fprintf(stderr, "Error al abrir la cola del servidor en el cliente.\n");
        return -1;
    }

    sprintf(client_name, "%s%d", "/CLIENTE_", getpid()) ;
    queue_cliente = mq_open(client_name, O_CREAT|O_RDONLY) ;
    if (queue_cliente == -1) {
        fprintf(stderr, "Error al abrir la cola del cliente en el cliente.\n");
        mq_close(queue_servidor);
        return -1;
    }

    // Rellenar la peticion
    memset(&p, 0, sizeof(struct peticion));
    p.op = 1;
    strcpy(p.q_name, client_name);
    p.key = key;
    p.valor1 = value1;
    p.valor2_value = V_value2;
    p.valor2_N = N_value2;

    if (mq_send(queue_servidor, (const char *)&p, sizeof(struct peticion), 0) < 0){
        fprintf(stderr, "Error al enviar el mensaje al servidor.\n");
        perror("mq_send");
        return -1;
    }
    if (mq_receive(queue_cliente, (char *)&r, sizeof(struct respuesta), 0) < 0){
        fprintf(stderr, "Error al recibir el mensaje del servidor.\n");
        perror("mq_recv");
        return -1;
    }

    mq_close(queue_servidor);
    mq_close(queue_cliente);
    mq_unlink(client_name);
    // No sé si hay que hacer cast de r a struct respuesta
    return r.status;

    /*
    // abirir cola servidor
    int open_s = open_server();
    // crear peticion
    struct peticion pet = {.key = key,
                           .op = 1,
                           .valor1 = value1,
                           .valor2_N = N_value2,
                           .valor2_value = V_value2};
    strcpy(pet.q_name, queue_name_c);
    // mandar peticion
    int send = send_server((char *)&pet, size_pet, prio_p);
    // abrir cola cliente
    int open_c = open_client();
    // recibir respuesta
    struct respuesta res;
    int rec = receive_client((char *)&res, size_resp, &prio_r);
    int exit_value = check_errors(open_c, open_s, send, rec);
    return exit_value;
    */
}

int get_value(int key, char *value1, int *N_value2, double *V_value2){
    struct peticion p;
    struct respuesta r;
    mqd_t queue_servidor;
    mqd_t queue_cliente;
    char client_name[MAX];
    // Diria que no hace falta hacer queue attr, es medio lioso ahora mismo, no entiendo muy bien

    queue_servidor = mq_open("/SERVIDOR", O_CREAT|O_WRONLY, 0700, NULL) ;
    if (queue_servidor == -1) {
        fprintf(stderr, "Error al abrir la cola del servidor en el cliente.\n");
        return -1;
    }

    sprintf(client_name, "%s%d", "/CLIENTE_", getpid()) ;
    queue_cliente = mq_open(client_name, O_CREAT|O_RDONLY) ;
    if (queue_cliente == -1) {
        fprintf(stderr, "Error al abrir la cola del cliente en el cliente.\n");
        mq_close(queue_servidor);
        return -1;
    }

    // Rellenar la peticion
    memset(&p, 0, sizeof(struct peticion));
    p.op = 2;
    strcpy(p.q_name, client_name);
    p.key = key;
    p.valor1 = value1;
    p.valor2_value = V_value2;
    p.valor2_N_p = N_value2;

    if (mq_send(queue_servidor, (const char *)&p, sizeof(struct peticion), 0) < 0){
        fprintf(stderr, "Error al enviar el mensaje al servidor.\n");
        perror("mq_send");
        return -1;
    }
    if (mq_receive(queue_cliente, (char *)&r, sizeof(struct respuesta), 0) < 0){
        fprintf(stderr, "Error al recibir el mensaje del servidor.\n");
        perror("mq_recv");
        return -1;
    }

    mq_close(queue_servidor);
    mq_close(queue_cliente);
    mq_unlink(client_name);
    // No sé si hay que hacer cast de r a struct respuesta
    return r.status;

    /*
    // abirir cola servidor
    int open_s = open_server();
    // crear peticion
    struct peticion pet = {.key = key,
                           .op = 2,
                           .valor1 = value1,
                           .valor2_N_p = N_value2,
                           .valor2_value = V_value2};
    strcpy(pet.q_name, queue_name_c);
    // mandar peticion
    int send = send_server((char *)&pet, size_pet, prio_p);
    // abrir cola cliente
    int open_c = open_client();
    // recibir respuesta
    struct respuesta res;
    int rec = receive_client((char *)&res, size_resp, &prio_r);
    int exit_value = check_errors(open_c, open_s, send, rec);
    return exit_value;
    */
}

int modify_value(int key, char *value1, int N_value2, double *V_value2){
    struct peticion p;
    struct respuesta r;
    mqd_t queue_servidor;
    mqd_t queue_cliente;
    char client_name[MAX];
    // Diria que no hace falta hacer queue attr, es medio lioso ahora mismo, no entiendo muy bien

    queue_servidor = mq_open("/SERVIDOR", O_CREAT|O_WRONLY, 0700, NULL) ;
    if (queue_servidor == -1) {
        fprintf(stderr, "Error al abrir la cola del servidor en el cliente.\n");
        return -1;
    }

    sprintf(client_name, "%s%d", "/CLIENTE_", getpid()) ;
    queue_cliente = mq_open(client_name, O_CREAT|O_RDONLY) ;
    if (queue_cliente == -1) {
        fprintf(stderr, "Error al abrir la cola del cliente en el cliente.\n");
        mq_close(queue_servidor);
        return -1;
    }

    // Rellenar la peticion
    memset(&p, 0, sizeof(struct peticion));
    p.op = 3;
    strcpy(p.q_name, client_name);
    p.key = key;
    p.valor1 = value1;
    p.valor2_value = V_value2;
    p.valor2_N = N_value2;

    if (mq_send(queue_servidor, (const char *)&p, sizeof(struct peticion), 0) < 0){
        fprintf(stderr, "Error al enviar el mensaje al servidor.\n");
        perror("mq_send");
        return -1;
    }
    if (mq_receive(queue_cliente, (char *)&r, sizeof(struct respuesta), 0) < 0){
        fprintf(stderr, "Error al recibir el mensaje del servidor.\n");
        perror("mq_recv");
        return -1;
    }

    mq_close(queue_servidor);
    mq_close(queue_cliente);
    mq_unlink(client_name);
    // No sé si hay que hacer cast de r a struct respuesta
    return r.status;
    /*
    // abirir cola servidor
    int open_s = open_server();
    // crear peticion
    struct peticion pet = {.key = key,
                           .op = 3,
                           .valor1 = value1,
                           .valor2_N = N_value2,
                           .valor2_value = V_value2};
    strcpy(pet.q_name, queue_name_c);
    // mandar peticion
    int send = send_server((char *)&pet, size_pet, prio_p);
    // abrir cola cliente
    int open_c = open_client();
    // recibir respuesta
    struct respuesta res;
    int rec = receive_client((char *)&res, size_resp, &prio_r);
    int exit_value = check_errors(open_c, open_s, send, rec);
    return exit_value;
     */
}
int delete_key(int key){
    struct peticion p;
    struct respuesta r;
    mqd_t queue_servidor;
    mqd_t queue_cliente;
    char client_name[MAX];
    // Diria que no hace falta hacer queue attr, es medio lioso ahora mismo, no entiendo muy bien

    queue_servidor = mq_open("/SERVIDOR", O_CREAT|O_WRONLY, 0700, NULL) ;
    if (queue_servidor == -1) {
        fprintf(stderr, "Error al abrir la cola del servidor en el cliente.\n");
        return -1;
    }

    sprintf(client_name, "%s%d", "/CLIENTE_", getpid()) ;
    queue_cliente = mq_open(client_name, O_CREAT|O_RDONLY) ;
    if (queue_cliente == -1) {
        fprintf(stderr, "Error al abrir la cola del cliente en el cliente.\n");
        mq_close(queue_servidor);
        return -1;
    }

    // Rellenar la peticion
    memset(&p, 0, sizeof(struct peticion));
    p.op = 4;
    strcpy(p.q_name, client_name);
    p.key = key;

    if (mq_send(queue_servidor, (const char *)&p, sizeof(struct peticion), 0) < 0){
        fprintf(stderr, "Error al enviar el mensaje al servidor.\n");
        perror("mq_send");
        return -1;
    }
    if (mq_receive(queue_cliente, (char *)&r, sizeof(struct respuesta), 0) < 0){
        fprintf(stderr, "Error al recibir el mensaje del servidor.\n");
        perror("mq_recv");
        return -1;
    }

    mq_close(queue_servidor);
    mq_close(queue_cliente);
    mq_unlink(client_name);
    // No sé si hay que hacer cast de r a struct respuesta
    return r.status;
    /*
    // abirir cola servidor
    int open_s = open_server();
    // crear peticion
    struct peticion pet = {.key = key,
                            .op = 4};
    strcpy(pet.q_name, queue_name_c);
    // mandar peticion
    int send = send_server((char *)&pet, size_pet, prio_p);
    // abrir cola cliente
    int open_c = open_client();
    // recibir respuesta
    struct respuesta res;
    int rec = receive_client((char *)&res, size_resp, &prio_r);
    int exit_value = check_errors(open_c, open_s, send, rec);
    return exit_value;
     */
}

int exist(int key){
    struct peticion p;
    struct respuesta r;
    mqd_t queue_servidor;
    mqd_t queue_cliente;
    char client_name[MAX];
    // Diria que no hace falta hacer queue attr, es medio lioso ahora mismo, no entiendo muy bien

    queue_servidor = mq_open("/SERVIDOR", O_CREAT|O_WRONLY, 0700, NULL) ;
    if (queue_servidor == -1) {
        fprintf(stderr, "Error al abrir la cola del servidor en el cliente.\n");
        return -1;
    }

    sprintf(client_name, "%s%d", "/CLIENTE_", getpid()) ;
    queue_cliente = mq_open(client_name, O_CREAT|O_RDONLY) ;
    if (queue_cliente == -1) {
        fprintf(stderr, "Error al abrir la cola del cliente en el cliente.\n");
        mq_close(queue_servidor);
        return -1;
    }

    // Rellenar la peticion
    memset(&p, 0, sizeof(struct peticion));
    p.op = 4;
    strcpy(p.q_name, client_name);
    p.key = key;

    if (mq_send(queue_servidor, (const char *)&p, sizeof(struct peticion), 0) < 0){
        fprintf(stderr, "Error al enviar el mensaje al servidor.\n");
        perror("mq_send");
        return -1;
    }
    if (mq_receive(queue_cliente, (char *)&r, sizeof(struct respuesta), 0) < 0){
        fprintf(stderr, "Error al recibir el mensaje del servidor.\n");
        perror("mq_recv");
        return -1;
    }

    mq_close(queue_servidor);
    mq_close(queue_cliente);
    mq_unlink(client_name);
    // No sé si hay que hacer cast de r a struct respuesta
    return r.status;
    /*
    // abirir cola servidor
    int open_s = open_server();
    // crear peticion
    struct peticion pet = {.key = key,
                           .op = 4};
    strcpy(pet.q_name, queue_name_c);
    // mandar peticion
    int send = send_server((char *)&pet, size_pet, prio_p);
    // recibir respuesta
    // abrir cola cliente
    int open_c = open_client();
    struct respuesta res;
    int rec = receive_client((char *)&res, size_resp, &prio_r);
    int exit_value = check_errors(open_c, open_s, send, rec);
    return exit_value;
     */
}

int open_client(){
    // crear nombre de la cola
    sprintf(queue_name_c, "%s%d", "/CLIENTE_", getpid());
    // abrir cola cliente
    attr_cliente.mq_maxmsg = 10;
    attr_cliente.mq_msgsize = sizeof(struct respuesta);
    queue_cliente = mq_open(queue_name_c, O_CREAT | O_RDONLY, 0700, &attr_cliente);
    if (-1 == queue_cliente){
        fprintf(stderr, "Error al abrir la cola de mensajes del cliente.\n");
    }
    return queue_cliente;
}

// TODO: tal vez haya que cambiar el tema de los exits. habra que pensarlo
int open_server(){
    // crear nombre cola
    const static char queue_name_s[1024] = "/SERVIDOR";
    // abrir cola servidor
    attr_servidor.mq_maxmsg = 10;
    attr_servidor.mq_msgsize = sizeof(struct peticion);

    queue_servidor = mq_open(queue_name_s, O_CREAT| O_WRONLY, 0700, &attr_servidor);
    if (-1 == queue_servidor){
        fprintf(stderr, "Error al abrir la cola de mensajes del servidor.\n");
    }
    return queue_servidor;
}

int send_server(char * message, size_t size, unsigned int prio){
    int send = mq_send(queue_servidor, message, size, prio);
    if (-1 == send){
        fprintf(stderr, "Error al mandar el mensaje\n");
        return -1;
    }
    return 0;
}

int receive_client(char *message, size_t size, unsigned int *prio){
    int receive = mq_receive(queue_cliente, message, size, prio);
    if (-1 == receive){
        fprintf(stderr, "Error al recibir mensaje el mensaje\n");
        return -1;
    }
    return 0;
}
int check_errors(int open_c, int open_s, int send, int rec){
    if (open_c == -1 || open_s == -1 || send == -1 || rec == -1){
        return -1;
    }
    return 0;
}