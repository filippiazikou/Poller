#include "poller.h"

int init_state(int newsock) {
	int readnum, i, choicenum, write_size;
	char menu[] = "What would you like to do?\nChoice 1: Vote on Issue of the Day\nChoice 2: See Results\nChoice 3: Quit\nEnter your choice: ";
	char temp;
	
	/*Sending input*/
	readnum = MENU;
	if (write(newsock, &readnum, sizeof(int)) < 0) { /* Get message */
		errors(newsock, "writeserver");
		exit(1);
	}
	
	/*Sending size of menu*/
	write_size = strlen(menu) + 1;
	if( write(newsock, &write_size, sizeof(int) ) < 0)
	{
		errors(newsock, "writeserver");
		exit(1);
	}
	/* Sending menu */
	for(i = 0; i < write_size ; i++) {
		if( write(newsock, &menu[i], sizeof(char) ) < 0)
		{
			errors(newsock, "writeserver");
			exit(1);
		}
	}
		
	/*Read the choice*/
	if( read(newsock, &choicenum, sizeof(int)) < 0) {
		errors(newsock, "server_read");
		exit(1);
	}
	
	return choicenum;
}


void quit() {
	printf("Quiting..\n");
	fflush(stdout);
}

int vote_cred(int newsock, vote *v) {
	int readnum, i, found, write_size, read_size;
	char x;
	vote temp_vote;
	
	printf("vote cred..\n");
	fflush(stdout);
	
	/*Read the size of username*/
	if( read(newsock, &read_size, sizeof(int) ) < 0)
	{
		errors(newsock, "read_server");
		exit(1);
	}
	/*Read the username*/
	for (i = 0 ; i < read_size ; i++) {
		if( read(newsock, &x, sizeof(char) ) < 0)
		{
			errors(newsock, "server_read");
			exit(1);
		}
		
		v->username[i] = x;
	}

	printf("username = %s\n", v->username);
	fflush(stdout);
	
	/*Lock mutex*/
	pthread_mutex_lock (& hash_mtx );
	/*Check if username exists to file*/
	found = hash_scan(v->username);
	/*Unlock mutex*/
	pthread_mutex_unlock (& hash_mtx );
	
	if (found == NOT_FOUND) {
		
		/*Send input*/
		readnum = VOTE_SUBJECT;
		if( write(newsock, &readnum, sizeof(int) ) < 0)
		{	
			errors(newsock, "server_write");
			exit(1);
		}
		
		
		
		/*Sending size of subject*/
		write_size = strlen(info.hot_issue) + 1;
		if( write(newsock, &write_size, sizeof(int) ) < 0)
		{
			errors(newsock, "writeserver");
			exit(1);
		}
		/* Sending vote subject */
		for(i = 0; i < write_size ; i++) {
			if( write(newsock, &info.hot_issue[i], sizeof(char) ) < 0)
			{	
				errors(newsock, "server_write");
				exit(1);
			}
		}
		
		return STATE_VOTE_SELECTION;
	}
	else if (found == FOUND) {

		/* send input */
		readnum = VOTE_AGAIN;
		if( write(newsock, &readnum, sizeof(int) ) < 0)
		{	
			errors(newsock, "server_write");
			exit(1);
		}
		
		return STATE_INITIAL;
	}
}

void vote_selection(int newsock, vote *v) {
	int readnum, i, found = NOT_FOUND;
	FILE *fd;
	
	/*Read vote*/
	if( read(newsock, &v->answer, sizeof(char) ) < 0)
	{	
		errors(newsock, "server_read");
		exit(1);
	}
	
	/*Check again if username exists to file*/
	/*lock_mutex*/
	pthread_mutex_lock (& hash_mtx );
	found = hash_scan(v->username);
	pthread_mutex_unlock (& hash_mtx );	
	
	if (found == FOUND) {
		readnum = VOTE_AGAIN ;
	}
	else if (found == NOT_FOUND) {
	
		/*Write to database*/
		pthread_mutex_lock (& file );
		fd = fopen(info.poll_log_log, "rb+");
		if (fd == NULL) {
			errors(newsock, "Error while openning file to insert vote!");
				exit(1);
	}
	
		fseek(fd, 0, SEEK_END);
	
		fwrite(v, sizeof(vote), 1, fd);
	
		fclose(fd);

		pthread_mutex_unlock (& file );
	
		/*Write to hash table*/
		pthread_mutex_lock (& hash_mtx );
		if (hash_insert(v) != HASH_OK) {
			errors(newsock, "Error in hash_insert");
			exit(1);
		}
		
		/*Increase counter*/
		if (v->answer == 'y') 
			info.counter_yes += 1;
		else if (v->answer == 'n') 
			info.counter_no += 1;
	
		pthread_mutex_unlock (& hash_mtx );
	
		usleep(100000);
	

	
		/* send input */
		readnum = VOTE_REGISTERED ;
	}
	
	if( write(newsock, &readnum, sizeof(int) ) < 0)
	{	
		errors(newsock, "server_write");
		exit(1);
	}
	
}

void results_server(int newsock) {
	vote v;
	int found, readnum, i, write_size, read_size, percent;
	char results[200] ;
	
	
	/*Read the size of username*/
	if( read(newsock, &read_size, sizeof(int) ) < 0)
	{
		errors(newsock, "read_server");
		exit(1);
	}
	/*Read the username*/
	for (i = 0 ; i < read_size ; i++) {
		if( read(newsock, &v.username[i], sizeof(char) ) < 0)
		{
			errors(newsock, "server_read");
			exit(1);
		}
	}
	
	printf("Results username %s\n", v.username);
	fflush(stdout);
	
	/*Lock mutex*/
	pthread_mutex_lock (& hash_mtx );
	/*Check if username exists to file*/
	found = hash_scan(v.username);
	
	/*Unlock mutex*/
	pthread_mutex_unlock (& hash_mtx );
	
	if (found == NOT_FOUND) {
		/* send input */
		readnum = NOT_VOTED ;
		if( write(newsock, &readnum, sizeof(int) ) < 0)
		{	
			errors(newsock, "server_write");
			exit(1);
		}
	}
	else if (found == FOUND) {
		/* send input */
		readnum = RESULTS ;
		if( write(newsock, &readnum, sizeof(int) ) < 0)
		{	
			errors(newsock, "server_write");
			exit(1);
		}
		/*Send poll results*/
		percent = 100 / ( info.counter_yes + info.counter_no );
		sprintf(results, "RESULTS:  %d %% of votes are 'yes' votes and %d %% 'no'!\n", info.counter_yes*percent, info.counter_no*percent);
		
		/*Sending size of results*/
		write_size = strlen(results) + 1;
		if( write(newsock, &write_size, sizeof(int) ) < 0)
		{
			errors(newsock, "writeserver");
			exit(1);
		}
		/* Sending vote results */
		for(i = 0; i < write_size ; i++) {
			if( write(newsock, &results[i], sizeof(char) ) < 0)
			{	
				errors(newsock, "server_write");
				exit(1);
			}
		}
		
	}
}
