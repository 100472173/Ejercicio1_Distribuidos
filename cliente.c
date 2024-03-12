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
    double V_value2[5] = {5.5, 4.5, 3.5, 2.5, 1.5};
    int set_value_id = set_value(key, value1, N_value2, (double *) V_value2);
    printf("Set value result: %d\n", set_value_id);


    char value[256];
    double vector[32];
    int n_value;
    int get_value_id = get_value(1, value, &n_value, vector);
    printf("Get value result: %d.\n %s, %d, %f\n", get_value_id, value, n_value, vector[0]);

}
