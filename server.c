#include "poller.h"


void handle_sigint(int signum) {
	exitc = 1;
	printf("Exit!\n");
	close(sock_global);
	shutdown(sock_global, SHUT_RDWR);
	signal(SIGINT, handle_sigint);
}


void initialize ( pool_t * pool, int bufferSize ) {
	pool -> start = 0;
	pool -> end = -1;
	pool -> count = 0;
	pool -> bufferSize = bufferSize;
	pool -> buffer = malloc (bufferSize*sizeof(int));
	if (pool -> buffer == NULL) {
		perror ("buffer malloc ");
		exit (1) ;
	}
	
}

void place ( pool_t * pool , int data ) {
	pthread_mutex_lock (& mtx );
	while (pool -> count >= pool->bufferSize ) {
		printf (" >> Found Buffer Full \n");
		pthread_cond_wait (& cond_nonfull , &mtx );
	}
	pool -> end = (pool -> end + 1) % pool->bufferSize ;
	pool -> buffer [pool -> end ] = data ;
	pool -> count ++;
	printf("place: %d\n", pool->count);
	pthread_mutex_unlock (& mtx );
}

int obtain( pool_t * pool ) {
	int data = 0;
	pthread_mutex_lock (& mtx );
	while (pool -> count <= 0) {
		printf (" >> Found Buffer Empty \n");
		pthread_cond_wait (& cond_nonempty , &mtx );
	}
	data = pool -> buffer [pool -> start ];
	pool -> start = (pool -> start + 1) % pool->bufferSize ;
	pool -> count --;
	pthread_mutex_unlock (& mtx );
	return data ;
}

void errors(int newsock, char *msg) {
	perror(msg);
	printf("\n");
	fflush(stdout);
	close(newsock);
	close(sock_global);
	shutdown(sock_global, SHUT_RDWR);	
}





int main(int argc, char *argv[]) {
	pthread_t *thr ;
	pthread_t cons , prod ;
	char r;
	int err;
	int port, sock, newsock;
	int i, numWorkerThreads, bufferSize, size, read;
	FILE *fd;
	(void) signal(SIGINT,handle_sigint);
	vote *v;
	struct timeval tim1;
	unsigned int serverlen, clientlen;			/* Server - client variables */
	struct sockaddr_in server, client;
	struct sockaddr *serverptr, *clientptr;
	struct hostent *rem;
	
	if ( argc < 6 )					/* At least 2 arguments */
	{
		fprintf(stderr, "usage: ./poller [portnum] [numWorkerThreads] [bufferSize] [hotIssueFileName] [poll-log] [\n");
		exit(0);
	}
	
	exitc = 0;
	
	port = atoi(argv[1]);	/* Port */
	numWorkerThreads = atoi(argv[2]);	/*Number of workers*/
	if (numWorkerThreads < 0) {
		fprintf(stderr, "Please type valid numWorkerThreads\n");
		exit(0);
	}
	bufferSize = atoi(argv[3]);
	if (bufferSize < 0) {
		fprintf(stderr, "Please type valid bufferSize\n");
		exit(0);
	}
	
	
	/*Initialize struct file_info*/
	
	/*HotIssueFilename*/
	fd = fopen(argv[4], "rb");
	if (fd == NULL) {
		fprintf(stderr, "The hotIssueFileName doen not exist!\n");
		exit(0);	
	}
	/*FileSize*/
	size = 0;
	while (!feof(fd)) {
		fread(&r, sizeof(char), 1, fd) ;
		size += 1;
	}
	info.hot_issue = malloc(sizeof(char)*size);
	fseek(fd, 0, SEEK_SET);
	i = 0;
	while (!feof(fd)) {
		fread(&info.hot_issue[i], sizeof(char), 1, fd);
		i += 1;
	}
	
	
	fclose(fd);
	
	/*poll-log*/
	info.poll_log = malloc((strlen(argv[5]) +1) * sizeof(char));
	strcpy(info.poll_log, argv[5]);
	/*data - Statistics*/
	info.poll_log_dat = malloc((strlen(argv[5]) +5) * sizeof(char));
	sprintf(info.poll_log_dat, "%s.dat", argv[5]);
	gettimeofday(&tim1, NULL);
	master_time=tim1.tv_sec+(tim1.tv_usec/1000000.0);

	/*Create the file doesn' t exist*/
	if (fopen(info.poll_log_dat, "rb") == NULL) {
		if (fopen(info.poll_log_dat, "wb") == NULL) {
			fprintf(stderr, "Error while creating poll_log_dat!\n");
			exit(0);
		}
	}
	/*log*/
	info.poll_log_log = malloc((strlen(argv[5]) +5) * sizeof(char));
	sprintf(info.poll_log_log, "%s.log", argv[5]);
	/*Create the file doesn' t exist*/
	if (fopen(info.poll_log_log, "rb") == NULL) {
		if (fopen(info.poll_log_log, "wb") == NULL) {
			fprintf(stderr, "Error while creating poll_log_log!\n");
			exit(0);
		}
	}
	
	/*Initialize hash table*/
	if (hash_init() != HASH_OK) {
		fprintf(stderr, "Error while initializing hash table!\n");
		exit(0);
	}
	
	
	/*Insert to Hash Table the votes*/
	info.counter_yes = 0;
	info.counter_no = 0;
	
	fd = fopen(info.poll_log_log, "rb");
	if ( fd == NULL) {
		fprintf(stderr, "Error while opening poll_log_log!\n");
		exit(0);
	}
	
	pthread_mutex_init (& hash_mtx , 0);
	
	v = malloc(sizeof(vote));
	
	while (!feof(fd)) {
		fread(v, sizeof(vote), 1, fd);
		if (hash_insert(v) != HASH_OK) {
			fprintf(stderr, "Error while inserting to hash table!\n");
			exit(0);
		}
		if (v->answer == 'y') info.counter_yes += 1;
		else if (v->answer == 'n') info.counter_no += 1;
	}
	
	free(v);
	fclose(fd);
	
	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) { /* Create socket */
		perror("socket");
		exit(1);
	}
	port = atoi(argv[1]); /* Convert port number to integer */
	server.sin_family = PF_INET; /* Internet domain */
	server.sin_addr.s_addr = htonl(INADDR_ANY); /* My Internet address */
	server.sin_port = htons(port); /* The given port */
	serverptr = (struct sockaddr *) &server;
	serverlen = sizeof(server);
	
	clientptr = (struct sockaddr *) &client;
	clientlen = sizeof (client);
	
	sock_global = sock;
	
	if (bind(sock, serverptr, serverlen) < 0) {/* Bind socket to address */
		perror("bind"); exit(1); 
	}
	if (listen(sock, 5) < 0) { /* Listen for connections */
		perror("listen"); exit(1); 
	}
	
	printf("Listening for connections to port %d\n", port);
	
	
	/*Use of buffer*/
	initialize (& pool, bufferSize );
	pthread_mutex_init (& mtx , 0);
	pthread_cond_init (& cond_nonempty , 0);
	pthread_cond_init (& cond_nonfull , 0);
	
	/*Mutexes for poll_log_dat and poll_log_log files*/
	pthread_mutex_init (& file , 0);
	pthread_mutex_init (& stat_file , 0);
	
	/*Threads Create*/
	thr = malloc(sizeof(pthread_t) * numWorkerThreads);
	if (thr == NULL) {
		perror ("malloc ");
		exit (1) ;
	}
	
	pthread_mutex_init (& worker_id , 0);
	workerNumber = 0;
	for (i = 0 ; i < numWorkerThreads ; i++) {
		if (err = pthread_create (& thr[i] , NULL , worker , NULL)) { /* New thread */
			perror (" pthread_create " );
			exit (1) ;
		}
		
	}


	while(1) {
		
		if ((newsock = accept(sock, clientptr, &clientlen)) < 0) {
			perror("accept"); 
			break;
		} /* Accept connection */
// 		if ((rem = gethostbyaddr((char *) &client.sin_addr.s_addr, sizeof client.sin_addr.s_addr, /* Find client's address */ client.sin_family)) == NULL) {
// 			perror("gethostbyaddr"); exit(1); 
// 		}
		printf("Accepted connection from %d\n", newsock);
	
		/*producer*/
		place (&pool , newsock );
		printf (" producer : %d\n", newsock );
		pthread_cond_signal (& cond_nonempty );
		usleep (0) ;
		
	}
	

	
	for (i = 0 ; i < numWorkerThreads ; i++) {
// 		printf("server ok %d\n", i);
// 		fflush(stdout);
		if (err = pthread_join (*( thr +i), NULL )) {
			/* Wait for thread termination */
			perror (" pthread_join ");
			exit (1) ;
		}
	}

	printf("vgike\n");
	
	/*Write vote counters in poll_log_dat file*/
	if ( (fd = fopen(info.poll_log_dat, "r+") ) == NULL) {
		fprintf(stderr, "Error while opening data file to write vote counters!\n");
		exit (1);
	}
	
	fseek(fd, 0, SEEK_END);
	
	
	fprintf(fd, "~~~RESULTS:  %d has voted 'yes' and %d 'no'!~~~\n\n\n", info.counter_yes, info.counter_no);
	
	fclose(fd);
	


	pthread_cond_destroy (& cond_nonempty );
	pthread_cond_destroy (& cond_nonfull );
	pthread_mutex_destroy (& mtx );
	pthread_mutex_destroy (& hash_mtx );
	pthread_mutex_destroy (& worker_id );
	pthread_mutex_destroy (& file );
	pthread_mutex_destroy (& stat_file );
	//hash_free();
	free(pool.buffer);
	printf("exit...\n");
	pthread_exit (NULL );
}