/********************************************
	Servidor de Nk-Tron
*********************************************
Trabajo Practico 4

Alumnos:
Silvina Torres          Dni: 30.724.953
Marcos Wolff            Dni: 31.925.539
Nicolas Cerrini Madrid  Dni: 32.111.349
Maximiliano Almada      Dni: 29.524.290

Comision Martes Noche - 2006

Archivo: servidor.c
Descripcion del archivo:
Codigo fuente del proceso servidor del juego.
Debe ejecutarse una instancia en alguna maquina de la red para poder organizar el juego.
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
#include <sys/types.h>

//#define PUERTO 9999
#define TAM_BUFFER 1024
#define COLA 20

#include "socket.c"
#include "fcsemaforos.c"

void* atender (void *);
void llenardatos(struct paketeSC *);
void procesarmovimientos();
void terminar();
void limpiamatriz();
int alguienvive();
void borrarmatriz(int);

int fdlisten;
pthread_t clientes[MAX_JUGADORES];
char vnombres[7][MAX_JUGADORES];
int fdclientes[MAX_JUGADORES];
int cant_niveles,jugadores,bots,dificultad;
int cont_clientes=0;

int matriz[80][23];
//int direcc[MAX_JUGADORES];
int **shmatriz;
int *direcc;
int id_mem,id_memd,id_memdx,id_memdy;
pid_t botpid[MAX_JUGADORES];
int *x, *y;
//int x[MAX_JUGADORES];
//int y[MAX_JUGADORES];
int sembot[MAX_JUGADORES];

int puntajes[MAX_JUGADORES];
int ganador;

int main (int argc, char* argv[])
{
	int error,i,j,ronda;
	int fdcliente;
	key_t llave,llaved,llavedx,llavedy;
	struct paketeSC packout;

	signal(SIGINT,terminar);

	//Valores por default
	cant_niveles=10;
	dificultad=5;
	bots=0;

	switch(argc)
	{
		case 1:
			printf("Servidor de NkTron\nERROR: Faltan parametros\n");
			printf("Uso: servidor Cant_Jugadores [Dificultad][Rondas][Bots]\n");
			printf("  Cant_Jugadores=2..6 , Dificultad=1..10 (0=Especial) , ");
			printf("Rondas=1..20 , Bots=1..6\n");
			return 1; 
		break;
		case 5:
			bots=atoi(argv[4]);
			if (bots<0 || bots >MAX_JUGADORES)
			{	printf("Cantidad de bots invalida (0..%d)\n",MAX_JUGADORES);
				return 1;
			}
		case 4:
			cant_niveles=atoi(argv[3]);
			if (cant_niveles<1 || cant_niveles>20)
			{	printf("Cantidad de rondas invalida (1..20)\n");
				return 1;
			}
		case 3:
			dificultad=atoi(argv[2]);
			if (dificultad<0 || dificultad>10)
			{	printf ("Dificultad invalida (0..10) (0=Especial)\n");
				return 1;
			}
		case 2:
			jugadores=atoi(argv[1]);
			if (jugadores<2 || jugadores>MAX_JUGADORES)
			{	printf("Cantidad de jugadores invalida (2..%d)\n",MAX_JUGADORES);
				return 1;
			}
	}
	if (bots>jugadores)
	{	printf("Cantidad de bots invalida, mayor a jugadores totales\n");
		return 1;
	}

	llave=ftok("./cliente.c",33);
	id_mem=shmget(llave,sizeof(int[80][23]),0777 | IPC_CREAT);
	if(id_mem==-1)
	{	printf("No consegui memoria!\n");return 1; }
	shmatriz=(int **)shmat(id_mem,(char *)0,0);
	if(!shmatriz)
	{	printf("No consegui compartir memoria!\n");return 1; }
	
	shmatriz=(int **)matriz;

	llaved=ftok("./cliente.c",32);
	id_memd=shmget(llaved,sizeof(int[MAX_JUGADORES]),0777 | IPC_CREAT);
	if(id_memd==-1)
	{	printf("No consegui memoria!\n");return 1; }
	direcc=(int *)shmat(id_memd,(char *)0,0);
	if(!direcc)
	{	printf("No consegui compartir memoria!\n");return 1; }

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



	printf("Servidor de NkTron listo y esperando conexiones...\n");
	printf("Se jugaran %d rondas con nivel de dificultad %d\n",cant_niveles,dificultad);	
	printf("Jugaran %d jugadores, de los cuales %d seran bots\n\n",jugadores,bots);

	for(i=0;i<MAX_JUGADORES;i++)
	{	strcpy(vnombres[i],"NULL");
		puntajes[i]=0;
	}
	fdlisten=escuchar(9999);
	if(fdlisten==-1)
	{	printf ("No se pudo abrir puerto para escuchar\n");
		return -1;;
	}

	for(i=0;i<bots;i++)
	{	//crear proceso de bot
		llave=ftok("./cliente.c",40+i);
		sembot[i]=obtenersem(llave);
		iniciasem(sembot[i],0); //1=verde

		cont_clientes++;
		strcpy(vnombres[i],"<Bot>");
		if ( (j=fork())==0 )
		{	char param[5];
			sprintf(param,"%d",i);
   			char * args[] = { "bot", param , NULL };
			execv("bot",args);			
		}
	}

	while (cont_clientes<jugadores)
	{		
		fdcliente=aceptar(fdlisten);
		if(fdcliente==-1)
		{
			printf ("no se pudo accept()\n");
			return -1;
		}
		cont_clientes++;
		error=pthread_create (&clientes[cont_clientes-1],NULL,atender,(void *)&fdcliente);
		if (error)
			printf ("no se pudo crear el hilo %d",clientes[cont_clientes-1]);		
	}

	sleep(2);
	printf ("Comenzando la partida...\n");

	//enviar a todos el pakete I
	for(i=bots;i<jugadores;i++)
	{	packout.cabecera='I';
                packout.cantniv=cant_niveles;
                for(j=0;j<MAX_JUGADORES;j++)
                        strcpy(packout.nombre[j],vnombres[j]);
                packout.nrojugador=i+1;
                enviar (fdclientes[i],(void*)&packout,sizeof(packout));
	}
	sleep(2);

	for(ronda=1;ronda<cant_niveles+1;ronda++)
	{
		//enviar a todos el pakete N
		packout.cabecera='N';
		packout.cantniv=ronda;
 		//establecer posiciones y direcciones iniciales 
		switch (jugadores)
		{
			case 2:
				direcc[0]=2;x[0]=6;y[0]=6;
				direcc[1]=4;x[1]=70;y[1]=18;
				break;
			case 3:
				direcc[0]=2;x[0]=4;y[0]=4;
				direcc[1]=4;x[1]=76;y[1]=4;
				direcc[2]=4;x[2]=72;y[2]=20;
				break;
			case 4:
				direcc[0]=3;x[0]=37;y[0]=4;
				direcc[1]=4;x[1]=76;y[1]=12;
				direcc[2]=1;x[2]=37;y[2]=20;
				direcc[3]=2;x[3]=4;y[3]=12;
				break;
			case 5:
				direcc[0]=2;x[0]=4;y[0]=4;
				direcc[1]=4;x[1]=75;y[1]=4;
				direcc[2]=2;x[2]=4;y[2]=20;
				direcc[3]=4;x[3]=75;y[3]=20;
				direcc[4]=1;x[4]=37;y[4]=16;
				break;
			case 6:
				direcc[0]=2;x[0]=4;y[0]=4;
				direcc[1]=4;x[1]=75;y[1]=4;
				direcc[2]=2;x[2]=4;y[2]=20;
				direcc[3]=4;x[3]=75;y[3]=20;
				direcc[4]=2;x[4]=8;y[4]=12;
				direcc[5]=4;x[5]=71;y[5]=12;
		}
		llenardatos(&packout);
		for(i=bots;i<jugadores;i++)
		{ enviar(fdclientes[i],(void*)&packout,sizeof(packout));  }
		
		printf("Empieza ronda %d...\n",ronda);
		limpiamatriz();
		ganador=0;

		sleep(3);
	
		while(alguienvive())
		{
			for(i=0;i<bots;i++)
				semverde(sembot[i]);			

			if (dificultad==0)
				usleep(300000/ronda);
			else
				usleep(500000/dificultad);

			//cuando se cumple el tiempo
			procesarmovimientos();
			if(!alguienvive())
				break;
			fflush(stdout);			
			packout.cabecera='S';
			llenardatos(&packout);
			
			//for(i=0;i<jugadores;i++)
			//printf("%d: <%d,%d;%d> ",i,x[i],y[i],direcc[i]);
			//printf("\n");

			for(i=bots;i<jugadores;i++)
			{ enviar(fdclientes[i],(void*)&packout,sizeof(packout));  }

		}
		
		packout.cabecera='F';	//Fin de ronda
		packout.ganador=ganador;
		for(i=0;i<jugadores;i++)
			packout.puntajes[i]=puntajes[i];
                for(i=bots;i<jugadores;i++)
                { enviar(fdclientes[i],(void*)&packout,sizeof(packout));  }		
		
		sleep(5);
	}

	printf("FIN DE JUEGO\n");

	for (i=bots;i<jugadores;i++)	
		pthread_join (clientes[i], NULL );
	printf("Eliminando hilos...\n");fflush(stdout);
	terminar();

	return 0;
}


void* atender(void *parametro)
{ 
	int fdcliente_local=*(int *)parametro;
	int nro_jugador=cont_clientes;
	int seguir=1,j;
	struct paketeSC packout;
	struct paketeCS packin;
	fdclientes[nro_jugador-1]=fdcliente_local;

while(seguir)
{	packin.cabecera='X';
	if(recibir (fdcliente_local,(void*)&packin,sizeof(packin)))
		perror("recibir");
	switch(packin.cabecera)
	{
	case 'N': //recibe nombre		
		strcpy(vnombres[nro_jugador-1],packin.nombre);
		printf("Se conecto: %s\n", packin.nombre);fflush(stdout);
		break;
	case 'S': //recibe movimiento
		if(direcc[nro_jugador-1]!=-1)		
		
			if( !(   (direcc[nro_jugador-1]==1 && packin.direcc==3)||
				(direcc[nro_jugador-1]==2 && packin.direcc==4)||
				(direcc[nro_jugador-1]==3 && packin.direcc==1)||
				(direcc[nro_jugador-1]==4 && packin.direcc==2) ) )
					direcc[nro_jugador-1]=packin.direcc;
	}
}

}//main

		
void llenardatos(struct paketeSC *out)
{	int j;
	for(j=0;j<jugadores;j++)
	{	out->direcc[j]=direcc[j];
		out->x[j]=x[j];
		out->y[j]=y[j];
	}	
}

void procesarmovimientos()
{	int i,murio;
	for(i=0;i<jugadores;i++)
	{ 	murio=0;
		switch(direcc[i])
		{
			case 1: //arriba
				if(y[i]==0 || matriz[x[i]][y[i]-1]!=0)
					murio=1;
				else
				{ y[i]--;
				  matriz[x[i]][y[i]]=i+1;	
				}
			break;
                   	case 2: //derecha         
                      	 	if(x[i]==79 || matriz[x[i]+1][y[i]]!=0)                 
                            		murio=1;          
                     		else               
                		{ x[i]++;                 
                 		  matriz[x[i]][y[i]]=i+1;     
                           	}
			break;			
			case 3: //abajo
				if(y[i]==22 || matriz[x[i]][y[i]+1]!=0)
					murio=1;
				else
				{ y[i]++;
				  matriz[x[i]][y[i]]=i+1;
				}
			break;
			case 4: //izquierda
				if(x[i]==0 || matriz[x[i]-1][y[i]]!=0)
					murio=1;
				else
				{ x[i]--;
				  matriz[x[i]][y[i]]=i+1;
				}
			}//switch
			if(murio) 
                        { direcc[i]=-1; //murio
                          printf("Jugador %s perdio !\n",vnombres[i]);
                          borrarmatriz(i+1);
			  puntajes[i]+=(jugadores-alguienvive()-1);
			  if(alguienvive()==0)
				ganador=i;
			}
	}		
}


void terminar ()
{	int i;
        desconectar(fdlisten);
	for(i=bots;i<jugadores;i++)
		desconectar(fdclientes[i]);
	printf("Cierre del servidor!\n");fflush(stdout);
	shmctl (id_mem, IPC_RMID, (struct shmid_ds *)NULL);
	shmctl (id_memd, IPC_RMID, (struct shmid_ds *)NULL);
	shmctl (id_memdx, IPC_RMID, (struct shmid_ds *)NULL);
	shmctl (id_memdy, IPC_RMID, (struct shmid_ds *)NULL);
	printf("Matando procesos hijos...");fflush(stdout);
	for(i=0;i<bots;i++)
	{	kill(botpid[i],SIGKILL) ;
		wait(NULL);	}
	printf("Ok\nTerminando proceso...\n");fflush(stdout);
	raise(9);
}
void limpiamatriz()
{       int i,j;
      //  printf("MATRIZ LIMPIA: %ld\n",matriz);
	fflush(stdout);

	for(i=0;i<80;i++) 
                for(j=0;j<23;j++)
         //              {
	//			printf("Matriz[%d][%d]\n",i,j);fflush(stdout);
				 matriz[i][j]=0;
	//		}

}
int alguienvive()
{       int i,cont=0;
	for(i=0;i<jugadores;i++)
                if(direcc[i]!=-1)
                        cont++;
        return cont;    
}

void borrarmatriz(int valor)
{	int i,j;
        for(i=0;i<80;i++)
                for(j=0;j<23;j++)
                        if(matriz[i][j]==valor) 
				matriz[i][j]=0;
}


