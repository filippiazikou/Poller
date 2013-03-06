#include "poller.h"

void errors_client(int newsock, char *msg) {
	perror(msg);
	close(newsock);
	close(sock_global);
	shutdown(sock_global, SHUT_RDWR);	
}

void menu(int sock) {
	char x;
	int choice, output;
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
		
		putchar(x);
	}
	
	printf("\n");
	fflush(stdout);
	
	scanf("%d", &choice);
	while ( choice != 1 && choice != 2 && choice != 3 ) {
		printf("Wrong number! Please type 1, 2 or 3!\n");
		fflush(stdout);
		scanf("%d", &choice);
	}

	if(choice == 1) 
		output = VOTE_BEGIN;
	else if(choice == 2) 
		output = POLL_RESULTS;
	else if(choice == 3)
		output = QUIT;
	
	printf("client in menu sending %d\n", output);
	if (write( sock, &output, sizeof(int) ) < 0) {
		perror("read");
		close(sock);
		exit(1);
	}

}


void cred(int sock) {
	int i;
	char  username[20];
	int write_size;
	
	/*Ask name from user*/
	printf("Please type username: \n");
	fflush(stdout);
	
	for (;;) {
		if (getchar() == '\n') break; 
	}
	
	
	scanf("%s", username);
	write_size = strlen(username) + 1;
	/*Sending size to server*/
	if( write(sock, &write_size, sizeof(int) ) < 0)
	{
		errors_client(sock, "write_client");
		exit(1);
	}
	
	/* sending username */
	for(i = 0; i < write_size ; i++) {
		if( write(sock, &username[i], sizeof(char) ) < 0)
		{
			errors_client(sock, "write_client");
			exit(1);
		}
	}
}

void subject(int sock) {
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
		
		putchar(x);
	}
	
	printf("\n");
	fflush(stdout);
	
	do{
		printf("Please type your vote (y or n): \n");
		fflush(stdout);
		for (;;) {
			if (getchar() == '\n') break; 
		}
		
		scanf("%c", &vote);
		
	} while (vote != YES && vote != NO) ;

	/*Write vote to server*/
	if( write(sock, &vote, sizeof(char) ) < 0)
	{
		errors_client(sock, "write_client");
		exit(1);
	}
}

void vote_again(int sock) {
	printf("Sorry, you have already participated in the poll! You cannot vote again. Press enter to continue!\n");
	fflush(stdout);
	/*Flush stdin*/
	for (;;) {
		if (getchar() == '\n') break; 
	}
	getchar();
}

void not_voted(int sock) {
	printf("Sorry you cannot see the poll results. You have not yet voted on the Issue of the Day! Press Enter to continue! \n");
	fflush(stdout);
	
	/*Flush stdin*/
	for (;;) {
		if (getchar() == '\n') break; 
	}
	getchar();
}

void results(int sock) {
	char x;
	int read_size, i;
	
	
	
	/*Read the size of results*/
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
		putchar(x);
	}

	printf("\n");
	fflush(stdout);
	
	/*Wait for Enter*/
	getchar();
}


void registered(int sock) {
	printf("You have succesfully voted\n");
	fflush(stdout);
}
