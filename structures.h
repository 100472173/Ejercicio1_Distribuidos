//
// Created by linux-lex on 1/03/24.
//

#ifndef EJERCICIO1_DISTRIBUIDOS_STRUCTURES_H
#define EJERCICIO1_DISTRIBUIDOS_STRUCTURES_H

#define MAX 256

struct peticion {
    int op;
    int key;
    char* valor1;
    int valor2_N;
    int* valor2_N_p;
    double* valor2_value;
    char q_name[MAX];
};

struct respuesta {
    int status;
};

struct tupla {
    int clave;
    char valor1[MAX];
    int valor2_N;
    double* valor2_value;
};
int s_init();
int s_get_value();
int s_set_value();
int s_modify_value();
int s_delete_key();
int s_exist();

#endif //EJERCICIO1_DISTRIBUIDOS_STRUCTURES_H
