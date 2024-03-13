//
// Created by linux-lex on 29/02/24.
//
#include "claves.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

/* int main(){
    // prueba de set value
    int key = 1;
    char value1[20] = "Messi";
    int N_value2 = 5;
    double V_value2[5] = {5.5, 4.5, 3.5, 2.5, 1.5};
    int set_value_id = set_value(key, value1, N_value2, (double *) V_value2);
    printf("Set value result: %d\n", set_value_id);

    // delete key
    // char value[256];
    // double vector[32];
    // int n_value;
    int get_value_id = delete_key(1);
    printf("Delete key result: %d.\n", get_value_id);
 
} */
int main(void)
{
    srand(time(NULL) * getpid()); // Inicialización adecuada de la semilla

    // Generación aleatoria de la operación y la clave
    int random_operation = rand() % 6;
    int random_key = (rand() % 100) + 1; // Clave en el rango de 1 a 10

    switch (random_operation)
    {
    // case 0:
    //     printf("Operación 'init' para la clave %d\n del pid %d", random_key, getpid());
    //     if (init() == -1){
    //         fprintf(stderr, "Error en init para la clave %d\n del pid %d", random_key, getpid());
    //     }
    //     break;
    case 1: // Operación 'set'
    {
        double values[3] = {1.5, 2.5, 3.5};
        printf("Operación 'set' para la clave %d\n del pid %d", random_key, getpid());
        if (set_value(random_key, "value", 3, values) == -1){
            fprintf(stderr, "Error en set_value para la clave %d\n del pid %d", random_key, getpid());
        }
    }
        break;
    
    case 2: // Operación 'get'
    {
        char value1[255];
        int N_value2;
        double V_value2[32];
        printf("Operación 'get' para la clave %d\n del pid %d", random_key, getpid());
        if (get_value(random_key, value1, &N_value2, V_value2) == -1){
            fprintf(stderr, "Error en get_value para la clave %d\n", random_key);
        }


    }
        break;
    case 3: //operacion modify
        {
        double values[3] = {1.5, 2.5, 3.5};
        printf("Operación 'modify' para la clave %d\n del pid %d", random_key, getpid());
        if (modify_value(random_key, "value2", 3, values) == -1) {
            fprintf(stderr, "Error en modify_value para la clave %d\n", random_key);
        }
        }
        break;
    case 4: // Operación 'delete'
        printf("Operación 'delete' para la clave %d\n del pid %d", random_key, getpid());
        if (delete_key(random_key) == -1){
            fprintf(stderr, "Error en delete_key para la clave %d\n", random_key);
        }
        break;
    case 5: // Operación 'exist'
        {
            printf("Operación 'exist' para la clave %d\n del pid %d", random_key, getpid());
            if (exist(random_key) == -1){
                fprintf(stderr, "Clave no existente para la operación 'exist' en la clave %d\n", random_key);
            }
        }
        break;
    }

    printf("He terminado la función\n");

    return 0;
}
