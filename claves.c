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


int init(){
    struct peticion p;
    struct respuesta r;
    mqd_t queue_servidor;
    mqd_t queue_cliente;
    char client_name[MAX];
    // abrir las colas
    int open_c = open_client(&queue_cliente);
    int open_s = open_server(&queue_servidor);
    
    // Rellenar la peticion
    sprintf(client_name, "%s%d", "/CLIENTE_", getpid());
    memset(&p, 0, sizeof(struct peticion));
    p.op = 0;
    strcpy(p.q_name, client_name);
    // mandar peticion al servidor
    int send_p = send_server(&queue_servidor, (const char *)&p, sizeof(struct peticion), 0);
    // recibir respuesta
    int recv_r = receive_client(&queue_cliente, (char *)&r, sizeof(struct respuesta), 0);
    // comprobar errores
    if (-1 == check_errors(open_c, open_s, send_p, recv_r)){
        printf("Communications error\n");
        return -1;
    }

    mq_close(queue_servidor);
    mq_close(queue_cliente);
    mq_unlink(client_name);
    // No sé si hay que hacer cast de r a struct respuesta
    return r.status;

}

int set_value(int key, char *value1, int N_value2, double *V_value2){
    struct peticion p;
    struct respuesta r;
    mqd_t queue_servidor;
    mqd_t queue_cliente;
    char client_name[MAX];
    // abrir las colas
    int open_c = open_client(&queue_cliente);
    int open_s = open_server(&queue_servidor);
    // Rellenar la peticion
    sprintf(client_name, "%s%d", "/CLIENTE_", getpid());
    memset(&p, 0, sizeof(struct peticion));
    p.op = 1;
    strcpy(p.q_name, client_name);
    p.key = key;
    strcpy(p.valor1, value1);
    p.valor2_N = N_value2;
    // copiar vector
    for (int i = 0; i < N_value2; i++) {
        p.valor2_value[i] = V_value2[i];
    }
    // mandar peticion al servidor
    int send_p = send_server(&queue_servidor, (const char *)&p, sizeof(struct peticion), 0);
    // recibir respuesta
    int recv_r = receive_client(&queue_cliente, (char *)&r, sizeof(struct respuesta), 0);
    // comprobar errores
    if (-1 == check_errors(open_c, open_s, send_p, recv_r)){
        printf("Communications error\n");
        return -1;
    }
    mq_close(queue_servidor);
    mq_close(queue_cliente);
    mq_unlink(client_name);
    // No sé si hay que hacer cast de r a struct respuesta
    return r.status;


}

int get_value(int key, char *value1, int *N_value2, double *V_value2){
    struct peticion p;
    struct respuesta r;
    mqd_t queue_servidor;
    mqd_t queue_cliente;
    char client_name[MAX];
    // abrir las colas
    int open_c = open_client(&queue_cliente);
    int open_s = open_server(&queue_servidor);

    // Rellenar la peticion
    sprintf(client_name, "%s%d", "/CLIENTE_", getpid());
    memset(&p, 0, sizeof(struct peticion));
    p.op = 2;
    strcpy(p.q_name, client_name);
    p.key = key;
    strcpy(p.valor1, value1);
    p.valor2_N_p = N_value2;

    // mandar peticion al servidor
    int send_p = send_server(&queue_servidor, (const char *)&p, sizeof(struct peticion), 0);
    // recibir respuesta
    int recv_r = receive_client(&queue_cliente, (char *)&r, sizeof(struct respuesta), 0);
    // comprobar errores
    if (-1 == check_errors(open_c, open_s, send_p, recv_r)){
        printf("Communications error\n");
        return -1;
    }
    // copiar valores
    strcpy(value1, r.valor1);
    *N_value2 = r.N_value2;
    for (int i = 0; i<r.N_value2; i++){
        V_value2[i] = r.valor2_value[i];
    }

    mq_close(queue_servidor);
    mq_close(queue_cliente);
    mq_unlink(client_name);
    // No sé si hay que hacer cast de r a struct respuesta
    return r.status;

}

int modify_value(int key, char *value1, int N_value2, double *V_value2){
    struct peticion p;
    struct respuesta r;
    mqd_t queue_servidor;
    mqd_t queue_cliente;
    char client_name[MAX];
    // abrir las colas
    int open_c = open_client(&queue_cliente);
    int open_s = open_server(&queue_servidor);

    // Rellenar la peticion
    sprintf(client_name, "%s%d", "/CLIENTE_", getpid());
    memset(&p, 0, sizeof(struct peticion));
    p.op = 3;
    strcpy(p.q_name, client_name);
    p.key = key;
    strcpy(p.valor1, value1);
    for (int i = 0; i < N_value2; i++){
        p.valor2_value[i] = V_value2[i];
    }
    p.valor2_N = N_value2;

    // mandar peticion al servidor
    int send_p = send_server(&queue_servidor, (const char *)&p, sizeof(struct peticion), 0);
    // recibir respuesta
    int recv_r = receive_client(&queue_cliente, (char *)&r, sizeof(struct respuesta), 0);
    // comprobar errores
    if (-1 == check_errors(open_c, open_s, send_p, recv_r)){
        printf("Communications error\n");
        return -1;
    }

    mq_close(queue_servidor);
    mq_close(queue_cliente);
    mq_unlink(client_name);
    // No sé si hay que hacer cast de r a struct respuesta
    return r.status;

}
int delete_key(int key){
    struct peticion p;
    struct respuesta r;
    mqd_t queue_servidor;
    mqd_t queue_cliente;
    char client_name[MAX];
    // abrir las colas
    int open_s = open_client(&queue_cliente);
    int open_c = open_server(&queue_servidor);

    // Rellenar la peticion
    sprintf(client_name, "%s%d", "/CLIENTE_", getpid());
    memset(&p, 0, sizeof(struct peticion));
    p.op = 4;
    strcpy(p.q_name, client_name);
    p.key = key;
    // mandar peticion al servidor
    int send_p = send_server(&queue_servidor, (const char *)&p, sizeof(struct peticion), 0);
    // recibir respuesta
    int recv_r = receive_client(&queue_cliente, (char *)&r, sizeof(struct respuesta), 0);
    // comprobar errores
    if (-1 == check_errors(open_c, open_s, send_p, recv_r)){
        printf("Communications error\n");
        return -1;
    }

    mq_close(queue_servidor);
    mq_close(queue_cliente);
    mq_unlink(client_name);
    // No sé si hay que hacer cast de r a struct respuesta
    return r.status;
}

int exist(int key){
    struct peticion p;
    struct respuesta r;
    mqd_t queue_servidor;
    mqd_t queue_cliente;
    char client_name[MAX];
    // abrir las colas
    int open_c = open_client(&queue_cliente);
    int open_s = open_server(&queue_servidor);

    // Rellenar la peticion
    sprintf(client_name, "%s%d", "/CLIENTE_", getpid());
    memset(&p, 0, sizeof(struct peticion));
    p.op = 5;
    strcpy(p.q_name, client_name);
    p.key = key;
    // mandar peticion al servidor
    int send_p = send_server(&queue_servidor, (const char *)&p, sizeof(struct peticion), 0);
    // recibir respuesta
    int recv_r = receive_client(&queue_cliente, (char *)&r, sizeof(struct respuesta), 0);
    // comprobar errores
    if (-1 == check_errors(open_c, open_s, send_p, recv_r)){
        printf("Communications error\n");
        return -1;
    }

    mq_close(queue_servidor);
    mq_close(queue_cliente);
    mq_unlink(client_name);
    // No sé si hay que hacer cast de r a struct respuesta
    return r.status;

}

int open_client(mqd_t *queue_cliente){
    char client_name[MAX];
    // crear nombre de la cola
    sprintf(client_name, "%s%d", "/CLIENTE_", getpid());
    // abrir cola cliente
    struct mq_attr attr_cliente;
    attr_cliente.mq_maxmsg = 10;
    attr_cliente.mq_msgsize = sizeof(struct respuesta);
    *queue_cliente = mq_open(client_name, O_CREAT | O_RDONLY, 0700, &attr_cliente);
    if (-1 == *queue_cliente){
        perror("mq_open");
    }
    return *queue_cliente;
}

int open_server(mqd_t * queue_servidor){
    // crear nombre cola
    const static char queue_name_s[1024] = "/SERVIDOR";
    // abrir cola servidor
    struct mq_attr attr_servidor;
    attr_servidor.mq_maxmsg = 10;
    attr_servidor.mq_msgsize = sizeof(struct peticion);

    *queue_servidor = mq_open(queue_name_s, O_CREAT| O_WRONLY, 0700, &attr_servidor);
    if (-1 == *queue_servidor){
        perror("mq_open");
    }
    return *queue_servidor;
}

int send_server(mqd_t * queue_servidor,const char * message, int size, unsigned int prio){
    int send = mq_send(*queue_servidor, message, size, prio);
    if (-1 == send){
        perror("mq_send");
        return -1;
    }
    return 0;
}

int receive_client(mqd_t *queue_cliente, char *message, int size, unsigned int *prio){
    int receive = mq_receive(*queue_cliente, message, size, prio);
    if (-1 == receive){
        perror("mq_recv");
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

