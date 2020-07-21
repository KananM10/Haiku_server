#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <errno.h>
#include <math.h>
#include <time.h>

//-------------------------------------------------------------------------------
/*
 * function to create unique shared memory key
 */
int access_segment(){ 
    key_t shm_key = ftok ("/dev/null", 65);      // ftok() to create unique key

    int shm_id = shmget (shm_key, sizeof (int), 0644);
    if (shm_id < 0){        
    	printf("There is no existing shared memory segment\n");                   
        perror ("shmget: ");
        exit (1);
    }

    return shm_id;
}

//-------------------------------------------------------------------------------

int main(){
	//init the signals to send them later to the server
	int signals[2] = {SIGINT, SIGQUIT};

	srand(time(NULL));

	//creating shared memory to get the pid of the server
	int shm_id = access_segment();
	int *p = (int *) shmat (shm_id, NULL, 0);   
	int serverPid = *p;

	char c,h;
	int random;

	printf("\033[31m");
	printf("Sending signals...\n");
	printf("\033[0m");

	//sending signals to the server by randomly choosing them from the array
	for(int i = 0; i < 100; i++){
	if(i > 5){
		printf("You want to quit? (y/n): ");
		scanf("%c", &h); 
		scanf("%c", &c);
	}
	if(h == 'Y' || h == 'y') break;

	sleep(1);
	random = rand()%2;
	kill(serverPid, signals[random]);

	}

	kill(serverPid, SIGTSTP);

	printf("\033[31m");
	printf("\nTerminating...\n");
	printf("\033[0m");
	//removing shared memory and shared value
	shmdt(p);
    shmctl (shm_id, IPC_RMID, 0); 
	return 0;
}

//-------------------------------------------------------------------------------

