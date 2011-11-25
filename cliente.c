/********************************************
	Cliente de Nk-Tron
*********************************************
Archivo: cliente.c
Descripcion del archivo:
Define el proceso de cliente real del juego.
Debe ser ejecutado por cada jugador que quiera incorporarse al juego
*/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <ncurses.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/ioctl.h>
//#include <linux/kd.h>

//#include "beep.c"

/* Puerto a utilizar */
#define PUERTO 9998
#define EBUSY 16


#include "socket.c"

void limpiamatriz();
void borrajugador(int);
void snd(int,int);

pthread_mutex_t mutx,mutvolver;
int matriz[80][23];
int rondas=0,ronda=0,nrojugador=0,jugadores=0;
char nombre[7],nombres[7][MAX_JUGADORES];
int sonido;
//void *graficar();
#include "hilograficador.c"

int main (int argc,char* argv[])
{	struct paketeSC pakin;
	struct paketeCS pakout;

	int seguir=1,error,fdsocket,i,j,tecla,movimiento;
	int max,auxd,auxvecd[MAX_JUGADORES],auxvecdd[MAX_JUGADORES];
	char auxvecs[MAX_JUGADORES][7],auxs[7];
	int vive[MAX_JUGADORES];

	pthread_t hilograf;
	//pthread_mutex_lock(&mutvolver);
	pthread_mutex_lock(&mutx);
	
	if(argc<2)
	{	printf("Cliente de NkTron\nERROR: Falta host del servidor\n");
		printf("  Uso: cliente IP/HOST [-sinsonido]\n");
		return 1;
	} 
	if(argc==3 && !strcmp(argv[2],"-sinsonido"))
		sonido=0;
	else
		sonido=1;
		

	initscr();
	snd(500,300);
	snd(700,300);
	snd(900,300);

	printw("Cliente de NkTron\n\nIngrese su nombre: ");
	refresh();
	scanw("%7s",nombre);

	pakout.cabecera='N';
	strcpy(pakout.nombre,nombre);	
	fdsocket=conectar(argv[1],PUERTO);
	//conectarse
	if(fdsocket==-1)
	{	endwin();
		printf ("Cliente de NkTron\nERROR: No se pudo Conectar\n");
		return 1;
	}

	/* Envia datos al Servidor */
	if (enviar (fdsocket,(void*)&pakout, sizeof(pakout)))
		perror("enviar()");
	
	printw("Esperando que comience el juego...\n\n");
	refresh();
        error = pthread_create (&hilograf, NULL, graficar, NULL);
        if(error!=0)
             { printf("No pude crear hilo"); return 1;}	

while(seguir)
{	pakin.cabecera='X';
	/* Recibe datos del Servidor */
	if (recibir (fdsocket,(void*)&pakin,sizeof(pakin)))
		perror("recibir");
	
	raw(); // ??
	keypad(stdscr,TRUE);
	cbreak(); //no espera a newlines
	noecho();
	nodelay(stdscr,TRUE);


	tecla = getch();
	fflush(stdin);
	movimiento=0;
	switch(tecla)
	{	case KEY_UP:
			movimiento=1;
		break;
		case KEY_DOWN:
			movimiento=3;
		break;
		case KEY_LEFT:
			movimiento=4;
		break;
		case KEY_RIGHT:
			movimiento=2;
		//leer el escape
	}
	if(movimiento)
	{	pakout.cabecera='S';
		pakout.direcc=movimiento;	
		if (enviar (fdsocket,(void*)&pakout, sizeof(pakout)))
			printf("ERROR !");
		movimiento=0;
	}

	switch(pakin.cabecera)
	{
	case 'I':	//Pakete de inicio de juego
		rondas=pakin.cantniv;
		nrojugador=pakin.nrojugador;
		for(i=0;i<MAX_JUGADORES;i++)
		{	strcpy(nombres[i],pakin.nombre[i]);
			if(jugadores==0 && strcmp(nombres[i],"NULL")==0)
				jugadores=i;
		}
		if(jugadores==0)
			jugadores=MAX_JUGADORES;
		printw("Tu numero de jugador es: %d de %d\n",nrojugador,jugadores);
		printw("El juego tiene %d rondas\n\n",rondas);
		printw("Participantes del juego\n-----------------------\n");
		for(i=0;i<jugadores;i++)
			printw("%d - %-7s\n",i+1,nombres[i]);
		refresh();

		sleep(2);
		clear();
		
	break;
	case 'N':	//Pakete de nueva ronda
		limpiamatriz();
		ronda=pakin.cantniv;
		for(i=0;i<jugadores;i++)
		  vive[i]=1;
	// NO BREAK
 
	case 'S':	//Pakete de sincronizacion de matriz
		for(i=0;i<jugadores;i++)
		{	
		  if(pakin.direcc[i]!=-1)
		    matriz[pakin.x[i]][pakin.y[i]]=i+1;
		  else
		  {  
		    	if(vive[i]==1)
			{	vive[i]=0;
				borrajugador(i+1);
		    		if(i+1==nrojugador)
				{ snd(400,400);snd(300,400);snd(200,400);
				}
			}
		  }
		}		

		//Desbloquear al hilo graficador		
		pthread_mutex_unlock(&mutx);
		pthread_mutex_lock(&mutvolver);
	if(pakin.cabecera=='N')
	{
	  snd(550,800);usleep(1100000);snd(550,800);usleep(1100000);snd(550,800);
	}

	break;		
	case 'F':	//Pakete de fin de ronda
		clear();
		printw("Termino ronda %d\n-------------\n",ronda);
		printw("Gano: %s\n\n",nombres[pakin.ganador]);

		if(pakin.ganador==nrojugador)
		{ snd(500,300);snd(700,400);snd(900,550);

		}

		printw("\t\tJugador\tPuntaje\n");
	
   		for(i=0;i<jugadores;i++)
		{	auxvecd[i]=pakin.puntajes[i];
			auxvecdd[i]=i;
			strcpy(auxvecs[i],nombres[i]);
		}
   		for(i=0;i<jugadores;i++)
   		{  max=i;
        	   for(j=i+1;j<jugadores;j++)
          		if(auxvecd[j]>auxvecd[max])
             	   		max=j;
		   auxd=auxvecd[max];		  
		   auxvecd[max]=auxvecd[i];		
		   auxvecd[i]=auxd;
                   auxd=auxvecdd[max];     
                   auxvecdd[max]=auxvecdd[i];
                   auxvecdd[i]=auxd;
                   strcpy(auxs,auxvecs[max]);     
                   strcpy(auxvecs[max],auxvecs[i]);
                   strcpy(auxvecs[i],auxs);
		}

		init_pair(5,COLOR_RED,COLOR_RED);
        	init_pair(2,COLOR_GREEN,COLOR_GREEN);
        	init_pair(6,COLOR_BLUE,COLOR_BLUE);
        	init_pair(4,COLOR_MAGENTA,COLOR_MAGENTA);
        	init_pair(1,COLOR_YELLOW,COLOR_YELLOW);
        	init_pair(3,COLOR_CYAN,COLOR_CYAN);
		
		for(i=0;i<jugadores;i++)
		{ 	
			if(auxvecdd[i]+1==nrojugador)
				printw(" -->"); 
			printw("\t");
			attron(COLOR_PAIR(auxvecdd[i]+1));
				printw("XXXXXX");
			attroff(COLOR_PAIR(auxvecdd[i]+1));
			printw("\t%s\t%d\n",auxvecs[i],auxvecd[i]);
		}
		printw("\n\n\n\n");
		if (ronda==rondas)
		{	
			printw("\n\n\tGANO %s\n",auxvecs[0]);

			printw("\n\n\n\n\nPresione una tecla para salir...");
			desconectar(fdsocket);	
			seguir=0;
			refresh();
			getchar();
		}
		else
			refresh();
	}

}//while(seguir)

//delscreen();
endwin();

return 0;
}

void limpiamatriz()
{	int i,j;
	for(i=0;i<80;i++)
		for(j=0;j<23;j++)
			matriz[i][j]=0;
}

void borrajugador(int num)
{	int i,j;
	for(i=0;i<80;i++)
		for(j=0;j<23;j++)
			if(matriz[i][j]==num)
				matriz[i][j]=0;
}

void snd(int freq ,int length)
{	if(sonido==0)
		return;
	FILE *dest;
 	dest=stdout;
  	fprintf(dest,"\33[10;%d]\33[11;%d]\a\33[10]\33[11]", freq, length);
  	fflush(dest);
  	//if (length) usleep(length*1000L);
}


