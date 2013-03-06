#include "poller.h"

/*Info for workers*/
char voteType;
char *serverName;
int portNumber, numVotes;

void menu3(int sock, int choice) {
	char x;
	int output;
	int read_size, i;
	
	
	/*Read the size of menu*/
	if( read(sock, &read_size, sizeof(int) ) < 0)
	{
		errors_client(sock, "read_client");
		exit(1);
	}
	
	for (i = 0 ; i < read_size ; i++) {
		if( read(sock, &x, sizeof(char) ) < 0)
		{
			perror("read");
			close(sock);
			exit(1);
		}
	}
	
	if(choice == 1) 
		output = VOTE_BEGIN;
	else if(choice == 2) 
		output = POLL_RESULTS;
	else if(choice == 3)
		output = QUIT;
	
	/*Send VOTE-BEGIN*/

	if (write( sock, &output, sizeof(int) ) < 0) {
		perror("read");
		close(sock);
		exit(1);
	}

}

void vote_again3(int sock) {
	printf("Sorry, you have already participated in the poll! You cannot vote again. Press enter to continue!\n");
	fflush(stdout);
}

void registered3(int sock) {
	printf("You have succesfully voted\n");
	fflush(stdout);
}


void subject3(int sock) {
	char x;
	char vote;
	int read_size, i;
	
	
	/*Read the size of subject*/
	if( read(sock, &read_size, sizeof(int) ) < 0)
	{
		errors_client(sock, "read_client");
		exit(1);
	}
	
	for (i = 0 ; i < read_size ; i++) {
		if( read(sock, &x, sizeof(char) ) < 0)
		{
			errors_client(sock, "read_client");
			exit(1);
		}
	}
	
	/*Write vote to server*/
	if( write(sock, &voteType, sizeof(char) ) < 0)
	{
		errors_client(sock, "write_client");
		exit(1);
	}
}

void cred3(int sock) {
	int i, size_name;
	char  username[20];
	int write_size;
	
	/*Random Username Length*/
	size_name = rand() % 10 + 5;
	
	/*Random Username*/
	for (i = 0 ; i < size_name - 1 ; i++)
		username[i] = rand() %  ('z' - 'a')+ 'a';
	username[size_name - 1] = '\0'; 

	write_size = strlen(username) + 1;
	/*Sending size to server*/
	if( write(sock, &write_size, sizeof(int) ) < 0)
	{
		errors_client(sock, "write_client");
		exit(1);
	}
	
	/*credentials response*/
	/* sending username */
	for(i = 0; i < write_size ; i++) {
		if( write(sock, &username[i], sizeof(char) ) < 0)
		{
			errors_client(sock, "write_client");
			exit(1);
		}
	}
}


void *client() {
	
	int sock;
	struct sockaddr_in server;
	struct sockaddr *serverptr = ( struct sockaddr *) &server;	
	struct hostent *rem;
	
	int input, choice;
	
	if( ( sock = socket ( PF_INET, SOCK_STREAM, 0) ) < 0) {
		perror("socket");
		exit(1);
	}
	
	/* Find server address */
	if (( rem = gethostbyname ( serverName) ) == NULL ) {
		herror("gethostbyname"); 
		exit (1);
	}
	
	server.sin_family = PF_INET;	/* Internet domain */
	memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
	server.sin_port = htons ( portNumber );
	
	if ( connect ( sock, serverptr, sizeof( server ) ) < 0) {
		perror("connect") ;
		exit(1);
	}
		
	printf("Connecting to port %d \n", portNumber);
	
	
	/*Read Menu*/
	if( read(sock, &input, sizeof(int)) < 0 )	/* getting remote input */
	{
		printf("read1\n");
		exit(1);
	}
	
	/*Wait Menu*/
	choice = 1;
	menu3(sock, choice);
	
	
	/*Read Credentials Request*/
	if( read(sock, &input, sizeof(int)) < 0 )	/* getting remote input */
	{
		printf("read1\n");
		exit(1);
	}
	/*Credentials request*/
	cred3(sock);
	
	
	if( read(sock, &input, sizeof(int)) < 0 )	/* getting remote input */
	{
		printf("read1\n");
		exit(1);
	}
	/*Vote subject*/
	if (input == VOTE_SUBJECT) {
		subject3(sock);
		/*Wait Register*/
		if( read(sock, &input, sizeof(int)) < 0 )	/* getting remote input */
		{
			printf("read1\n");
			exit(1);
		}
		registered3(sock);
	}
	else if (input == VOTE_AGAIN) {
		vote_again3(sock);
	}
	
	
	/*Read Menu*/
	if( read(sock, &input, sizeof(int)) < 0 )	/* getting remote input */
	{
		printf("read1\n");
		exit(1);
	}
	/*QUIT*/
	choice = 3;
	menu3(sock, choice);
	
	
	/*Read Quit*/
	if( read(sock, &input, sizeof(int)) < 0 )	/* getting remote input */
	{
		printf("read1\n");
		exit(1);
	}
	
	close(sock);
	
	pthread_exit (NULL) ; 
	
	
}

int main(int argc, char **argv) {
	int i;
	int err;
	
	pthread_t *thr ;
	
	srand(time (NULL) );
	
	if(argc != 5) {
		fprintf(stderr, "Wrong input..\n");
		fprintf(stderr, "Usage: prompt> ./pollSwayer [servername] [portnum] [numVotes] [voteType] (y or n) \n");
		exit( EXIT_FAILURE );
	}
	
	/*Server Name*/
	serverName = malloc( (strlen(argv[1]) + 1) * sizeof(char));
	strcpy(serverName, argv[1]);
	
	/*Port Number*/
	portNumber = atoi(argv[2]);
	
	/*Number of Votes*/
	numVotes = atoi (argv[3]);
	if (numVotes < 0) {
		fprintf(stderr, "Please type valid number of votes!\n");
		exit(0);
	}
	
	/*Type of Vote*/
	voteType = argv[4][0];
	if (voteType != YES && voteType != NO) {
		printf("Please type 'y' for vote 'yes' or 'n' for vote 'no'\n");
		exit(EXIT_FAILURE);
	}
	
	/*Create Threads*/
	thr = malloc(sizeof(pthread_t) * numVotes);
	if (thr == NULL) {
		perror ("malloc ");
		exit (1) ;
	}
	
	for (i = 0 ; i < numVotes ; i++) {
		if (err = pthread_create (& thr[i] , NULL , client , NULL)) { /* New thread */
			perror (" pthread_create " );
			exit (1) ;
		}
	}
	
	
	for (i = 0 ; i < numVotes ; i++) {
		if (err = pthread_join (*( thr +i), NULL )) {
			/* Wait for thread termination */
			perror (" pthread_join ");
			exit (1) ;
		}
	}

	free(serverName);

	pthread_exit (NULL );
}

