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
#include <signal.h>


// mutex para copiar la peticion recibida a una variable local
pthread_mutex_t sync_mutex;
bool sync_copied = false;
pthread_cond_t sync_cond;
// cola de servidor
mqd_t queue_servidor;
// mutex para hacer peticiones concurrentes. Escrituras en almacen
pthread_mutex_t almacen_mutex;




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





int main (){
    // Inicializamos el almacén
    almacen = (struct tupla*)malloc(max_tuplas*sizeof(struct tupla));
    // senial para cerrar servidor
    signal(SIGINT, close_server);
        // cargamos los datos del almacen
        if (-1 == load()){
        fprintf(stderr, "Error en servidor al cargar el almacen del archivo binary\n");
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
    pthread_mutex_init(&almacen_mutex, NULL);
    pthread_cond_init(&sync_cond, NULL);
    // Abrimos la cola para lectura
    queue_servidor = mq_open("/SERVIDOR", O_CREAT | O_RDONLY, 0700, &attr_servidor);
    if (queue_servidor == -1){
        perror("Error en servidor. Mq_open queue servidor");
        return -1;
    }
    while (1) {
        if (mq_receive(queue_servidor, (char *)&p, sizeof(struct peticion), &prio) < 0) {
            perror("Error en servidor. Mq_recv");
            return -1;
        }
        if (pthread_create(&thid[contador], &attr, tratar_peticion, (struct perticion *) &p) != 0) {
            perror("Error en servidor. Pthread_create");
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

void * tratar_peticion (void* pp){
    struct peticion * p = pp;
    struct mq_attr attr_cliente;
    attr_cliente.mq_maxmsg = 10;
    attr_cliente.mq_msgsize = sizeof(struct respuesta);
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
    switch (p_local.op){
    case 0:
        resp.status = s_init();
        break;
    case 1:
        resp.status = s_set_value(p_local.key, p_local.valor1, p_local.valor2_N, p_local.valor2_value);
        break;
    case 2:
        resp.status = s_get_value(p_local.key, resp.valor1, &resp.N_value2, resp.valor2_value);
        break;
    case 3:
        resp.status = s_modify_value(p_local.key, p_local.valor1, p_local.valor2_N, p_local.valor2_value);
        break;
    case 4:
        resp.status = s_delete_key(p_local.key);
        break;
    case 5:
        resp.status = s_exist(p_local.key);
        break;
    }
    // Abrimos la cola del cliente
    mqd_t queue_cliente = mq_open(p_local.q_name, O_CREAT | O_WRONLY, 0700, &attr_cliente);
    if (queue_cliente == -1)
    {
        perror("Error en servidor. Mq_open queue cliente");
        mq_close(queue_servidor);
        mq_unlink("/SERVIDOR");
    }
    else
    {
        // Mandamos el mensaje al cliente
        if (mq_send(queue_cliente, (char *)&resp, sizeof(struct respuesta), 0) < 0)
        {
            perror("Error en servidor. Mq_send");
            mq_close(queue_servidor);
            mq_unlink("/SERVIDOR");
            mq_close(queue_cliente);
            mq_unlink(p_local.q_name);
        }
        mq_close(queue_cliente);
        mq_unlink(p_local.q_name);
    }

    return NULL;
    // pthread_exit(0) ;
}

int s_init() {
    // mutex lock
    pthread_mutex_lock(&almacen_mutex);
    free(almacen);
    // Cada vez que haces free del almacen y lo pones a 0. Cuando un nuevo servidor
    // viene es posible que lo liberes a la vez que otro cliente esta borrando
    // Ya pero en el otro no tienes mutex
    almacen = NULL;
    almacen = (struct tupla *)malloc(max_tuplas * sizeof(struct tupla));
    // poner a 0 todos los elementos del almacen
    size_t elementos = max_tuplas * sizeof(struct tupla);
    memset(almacen, 0, elementos);
    // mutex unlock
    // borar todas las tuplas del almacen
    char file[MAX];
    getcwd(file, sizeof(file));
    strcat(file, "/data_structure/almacen.txt");
    // abrir fichero y sobrescribir sus contenidos
    FILE * f = fopen(file, "w");
    // error al abrir el fichero
    if (NULL == f){
        perror("Error en servidor al abrir el fichero");
        return -1;
    }
    fclose(f);
    pthread_mutex_unlock(&almacen_mutex);

    return 0;

}

int s_set_value(int key, char *valor1, int valor2_N, double *valor2_value) {
    // bloquear el mutex
    pthread_mutex_lock(&almacen_mutex);
    // comprobar errores
    // rango valor2n
    if (valor2_N < 1 || valor2_N > 32){
        fprintf(stderr, "Error: N_value2 no esta en el rango [1,32].\n");
        pthread_mutex_unlock(&almacen_mutex);
        return -1;
    }
    // comprobar la existencia de key
    // iterar por el almacen
    
    for (int i = 0; i < n_elementos; i++){
        printf("%d %d \n", almacen[i].clave, key);
        if (almacen[i].clave == key){
            fprintf(stderr, "Error: Ya existe la key en el almacen. \n");
            pthread_mutex_unlock(&almacen_mutex);
            return -1;
        }
    }
    // comprobar el tamanio de almacen
    if (n_elementos == max_tuplas){
        // duplicar tamanio de almacen
        almacen = realloc(almacen, 2 * max_tuplas * sizeof(struct tupla));
        max_tuplas = max_tuplas * 2;
    }

    // crear tupla de insercion
    struct tupla insertar;
    insertar.clave = key;
    insertar.valor2_N = valor2_N;
    strcpy(insertar.valor1, valor1);
    // copiar vector
    for (int i = 0; i < valor2_N; i++) {
        insertar.valor2_value[i] = valor2_value[i];
    }
    printf("%d \n", insertar.clave);
    // agregar a almacen
    
    almacen[n_elementos] = insertar;
    printf("%d\n", almacen[n_elementos].clave);
    n_elementos++;
    // desbloquear mutex
    pthread_mutex_unlock(&almacen_mutex);
    // devolver valor
    return 0;
}

int s_get_value(int key, char *valor1, int *valor2_N, double *valor2_value){
    // iterar por el almacen
    pthread_mutex_lock(&almacen_mutex);
    int existe = -1;
    for (int i = 0; i < n_elementos; i++){
        if (almacen[i].clave == key){
            existe = 0;
            // copiar informacion
            strcpy(valor1, almacen[i].valor1);
                *valor2_N = almacen[i].valor2_N;
            for (int j = 0; j<almacen[i].valor2_N; j++){
                valor2_value[j] = almacen[i].valor2_value[j];
            }
        }
    }
    pthread_mutex_unlock(&almacen_mutex);
    // devolver valor
    return existe;
}

int s_modify_value(int key, char *valor1, int valor2_N, double *valor2_value) {
    pthread_mutex_lock(&almacen_mutex);
    // comprobar error fuera de rango
    if (valor2_N < 1 || valor2_N > 32){
        fprintf(stderr, "Error: N_value2 no esta en el rango [1,32].\n");
        pthread_mutex_unlock(&almacen_mutex);
        return -1;
    }
    // iterar por almacen
    
    int existe = -1;
    for (int i = 0; i < n_elementos; i++){
        if (almacen[i].clave == key){
            
            existe = 0;
            // modificar valores
            strcpy(almacen[i].valor1, valor1);
            almacen[i].valor2_N = valor2_N;
            // // igualar todo el vector a 0
            memset(almacen[i].valor2_value, 0, 32*sizeof(double));
            // copiar parametro a vector
            for (int j = 0; j<valor2_N; j++){
                almacen[i].valor2_value[j] = valor2_value[j];
            }
        }
    }
    pthread_mutex_unlock(&almacen_mutex);
    return existe;
}

int s_delete_key(int key) {
    pthread_mutex_lock(&almacen_mutex);
    int existe = -1;
    // iterar por el almacen
    for (int i = 0; i < n_elementos; i++){
        if (almacen[i].clave == key){
            existe = 0;
            // esto funciona si el orden de las tuplas no importa. Sino hay que cambiarlo
            // copiar ultimo elemento del almacen al indice
            almacen[i] = almacen[n_elementos-1];
            // borrar ultimo elemento del almacen
            struct tupla tupla_vacia;
            memset(&tupla_vacia, 0, sizeof(struct tupla));
            almacen[n_elementos-1] = tupla_vacia;
            // bajar el numero de elementos
            n_elementos--;
        }
    }
    pthread_mutex_unlock(&almacen_mutex);
    return existe;
}

int s_exist(int key) {
    pthread_mutex_lock(&almacen_mutex);
    int existe = 0;
    // iterar por el almacen
    for (int i = 0; i<n_elementos; i++){
        if (almacen[i].clave == key){
            existe = 1;
        }
    }
    pthread_mutex_unlock(&almacen_mutex);
    // devolver existencia
    return existe;
   
    // falta la parte del error
}

void close_server(){
    // hacer el free y salir
    printf("\n Closing server \n");
    write_back();
    free(almacen);
    almacen = NULL;
    mq_close(queue_servidor);
    mq_unlink("/SERVIDOR");
    exit(0);
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
    strcat(file, "/almacen.txt");

    // abrir descriptor de fichero
    FILE *f = fopen(file, "a+");
    rewind(f);
    // comprobar error al abrir fichero
    if (f == NULL){
        perror("Error en servidor al abrir el fichero");
        return -1;
    }
    // bucle para ir leyendo elementos
    while(fread(&almacen[n_elementos], sizeof(struct tupla), 1, f) == 1){
        // comprobar el tamanio de almacen
        if (n_elementos == max_tuplas-1){
            // duplicar tamanio de almacen
            almacen = realloc(almacen, 2 * max_tuplas * sizeof(struct tupla));
            max_tuplas = max_tuplas * 2;
        }
        n_elementos++;
    }
    fclose(f);
    /*
    // reescribir el archibo el archivo
    FILE *f_erase = fopen(file, "w");
    // comprobar reescribiendo el fichero
    if (f_erase == NULL){
        perror("Error en servidor al reescribir el fichero");
        return -1;
    }
    fclose(f_erase);
     */
    return 0;
}
int write_back(){
    // cerrar colas servidor
    char file[MAX];
    getcwd(file, sizeof(file));
    strcat(file, "/data_structure/almacen.txt");
    // abrir descriptor de archivo
    FILE *f = fopen(file, "w");
    // comprobar error al abrir archivo
    if (f == NULL){
        perror("Error en servidor al abrir el fichero");
        return -1;
    }
    // bucle para escribir en archivo
    for (int i=0; i<n_elementos; i++){
        fwrite(&almacen[i], sizeof(struct tupla), 1, f);
    }
    fclose(f);
    return 0;
}