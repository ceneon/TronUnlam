/********************************************
      Hilo de graficacion de Nk-Tron
*********************************************
Archivo: hilograficador.c
Descripcion del archivo:
Codigo fuente del hilo de ejecucion del proceso cliente encargado de la salida por pantalla del juego.
Es incluido en el proceso cliente
*/

void *graficar()
{	int i,j;

	start_color();
        init_pair(5,COLOR_RED,COLOR_RED);
        init_pair(2,COLOR_GREEN,COLOR_GREEN);
        init_pair(6,COLOR_BLUE,COLOR_BLUE);
        init_pair(4,COLOR_MAGENTA,COLOR_MAGENTA);
        init_pair(1,COLOR_YELLOW,COLOR_YELLOW);
        init_pair(3,COLOR_CYAN,COLOR_CYAN);
	init_pair(10,COLOR_BLACK,COLOR_WHITE);
	init_pair(11,COLOR_BLACK,COLOR_BLACK);
	
	while(1)
	{
		pthread_mutex_lock(&mutx);

		//system("clear");
		move(0,0);

		attron(COLOR_PAIR(10));
		for(j=0;j<80;j++)
			mvaddch(0,j,32);
		move(0,0);
		printw("NkTron \t\tRonda %d de %d\t",ronda,rondas);
		printw("Ud. es jugador %d, %s   ",nrojugador,nombres[nrojugador-1]);
		attroff(COLOR_PAIR(10));		
		
		attron(COLOR_PAIR(nrojugador));
		printw("XXXXXXXX");
		attroff(COLOR_PAIR(nrojugador));		
		move(1,0);


		init_color(COLOR_BLACK,1000,0,0);
		init_pair(11,COLOR_BLACK,COLOR_BLACK);
	
		for(i=0;i<23;i++)  //es 22 pero hasta ncurses va a ser asi
		{	for(j=0;j<80;j++)   //es 80 pero hasta ncurses va a ser asi
			{  if(matriz[j][i]==0)
			   {	
				attron(COLOR_PAIR(11));
				mvaddch(i+1,j,32);     // j es x
				attroff(COLOR_PAIR(11));
			   }
			  else
			  {
				attron(COLOR_PAIR(matriz[j][i]));
				mvaddch(i+1,j,matriz[j][i]+48 );
				attroff(COLOR_PAIR(matriz[j][i]));	
			  }
			}
			//printf("\n");
		}
		init_color(COLOR_BLACK,0,0,0);
		init_pair(11,COLOR_BLACK,COLOR_BLACK);

		pthread_mutex_unlock(&mutvolver);
		refresh();	
	}
	
}
