#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <errno.h>


#define NTAB(t) sizeof(t)/sizeof(POSIX_SIGNAL)
volatile sig_atomic_t count;


//-------------------------------------------------------------------------------
typedef struct {
	int signo;
	char *catname;
}POSIX_SIGNAL; 


POSIX_SIGNAL tab[] = {
	{ SIGINT, "Japanese", },
	{ SIGQUIT, "Western", } 
};

//-------------------------------------------------------------------------------

/*
 * function to create unique shared memory key
 */
int create_segment(){ 
    key_t shm_key = ftok ("/dev/null", 65); //ftok() to create unique key

    int shm_id = shmget (shm_key, sizeof (int), 0644 | IPC_CREAT);
    if (shm_id < 0){                           
        perror ("shmget\n");
        exit (1);
    }

    return shm_id;
}

//-------------------------------------------------------------------------------

/*
 * Function that handles SIGINT and SIGQUIT signals  by printing the corrisponding haiku category
 */
void signal_handler(int sig){
	for(int i = 0; i < NTAB(tab); i++){
		
		if(tab[i].signo == sig){
			printf("Round: %d \t Signal: %s \t Haiku category: %s\n",count, sig == 2 ? "SIGINT " : "SIGQUIT", tab[i].catname);
			break;
		}
	}
	count++;
}

//-------------------------------------------------------------------------------

int main(){
	count = 0;
	pid_t serverPid =  getpid(); // getting the pid of the process to send it to the client
	
	// creating shared memory segment to seng pid to the client through it
	int shm_id = create_segment();
	int *p = (int *) shmat (shm_id, NULL, 0);   
    *p = 0;
    *p += serverPid;

    //sigaction structure to set the signal_handlers to signals
    struct sigaction sigact;

    sigemptyset( &sigact.sa_mask );
    sigact.sa_flags = 0;
    sigact.sa_handler = signal_handler;
    sigaction( SIGINT, &sigact, NULL );
    sigaction( SIGQUIT, &sigact, NULL );

   	
    //waiting for signals
    while(count < 100) {
    	pause();	
    }

	return 0;
}

//-------------------------------------------------------------------------------


