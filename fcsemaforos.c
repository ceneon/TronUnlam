/********************************************
      Funciones de Semaforos de Nk-Tron
*********************************************
Archivo: fcsemaforos.c
Descripcion del archivo:
Ofrece funciones de comunicacion de procesos con semaforos.
Es utilizado por el proceso servidor y el proceso bot.
*/


//Prototipos
//
// semrojo(int id_sem)
// semverde(int id_sem)
// creasem(int numero)
// iniciasem(int sem,int valor)

#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <errno.h>

void semrojo(int id_sem)//poner en rojo el semaforo
{
        struct sembuf operacion;
        operacion.sem_num=0;
        operacion.sem_op=-1;
        operacion.sem_flg=0;
        semop (id_sem, &operacion, 1);
}
void semverde(int id_sem)//poner en verde el semaforo
{
        struct sembuf operacion;
        operacion.sem_num=0;
        operacion.sem_op=1;
        operacion.sem_flg=0;
        semop (id_sem, &operacion, 1);
}

int obtenersem(key_t llave)
{       int id_sem;
        id_sem=semget (llave, 1, 0777 | IPC_CREAT );
        if (id_sem==-1)
        {printf ("no consegui semaforo:%d\n",errno);exit(1);}
        return id_sem;
}

void iniciasem(int sem,int valor)
{
        semctl (sem, 0/*numero de sem*/, SETVAL/*inicializar*/, valor);
}

int creasem(int numero)
{	int id_sem;
        key_t llave=  ftok ("/bin/ls", numero);
	if (llave==-1)
	{printf ("no consegui llave\n");exit(1);}
	id_sem=obtenersem(llave);
	return id_sem;
}
