//
// Created by linux-lex on 29/02/24.
//
#include "claves.h"
#include <stdio.h>
#include <stdlib.h>

int main(){
    // prueba de set value
    int key = 1;
    char value1[20] = "Messi";
    int N_value2 = 5;
    double V_value2[5] = {5.2, 4.1, 3.1, 2.4, 1.5};
    int set_value_id = set_value(key, value1, N_value2, (double *) V_value2);
    printf("Set value result: %d\n", set_value_id);

    // int exists_id = exist(1);
    // printf("Exists result: %d\n", exists_id);

    /* // abir cola de mensajes para escribir
    mqd_t queue_write;
    struct mq_attr attr_write;
    // modificar atributos de cola
    queue_write = mq_open("/SERVIDOR", O_WRONLY, 0777, &attr_write); // abrir cola de mensajes
    if (-1 == queue_write){
        fprintf(stderr, "Error al abrir la cola de mensajes.\n");
        return -1;
    }
    // abir cola de mensajes para leer
    mqd_t queue_read;
    // modificar atributos cola
    struct mq_attr attr_read;
    char queue_r_name[1024];
    sprintf(queue_r_name, "%s%d", "/CLIENTE_", getpid());
    queue_read = mq_open(queue_r_name, O_CREAT| O_RDONLY, 0777, &attr_read);
    if (-1 == queue_write){
        fprintf(stderr, "Error al abrir la cola de mensajes.\n");
        return -1;
    }
    // llamar a funcion init
    int init_id = init();

    if (-1 == init_id){
        fprintf(stderr, "Error al borrar las tuplas.\n");
        return -1;
    }
    return queue_read; */

}
