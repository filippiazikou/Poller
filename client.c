#include "poller.h"

int main(int argc, char *argv[])
{
	int port, sock, i;
	char buf[256];
	struct sockaddr_in server;
	struct sockaddr *serverptr = ( struct sockaddr *) &server;	
	struct hostent *rem;
	char epilogi;
	int input;
	int flag = 0;
	char readnum;

	if(argc != 3) {
		fprintf(stderr, "Wrong input..\n");
		fprintf(stderr, "Usage: prompt> client [hostname] [portnum]\n");
		exit( EXIT_FAILURE );
	}

  
	if( ( sock = socket ( PF_INET, SOCK_STREAM, 0) ) < 0) {
		perror("socket");
		exit(1);
	}
	
	/* Find server address */
	if (( rem = gethostbyname ( argv[1]) ) == NULL ) {
		herror("gethostbyname"); 
		exit (1);
	}
	
	port = atoi ( argv[2] ); 	/* Convert port number to integer */
	server.sin_family = PF_INET;	/* Internet domain */
	memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
	server.sin_port = htons ( port );
	

	if ( connect ( sock, serverptr, sizeof( server ) ) < 0) {
		perror("connect") ;
		exit(1);
	}
		
	printf("Connecting to port %d \n", port);
	
	for(;;) {
		if( read(sock, &input, sizeof(int)) < 0 )	/* getting remote input */
		{
			printf("read1\n");
			exit(1);
		}

		printf("client received %d\n", input);
		switch(input) {
			case QUIT:
				flag = 1;
				break;
			case MENU:
				menu( sock );
				break;
			case CRED_REQUESTS:
				cred(sock);
				break;
			case VOTE_SUBJECT:
				subject(sock);
				break;
			case VOTE_AGAIN:
				vote_again(sock);
				break;
			case NOT_VOTED:
				not_voted(sock);
				break;
			case RESULTS:
				results(sock);
				break;
			case VOTE_REGISTERED:
				registered(sock);
				break;
			default:
				flag = 1;
				
		}

		if( flag == 1)
			break;
	}


}

