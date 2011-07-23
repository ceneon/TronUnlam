all:
	gcc servidor.c -o servidor -lpthread
	gcc bot.c -o bot
	gcc cliente.c -o cliente -lpthread -lncurses
cliente:	cliente.c hilograficador.c socket.c 
	gcc cliente.c -o cliente -lpthread -lncurses

servidor:	servidor.c socket.c fcsemaforos.c bot.c
	gcc bot.c -o bot
	gcc servidor.c -o servidor -lpthread
