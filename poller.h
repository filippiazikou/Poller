#ifndef _H_
#define _H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

#define YES 'y'
#define NO 'n'

#define TRUE 1
#define FALSE 2

/*defines for hash table*/
#define HASH_CAPACITY 512
#define HASH_OK -1
#define HASH_ERROR -2
#define FOUND 5
#define NOT_FOUND 6

#define QUIT 100
#define MENU 200
#define CRED_REQUESTS 300
#define	VOTE_AGAIN 400
#define NOT_VOTED 500
#define RESULTS 600
#define VOTE_SUBJECT 700
#define VOTE_REGISTERED 800

#define VOTE_BEGIN 900
#define POLL_RESULTS 1000

#define STATE_VOTE_CREDENTIALS 1100
#define STATE_RESULTS_CREDENTIALS 1200
#define STATE_VOTE_SELECTION 1300
#define STATE_INITIAL 1400


typedef struct {
	char *poll_log;
	char *poll_log_dat;
	char *poll_log_log;
	char *hot_issue;
	int counter_yes;
	int counter_no;
}file_info; 

file_info info;

typedef struct vote_struct{
	char username[20];
	char answer;	//YES or NO
	struct vote_struct *next;
} vote; 

typedef struct {
	int start ;
	int end ;
	int count ;
	int bufferSize;
	int *buffer;
} pool_t ;

int workerNumber; /*Number of Worker*/
double master_time;
	


volatile sig_atomic_t exitc;
pthread_mutex_t mtx ;
pthread_cond_t cond_nonempty ;
pthread_cond_t cond_nonfull ;
pool_t pool ;

pthread_mutex_t file ;	/*For Data File*/
pthread_mutex_t stat_file;	/*For the statistics File*/
pthread_mutex_t worker_id;	/*For worker number*/
pthread_mutex_t hash_mtx;	/*For hash functions*/

int sock_global;

/*Server functions*/
void handle_sigint(int signum) ;
void initialize ( pool_t * pool, int bufferSize );
void place ( pool_t * pool , int data );
int obtain( pool_t * pool ) ;
void errors(int newsock, char *msg);
int init_state(int newsock);
void quit();
int vote_cred(int newsock, vote *v);
void vote_selection(int newsock, vote *v) ;
void results_server(int newsock);
void *worker();



/*Client functions*/
void menu(int sock);
void cred(int sock);
void vote_again(int sock);
void not_voted(int sock) ;
void results(int sock);
void subject(int sock);
void registered(int sock);


/*hash table functions*/
int hash_init();
int hash_insert(vote *v);
int hash_scan(char *username);
int hash_function(char* value);
#endif