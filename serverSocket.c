#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int MAX_CLIENTS;
int NUM_CONEXIONS;

void doServiceFork (int fd)
{
    int pid;
    pid = fork();
    if(pid < 0){
        perror ("Error al crear hijo\n");
        exit(1);
    }
    if(pid == 0){
        doService(fd);
        exit(0);
    }
    NUM_CONEXIONS++;
}

doService(int fd) {
int i = 0;
char buff[80];
char buff2[80];
int ret;
int socket_fd = (int) fd;

	ret = read(socket_fd, buff, sizeof(buff));
	while(ret > 0) {
		buff[ret]='\0';
		sprintf(buff2, "Server [%d] received: %s\n", getpid(), buff);
		write(1, buff2, strlen(buff2));
		ret = write(fd, "caracola ", 8);
		if (ret < 0) {
			perror ("Error writing to socket");
			exit(1);
		}
		ret = read(socket_fd, buff, sizeof(buff));
	}
	if (ret < 0) {
			perror ("Error reading from socket");

	}
	sprintf(buff2, "Server [%d] ends service\n", getpid());
	write(1, buff2, strlen(buff2));

}


main (int argc, char *argv[])
{
  int socketFD;
  int connectionFD;
  char buffer[80];
  int ret;
  int port;
  int type;

  MAX_CLIENTS = 100;
  NUM_CONEXIONS = 0;

  if (argc != 3)
    {
      strcpy (buffer, "Usage: ServerSocket PortNumber Type\n");
      write (2, buffer, strlen (buffer));
      exit (1);
    }

  port = atoi(argv[1]);
  socketFD = createServerSocket (port);
  if (socketFD < 0)
    {
      perror ("Error creating socket\n");
      exit (1);
    }

  type = atoi(argv[2]);
  if (type!=3) {
    while (1) {
	    connectionFD = acceptNewConnections (socketFD);
	    if (connectionFD < 0){
		    perror ("Error establishing connection \n");
		    deleteSocket(socketFD);
		    exit (1);
	    }

  	   if(type==1) doService(connectionFD); //Sequential
           else doServiceFork(connectionFD); //Concurrent unbounded
    }
  } else {
    //Bounded
    #ifdef DEBUG
    printf("Conexions: [%d/%d]\n",NUM_CONECTIONS, MAX_CLIENTS);
#endif

    if(NUM_CONEXIONS >= MAX_CLIENTS){
      #ifdef DEBUG
        printf("Màxim número de connexions\n");
      #endif
        while (waitpid(-1,NULL,0) > 0);
    }
    connectionFD = acceptNewConnections (socketFD);
    if (connectionFD < 0){
        perror ("Error establishing connection \n");
        deleteSocket(socketFD);
        exit (1);
    }
    doServiceFork(connectionFD);
  }
}
