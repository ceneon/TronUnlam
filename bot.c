/********************************************
      Bot (Jugador Virtual)  de Nk-Tron
*********************************************
Trabajo Practico 4

Alumnos:
Silvina Torres  	Dni: 30.724.953
Marcos Wolff		Dni: 31.925.539
Nicolas Cerrini Madrid	Dni: 32.111.349
Maximiliano Almada	Dni: 29.524.290

Comision Martes Noche - 2006

Archivo: bot.c
Descripcion del archivo:
Define el proceso de cliente virtual del juego.
Es ejecutado automaticamente por el proceso servidor
*/
 
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

#include "fcsemaforos.c"
#define MAX_JUGADORES 6

int main(int argc,char *argv[])
{
key_t llave,llaved,llavedx,llavedy;
int **shmatriz;
int *direcc,*x,*y;
int id_mem,id_memd,id_memdx,id_memdy;
int nrojugador,semaf;
//int matriz[80][23];
int *matriz;

if (argc <2)
{
printf("Soy un bot que le falta un parametro, MATENME\n");
while(1);
}

nrojugador=atoi(argv[1]);
printf("Soy el bot jugador %d\n",nrojugador+1);

llave=ftok("./cliente.c",33);
id_mem=shmget(llave,sizeof(int[80][23]),0777 | IPC_CREAT);
if(id_mem==-1)
{       printf("No consegui memoria!\n");return 1; }
shmatriz=(int **)shmat(id_mem,(void *)0,0);
if(!shmatriz)
{       printf("No consegui compartir memoria!\n");return 1; }



//shmatriz=(int **)matriz;
matriz=(int *)shmatriz;


llaved=ftok("./cliente.c",32);
id_memd=shmget(llaved,sizeof(int[MAX_JUGADORES]),0777 | IPC_CREAT);
if(id_memd==-1)
{       printf("No consegui memoria!\n");return 1; }
direcc=(int *)shmat(id_memd,(char *)0,0);
if(!direcc)
{       printf("No consegui compartir memoria!\n");return 1; }

llavedx=ftok("./cliente.c",31);
id_memdx=shmget(llavedx,sizeof(int[MAX_JUGADORES]),0777 | IPC_CREAT);
if(id_memdx==-1)
{       printf("No consegui memoria!\n");return 1; }
x=(int *)shmat(id_memdx,(char *)0,0);
if(!x)
{       printf("No consegui compartir memoria!\n");return 1; }

llavedy=ftok("./cliente.c",30);
id_memdy=shmget(llavedy,sizeof(int[MAX_JUGADORES]),0777 | IPC_CREAT);
if(id_memdy==-1)
{       printf("No consegui memoria!\n");return 1; }
y=(int *)shmat(id_memdy,(char *)0,0);
if(!y)
{       printf("No consegui compartir memoria!\n");return 1; }


llave=ftok("./cliente.c",40+nrojugador);
semaf=obtenersem(llave);

while(1)
{	int choke=0,i,x2,y2,x3,y3;
semrojo(semaf);

//printf("BOT: hago algo?\t");

if(direcc[nrojugador]!=-1)
{	x3=x[nrojugador];
	y3=y[nrojugador];
	for(i=0;i<2;i++)
	{	
		switch(direcc[nrojugador])
		{	case 1:
				y3--;
			break;
			case 2:
				x3++;
			break;
			case 3:
				y3++;
			break;
			case 4:
				x3--;
		}
		if(i==0)
		{  x2=x3;
		   y2=y3;
		}
	}
	printf(" act: <%d,%d> prox: <%d,%d> mas: <%d,%d>\n",
		x[nrojugador],y[nrojugador],x2,y2,x3,y3);
	if( x2<0 || x2>79 || y2<0 || y2>22)
		choke=1;
	if( x3<0 || x3>79 || y3<0 || y3>22)
		choke=1;			
	if(!choke)
	{	if(shmatriz[x2*80+y2]!=0)
		//if(matriz[y2*23+x2]!=0)
			choke=1;
		if(shmatriz[x3*80+y3]!=0)
		//if(matriz[y3*23+x3]!=0)
			choke=1;
	}	 


	int j;
	for(i=100000;i<79;i++)
	{
		for(j=0;j<23;j++)
			//printf("%d",matriz[i][j]);
			printf("%d",matriz[i*80+j]);
		printf("\n");
	}


	if(choke)
	{	//randomize();
			i=random()%9;    
			printf("BOT: voy a chocar !\n");
			if(direcc[nrojugador]==1 || direcc[nrojugador]==3)
			{	if(i<4)
				  direcc[nrojugador]=2;
				if(i>4)
				  direcc[nrojugador]=4;
			}
			else
			{
				if(i<4)
				  direcc[nrojugador]=1;
				if(i>4)
				  direcc[nrojugador]=3;
			}
	}
	else
	{
		i=random()%10;
		if(i==0)
			direcc[nrojugador]++;
		if(i==1)
			direcc[nrojugador]--;
		if(direcc[nrojugador]==0) direcc[nrojugador]=4;
		if(direcc[nrojugador]==5) direcc[nrojugador]=1;
		
	}
 }//if direcc!=-1

}//while(1)

printf("BOT: estoy en %d,%d\n",x[nrojugador],y[nrojugador]);
sleep(10);
printf("pa arriba\n");
direcc[nrojugador]=1;
printf("BOT: estoy en %d,%d\n",x[nrojugador],y[nrojugador]);
sleep(10);
printf("pa derecha\n");
direcc[nrojugador]=2;
printf("BOT: estoy en %d,%d\n",x[nrojugador],y[nrojugador]);
sleep(10);
printf("pa arriba \n");
direcc[nrojugador]=3;
printf("BOT: estoy en %d,%d\n",x[nrojugador],y[nrojugador]);


shmctl (id_mem, IPC_RMID, (struct shmid_ds *)NULL);
shmctl (id_memd, IPC_RMID, (struct shmid_ds *)NULL);
shmctl (id_memdx, IPC_RMID, (struct shmid_ds *)NULL);
shmctl (id_memdy, IPC_RMID, (struct shmid_ds *)NULL);
while(1);
}


