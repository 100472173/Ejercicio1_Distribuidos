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
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

pthread_mutex_t sync_mutex;
bool sync_copied = false;
pthread_cond_t sync_cond;
mqd_t queue_servidor;

/* La jugada es la siguiente. Primero vamos a inicializar en el main el almacen con espacio para max_tuplas.
 * Luego, cuando vayamos a añadir una tupla comprobamos si el almacen esta lleno, si está lleno duplicamos
 * la capacidad y hacemos un realloc para poder seguir metiendo tuplas. OJO MUCHO CUIDADO porque los accesos
 * tanto al almacen como a las variables que controlan el numero de elementos y la capacidad deberían estar
 * protegidas por algún mecanismo de sincronizacion y exlusión mutua. MIRAR ESTO MUY BIEN PARA NO LIARLA */
struct tupla* almacen = NULL;
// numero de elementos en el array
int n_elementos = 0;
// elmentos maximos del array
int max_tuplas = 50;




int main ( int argc, char *argv[] )
{
    // Inicializamos el almacén
    almacen = (struct tupla*)malloc(max_tuplas*sizeof(struct tupla));
    // cargamos los datos del almacen
    if (-1 == load()){
        fprintf(stderr, "Error al cargar el almacen del archivo binary");
        return -1;
    }
    // Inicializamos peticion y variables
    struct peticion p;
    unsigned int prio = 0;
    int contador = 0;
    // Inicializamos los atributos de la cola
    struct mq_attr attr_servidor;
    attr_servidor.mq_maxmsg = 10;
    attr_servidor.mq_msgsize = sizeof(struct peticion);
    // Inicializamos los hilos
    pthread_attr_t attr;
    pthread_t thid[MAX];
    pthread_attr_init(&attr) ;
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    // Inicializamos mutex y variable condicion
    pthread_mutex_init(&sync_mutex, NULL);
    pthread_cond_init(&sync_cond, NULL);
    // Abrimos la cola para lectura
    queue_servidor = mq_open("/SERVIDOR", O_CREAT | O_RDONLY, 0700, &attr_servidor);
    if (queue_servidor == -1){
        fprintf(stderr, "Error al crear la cola de mensajes.\n");
        return -1;
    }
    while (1) {
        // para la recepcion tal vez haya que hacer unmarshaling la conversion que esta
        // aqui no se si sirva, hay que hacer pruebas.
        if (mq_receive(queue_servidor, (char *)&p, sizeof(struct peticion), &prio) < 0) {
            fprintf(stderr, "Error al recibir la petición de la cola de mensajes.\n");
            return -1;
        }
        if (pthread_create(&thid[contador], &attr, (void*)tratar_peticion, (struct perticion *) &p) != 0) {
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
    // liberar memoria almacen
    if (almacen != NULL){
        almacen = NULL;
        free(almacen);
    }
    
    return 0;
}

void tratar_peticion (struct peticion* p){
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
        case 1: resp.status = s_get_value(p_local.key, p_local.valor1, p_local.valor2_N, p_local.valor2_value);
            break;
        case 2: resp.status = s_set_value(p_local.key, p_local.valor1, p_local.valor2_N_p, p_local.valor2_value);
            break;
        case 3: resp.status = s_modify_value(p_local.key, p_local.valor1, p_local.valor2_N, p_local.valor2_value);
            break;
        case 4: resp.status = s_delete_key(p_local.key);
            break;
        case 5: resp.status = s_exist(p_local.key);
            break;
    }
    // Abrimos la cola del cliente
    //TODO: falta darle los atributos
    mqd_t queue_cliente = mq_open(p_local.q_name, O_CREAT | O_WRONLY, 0700, NULL);
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
            mq_unlink(p_local.q_name);
        }
    }
    pthread_exit(0) ;
}


int s_init() {
    // poner a 0 todos los elementos del almacen
    size_t elementos = max_tuplas * sizeof(struct tupla);
    memset(almacen, 0, elementos);
    // borar todas las tuplas del almacen
    char file[MAX];
    getcwd(file, sizeof(file));
    strcat(file, "/data_structure/almacen.txt");
    // abrir fichero y sobrescribir sus contenidos
    FILE * F = fopen(file, "wb");
    // error al abrir el fichero
    if (NULL == F){
        return -1;
    }
    fclose(F);
    return 0;

}

int s_get_value(int key, char *valor1, int valor2_N, double *valor2_value) {
    // iterar por el almacen
    int existe = -1;
    for (int i = 0; i < n_elementos; i++){
        if (almacen[i].clave == key){
            existe = 0;
            // copiar informacion
            strcpy(valor1, almacen[i].valor1);
            valor2_N = almacen[i].valor2_N;
            memcpy(valor2_value, almacen[i].valor2_value, valor2_N*sizeof(double));
        }
    }
    // devolver valor
    return existe;
}

int s_set_value(int key, char *valor1, int* valor2_N_p, double *valor2_value) {
    // comprobar errores
    // rango valor2n
    if (*valor2_N_p < 1 || *valor2_N_p > 32){
        fprintf(stderr, "Error: N_value2 no esta en el rango [1,32].\n");
        return -1;
    }
    // comprobar la existencia de key
    // iterar por el almacen
    for (int i = 0; i < n_elementos; i++){
        if (almacen[i].clave == key){
            fprintf(stderr, "Error: Ya existe la key en el almacen. \n");
            return -1;
        }
    }

    // crear tupla de insercion
    struct tupla insertar;
    insertar.clave = key;
    strcpy(insertar.valor1, valor1);
    memcpy(insertar.valor2_value, valor2_value, *valor2_N_p*sizeof(double));
    // comprobar el tamanio de almacen
    if (n_elementos == max_tuplas){
        // duplicar tamanio de almacen
        almacen = realloc(almacen, 2 * max_tuplas * sizeof(struct tupla));
        max_tuplas = max_tuplas * 2;
    }
    // agregar a almacen
    almacen[n_elementos] = insertar;
    n_elementos++;

    // devolver valor
    return 0;
}

int s_modify_value(int key, char *valor1, int valor2_N, double *valor2_value) {
    // comprobar error fuera de rango
    if (valor2_N < 1 || valor2_N > 32){
        fprintf(stderr, "Error: N_value2 no esta en el rango [1,32].\n");
        return -1;
    }
    // iterar por almacen
    int existe = -1;
    for (int i = 0; i < n_elementos; i++){
        if (almacen[i].clave == key){
            existe = 0;
            // modificar valores
            strcpy(almacen[i].valor1, valor1);
            // igualar todo el vector a 0
            memset(almacen[i].valor2_value, 0, 32*sizeof(double));
            // copiar parametro a vector
            memcpy(almacen[i].valor2_value, valor2_value, valor2_N*sizeof(double));
        }
    }
    return existe;
}

int s_delete_key(int key) {
    int existe = -1;
    // iterar por el almacen
    for (int i = 0; i < n_elementos; i++){
        if (almacen[i].clave == key){
            existe = 0;
            // esto funciona si el orden de las tuplas no importa. Sino hay que cambiarlo

            // copiar ultimo elemento del almacen al indice
            almacen[i] = almacen[n_elementos-1];
            // borrar ultimo elemento del almacen
            size_t elementos = sizeof(struct tupla);
            memset(almacen + ((n_elementos-1)*sizeof(struct tupla)), 0, elementos);
            // bajar el numero de elementos
            n_elementos--;
        }
    }
    return existe;
}

int s_exist(int key) {
    int existe = 0;
    // iterar por el almacen
    for (int i = 0; i<n_elementos; i++){
        if (almacen[i].clave == key){
            existe = 1;
        }
    }
    // devolver existencia
    return existe;

    // falta la parte del error
}
int load(){
    // obtener directorio
    char cwd[MAX];
    getcwd(cwd, sizeof(cwd));
    // crear directorio si no existe
    strcat(cwd, "/data_structure");
    DIR * dir = opendir(cwd);
    // existe el fichero
    if (dir){
        closedir(dir);
    }
    // no existe
    if (ENOENT == errno){
        mkdir(cwd, 0777);
    }
    // path del archivo
    char file[MAX];
    strcpy(file, cwd);
    strcat(file, "almacen.txt");

    // abrir descriptor de fichero
    FILE *F = fopen(file, "rb");
    // comprobar error al abrir fichero
    if (F == NULL){
        printf("Eror opening binary text file\n");
        return -1;
    }
    // bucle para ir leyendo elementos
    while(fread(&almacen[n_elementos], sizeof(struct tupla), 1, F) == 1){
        // comprobar el tamanio de almacen
        if (n_elementos == max_tuplas)
        {
            // duplicar tamanio de almacen
            almacen = realloc(almacen, 2 * max_tuplas * sizeof(struct tupla));
            max_tuplas = max_tuplas * 2;
        }
        n_elementos++;
    }
    fclose(F);
    return 0;
}
// hay que ponerla cuando termine el servidor
int write_back(){
    char file[MAX];
    getcwd(file, sizeof(file));
    strcat(file, "/data_structure/almacen.txt");
    // abrir descriptor de archivo
    FILE *F = fopen(file, "wb");
    // comprobar error al abrir archivo
    if (F == NULL){
        printf("Eror opening binary text file\n");
        return -1;
    }
    // bucle para escribir en archivo
    for (int i=0; i<n_elementos; i++){
        fwrite(&almacen[i], sizeof(struct tupla), 1, F);
    }
    return 0;
}