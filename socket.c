/********************************************
      Funciones de socket de Nk-Tron
*********************************************
Archivo: socket.c
Descripcion del archivo:
Ofrece funciones de comunicacion por socket.
Es incluido en el proceso cliente y en el proceso servidor.
*/

#define COLA 20 // cant de conexiones creo
#define MAX_JUGADORES  6 // cantidad maxima de jugadores
struct sockaddr_in serveraddr, clientaddr;

struct paketeSC
{  char cabecera;       
        // I = inicio (cantniv,nombre,nrojugador) 
        // N  (ronda) = (cantniv, direcc[],x[],y[])  //cantniv es nro de ronda aca
        // S = (direcc[],x[],y[])  // si alguien perdio lo indica con direcc=-1
        // F = (ganador,puntajes[])
    int cantniv, x[6], y[6], direcc[6];
    char nombre[7][6];
    int nrojugador;
    int puntajes[6];
    int ganador;
};

struct paketeCS
{    char cabecera;
       // N = nombre
       // S = direcc
    char nombre[7];
    int direcc;
};

int conectar (char* host,int puerto)
{
	int fdsocket; //Descriptor del Socket
	struct sockaddr_in serveraddr;
	
	struct hostent *host_name;
	if ((host_name = gethostbyname(host)) == 0) //gethostbyname --> convierte "localhost" a "127.0.0.1"
	{
		printf("gethostbyname()");//perror("gethostbyname()");
		return -1;//exit(-1);
	}
	
	/*Se inicializa la estructura serveraddr */
	/* sin_family ---> Familia de protocolos
	   sin_port -----> Numero de puerto del servidor
	   sin_addr -----> Direccion IP numerica del servidor
	 */
	bzero(&serveraddr, sizeof(serveraddr)); //Se limpia la estructura
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_addr.s_addr = ((struct in_addr *)(host_name->h_addr))->s_addr;
	serveraddr.sin_port = htons(puerto);
	
	fdsocket = socket(AF_INET, SOCK_STREAM, 0);
	if(fdsocket == -1)
	{
		printf("socket()");//perror("socket()");
		return -1;//return -1; //exit(-1);
	}
	
	if(connect(fdsocket, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
	{	//printf("ERRconnect()\n");
		return -1; //exit(-1);
	}
	return fdsocket;
}
int enviar (int fdsocket,void *data,int tam)
{
	if(send(fdsocket, data, tam,0) == -1)
	{
		printf("send()");
		return -1;//return -1; //exit(-1);
	}
	return 0;
}

int recibir (int fdsocket,void *buffer,int tam)
{
	if(recv(fdsocket, buffer, tam, MSG_WAITALL) == -1)
	{
		printf("recv()");
		return -1;//return -1; //exit(-1);
	}
	return 0; 
}
int desconectar (int fdsocket)
{
	close(fdsocket);	
}

// servidor 

int escuchar (int puerto)
{
	int fdlisten;
	struct sockaddr_in serveraddr;
	/* Crea un Socket TCP */
	fdlisten = socket(AF_INET, SOCK_STREAM, 0);
	if(fdlisten == -1)
	{
		printf("socket()");
		return -1; //exit(-1);
	}
	
	bzero(&serveraddr, sizeof(serveraddr)); //Se limpia la estructura
	serveraddr.sin_family = AF_INET; //AF_INET --> Selecciona la pila TCP
	serveraddr.sin_addr.s_addr = INADDR_ANY; //INADDR_ANY --> permite IP de cualquier host
	serveraddr.sin_port = htons(puerto); //htons() --> convierte el entero a orden de bytes de red
	
	if(bind(fdlisten,(struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
	{	printf("bind()");
		return -1; //exit(-1);
	}
	
	if(listen(fdlisten,COLA) == -1)
	{
		printf("listen()");
		return -1; //exit(-1);
	}

	return fdlisten;
}

int aceptar (int fdlisten)
{
	//struct sockaddr_in clientaddr;
	int fdcliente; 
	socklen_t sin_size=sizeof(struct sockaddr_in);

	fdcliente = accept(fdlisten,(struct sockaddr *)&clientaddr, &sin_size);
	if(fdcliente == -1)
	{
		printf ("accept()");//perror("accept()");
		return -1;//exit(-1);
	}
	return fdcliente;
}


