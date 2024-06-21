#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <iostream>
#include "pist3.h"
#include <errno.h>
#define DIM 26

#define PERROR(a)\
   {\
    LPVOID lpMsgBuf;\
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM\
                 |FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(),\
                  MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),\
                  (LPTSTR) &lpMsgBuf, 0, NULL);\
    fprintf (stderr, "%s:%s\n", a, lpMsgBuf);\
    LocalFree (lpMsgBuf);\
    }

HINSTANCE libreria = LoadLibrary(TEXT("pist3.dll"));

typedef int (*VAR_PIST_inicio)(unsigned int, int, int);
typedef int (*VAR_PIST_nuevoPistolero)(char);
typedef char (*VAR_PIST_vIctima)(void);
typedef int (*VAR_PIST_disparar)(char);
typedef int (*VAR_PIST_morirme)(void);
typedef int (*VAR_PIST_fin)(void);
typedef int (*VAR_refrescar)(void);
typedef void (*VAR_pon_error)(char);

struct fpistolero {

    VAR_PIST_inicio PIST_inicio;
    VAR_PIST_nuevoPistolero PIST_nuevoPistolero;
    VAR_PIST_vIctima PIST_vIctima;
    VAR_PIST_disparar PIST_disparar;
    VAR_PIST_morirme PIST_morirme;
    VAR_PIST_fin PIST_fin;
    VAR_refrescar refrescar;
    VAR_pon_error pon_error;


}f;
typedef struct datos
{
    HANDLE sem1, sem2, sem3, sem4, sem5, sem6, sem7;
    int posicion, retorno, ganador;
    HANDLE pistoleros[DIM];
    int jugadores[DIM];

}dat;
DWORD WINAPI pistolero(LPVOID pistolero);


int seed, velocidad, n_pistoleros;
int procesos_vivos = 0;
MSG mensaje;
int coord = 1;
int numVictima;

int carga_funciones();


char letras_asignadas[] = { "ABCDEFGHIJKLMN?OPQRSTUVWXYZ" };

int main(int argc, char* argv[])
{
    int i;
    
    dat datos;
    system("mode con:cols=80 lines=25");
    n_pistoleros = atoi(argv[1]);
    velocidad = atoi(argv[2]);
    printf("El numero de pistoleros es %d.", n_pistoleros - 2);
    printf("La velocidad de ejecucion es %d.", velocidad);

    if (n_pistoleros < 0 || n_pistoleros>26) {
        perror("Numero de procesos incorrecto.\n");
        exit(-1);
    }

    if (velocidad < 0) {
        perror("Velocidad incorrecta. Introduce una velocidad positiva.\n");
        exit(-1);
    }

    if (argc == 3 && argv[3] > 0) {
        seed = atoi(argv[3]);
    }
    else {
        seed = 0;
    }

    carga_funciones();

    datos.sem1 = CreateSemaphore(NULL, 0, DIM, NULL);
    if (datos.sem1 == NULL)
    {
        PERROR("Error al crear semaforo sem1");
        return 100;
    }

    datos.sem2 = CreateSemaphore(NULL, 0, DIM, NULL);
    if (datos.sem2 == NULL)
    {
        PERROR("Error al crear semaforo sem2");
        return 100;
    }

    datos.sem3 = CreateSemaphore(NULL, 0, DIM, NULL);
    if (datos.sem3 == NULL)
    {
        PERROR("Error al crear semaforo sem3");
        return 100;
    }

    datos.sem4 = CreateSemaphore(NULL, 0, DIM, NULL);
    if (datos.sem4 == NULL)
    {
        PERROR("Error al crear semaforo sem4");
        return 100;
    }

    datos.sem5 = CreateSemaphore(NULL, 0, DIM, NULL);
    if (datos.sem5 == NULL)
    {
        PERROR("Error al crear semaforo sem5");
        return 100;
    }

    datos.sem6 = CreateSemaphore(NULL, 0, DIM, NULL);
    if (datos.sem6 == NULL)
    {
        PERROR("Error al crear semaforo sem6");
        return 100;
    }

    datos.sem7 = CreateSemaphore(NULL, 1, 1, NULL);
    if (datos.sem7 == NULL)
    {
        PERROR("Error al crear semaforo sem6");
        return 100;
    }


    ///// COMIENZO DE LA EJECUCION DEL PROGRAMA
    f.PIST_inicio(n_pistoleros, velocidad, seed);


    // CREACION DE PROCESOS  
    procesos_vivos = n_pistoleros;

    i = 0;
    for (i = 0; i < n_pistoleros; i++) {

        
        datos.jugadores[i] = i;
        datos.posicion = i;
        datos.pistoleros[i] = CreateThread(NULL, 0, pistolero, &datos, 0, NULL);
        WaitForSingleObject(datos.sem1, INFINITE);
       


    }
    ReleaseSemaphore(datos.sem2, n_pistoleros, NULL);
    WaitForMultipleObjects(n_pistoleros, datos.pistoleros, TRUE, INFINITE);

    for (i = 0; i < n_pistoleros; i++) {
        if ((GetExitCodeThread(datos.pistoleros, LPDWORD(&datos.retorno)) == 0)) {
            PERROR("GetExitCodeThread");
            return 100;
        }

        if (datos.retorno == 1) {
            datos.ganador = i + 1;
        }
    }


    f.PIST_fin();
}


DWORD WINAPI pistolero(LPVOID pistolero) {
    dat* p_datos;
    int numHilo;
    int j, i = 0;
    char letraVictima;
    int numVictima;
    int jugando = n_pistoleros;
    int pist;
    p_datos = (dat*)pistolero;
    f.PIST_nuevoPistolero(letras_asignadas[p_datos->posicion]);
    numHilo = p_datos->posicion;
    ReleaseSemaphore(p_datos->sem1, 1, NULL);
    WaitForSingleObject(p_datos->sem2, INFINITE);

    for (;;) {
        letraVictima = f.PIST_vIctima();
        f.PIST_disparar(letraVictima);

        
        WaitForSingleObject(p_datos->sem7, INFINITE);
        for (j = 0; j < DIM; j++) {
            if (letras_asignadas[j] == letraVictima) {
                numVictima = j;
                p_datos->jugadores[numVictima] = -1;
            }
        }
        ReleaseSemaphore(p_datos->sem7, 1, NULL);

        pist = jugando;
        //Esperamos que todos hayan disparado antes de que comprueben si han muerto
        ReleaseSemaphore(p_datos->sem3, 1, NULL);
        if (numHilo == coord) {
            for (j = 0; j < jugando; j++) {
                WaitForSingleObject(p_datos->sem3, INFINITE);
            }

            //Actualizamos el numero de jugadores
            jugando = 0;

            for (j = 0; j < n_pistoleros; j++) {
                if (p_datos->jugadores[j] != -1) {
                    jugando++;
                }
            }

            //Buscamos al siguiente coordinador
            for (j = 0; j < n_pistoleros; j++) {
                if (p_datos->jugadores[j] != -1) {
                    coord = i;
                    break;
                }
            }

            ReleaseSemaphore(p_datos->sem4, jugando, NULL);
        }
        WaitForSingleObject(p_datos->sem4, INFINITE);


        if (p_datos->jugadores[numHilo] == -1) {
            
            f.PIST_morirme();
            ReleaseSemaphore(p_datos->sem5, 1, NULL);
            exit(0);
        }


        //Esperamos que la variable jugdores se actualize correctamente
        ReleaseSemaphore(p_datos->sem5, 1, NULL);
        if (numHilo == coord) {
            for (j = 0; j < pist; j++) {
                WaitForSingleObject(p_datos->sem5, INFINITE);
            }
            ReleaseSemaphore(p_datos->sem6, jugando, NULL);
        }
        WaitForSingleObject(p_datos->sem6, INFINITE);

        if (jugando == 1) {
            //Solo queda un jugador (ganador)
            exit(numHilo);
        }
    }

}

int carga_funciones() {


    if ((f.PIST_inicio = (VAR_PIST_inicio)GetProcAddress(libreria, "PIST_inicio")) == NULL) {
        exit(-1);
    }


    if ((f.PIST_nuevoPistolero = (VAR_PIST_nuevoPistolero)GetProcAddress(libreria, "PIST_nuevoPistolero")) == NULL) {
        exit(-1);
    }


    if ((f.PIST_vIctima = (VAR_PIST_vIctima)GetProcAddress(libreria, "PIST_vIctima")) == NULL) {
        exit(-1);
    }

    if ((f.PIST_disparar = (VAR_PIST_disparar)GetProcAddress(libreria, "PIST_disparar")) == NULL) {
        exit(-1);
    }


    if ((f.PIST_morirme = (VAR_PIST_morirme)GetProcAddress(libreria, "PIST_morirme")) == NULL) {
        exit(-1);
    }


    if ((f.PIST_fin = (VAR_PIST_fin)GetProcAddress(libreria, "PIST_fin")) == NULL) {
        exit(-1);
    }

    if ((f.refrescar = (VAR_refrescar)GetProcAddress(libreria, "refrescar")) == NULL) {
        exit(-1);
    }

    if ((f.pon_error = (VAR_pon_error)GetProcAddress(libreria, "pon_error")) == NULL) {
        exit(-1);
    }

    return 0;

}
