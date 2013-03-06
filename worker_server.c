#include "poller.h"

/* Thread function */
void *worker(){
	
	int workNum;
	int readnum, i;
	int newsock;
	int connectionsNum = 0;
	int state, end, choice;
	double t2;
	struct timeval tim;
	FILE *fd;	/*Statistics file*/
	vote *v;

	
	pthread_mutex_lock (& worker_id);
	workNum = workerNumber;
	workerNumber += 1;
	pthread_mutex_unlock (& worker_id);
	
	v = malloc(sizeof(vote));
		
	printf ("I am the newly created thread %ld with id %d\n", pthread_self (), workNum);
	while(1) {
		while ( pool.count > 0) {
			
			connectionsNum += 1;
			
			newsock = obtain (& pool );
			pthread_cond_signal (& cond_nonfull );
			usleep (500000) ;
			
			
			/*Write statistics!*/
			pthread_mutex_lock (& stat_file );
			fd = fopen(info.poll_log_dat, "r+");
			if (fd == NULL) {
				errors(newsock, "Error while openning file to insert statistics!");
				exit(1);
			}
	
			fseek(fd, 0, SEEK_END);
			
			gettimeofday(&tim, NULL);
			t2=tim.tv_sec+(tim.tv_usec/1000000.0);
			
			fprintf(fd, "Stat-req-arrival: %.6lf\nStat-req-dispatch: %.6lf\nStat-thread-id: %d\nStat-thread-count: %d\n\n\n", master_time, t2 - master_time, workNum, connectionsNum);
	
			fclose(fd);
	
			pthread_mutex_unlock (& stat_file );
			
			
			/*Begin Poll Πrocedure*/
			state = STATE_INITIAL;
			end = FALSE;
			for (;;) {
				switch(state) {
					
					case STATE_INITIAL:
						choice = init_state(newsock);
						switch(choice) {
							case VOTE_BEGIN:
								/*Send Credentials Request*/
								readnum = CRED_REQUESTS;
								if( write(newsock, &readnum, sizeof(int) ) < 0)
								{
									errors(newsock, "server_write");
									exit(1);
								}
								/*Change state*/
								state = STATE_VOTE_CREDENTIALS;
								break;
							case POLL_RESULTS:
								/*Send Credentials Request*/
								readnum = CRED_REQUESTS;
								if( write(newsock, &readnum, sizeof(int) ) < 0)
								{
									errors(newsock, "server_write");
									exit(1);
								}
								/*Change state*/
								state = STATE_RESULTS_CREDENTIALS;
								break;
							case QUIT:
								state = QUIT;
								break;
							default:
								state = QUIT;
								break;
						}
						break;
					case STATE_VOTE_CREDENTIALS:
						state = vote_cred(newsock, v);
						break;
					case STATE_RESULTS_CREDENTIALS:
						results_server(newsock);
						state = STATE_INITIAL;
						break;
					case STATE_VOTE_SELECTION:
						vote_selection(newsock, v);
						state = STATE_INITIAL;
						break;
					case QUIT:
						readnum = QUIT;
						end = TRUE;
						break;
				}
				if (end == TRUE) {
					if( write(newsock, &readnum, sizeof(int) ) < 0)
					{
						errors(newsock, "server_write");
						exit(1);
					}
					break;
				}
			}
				
			close(newsock);
			
		}
		if (exitc == 1)
			break;		
	}
	
	free(v);
	pthread_exit (NULL) ; 
}
