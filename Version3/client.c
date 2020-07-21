#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <errno.h>
#include <math.h>

//----------------------------------------------------------------------------------------------------

/*
 * function to create unique shared memory key
 */

int access_segment(){ 
    key_t shm_key = ftok ("/dev/null", 65);      // ftok() to create unique key

    int shm_id = shmget (shm_key, sizeof (int), 0644);
    if (shm_id < 0){  
        printf("\nThere is no existing shared memory segment\n");                         
        perror ("shmget: ");
        exit (1);
    }

    return shm_id;
}

//-----------------------------------------------------------------------------------------------------

/*
 * function to handle the signals SIGINT, SIGQUIT, SIGTSTP
 */

void signal_handler(int sig){

    int shm_id = access_segment();
    int *p = (int *) shmat (shm_id, NULL, 0);   
    int serverPid = *p;

    kill(serverPid, sig);

    pid_t pid = getpid();
    if(sig == SIGTSTP) {
        //removing shared memory and shared value
        shmdt(p);
        shmctl (shm_id, IPC_RMID, 0); 
        // printf("%d\n", pid);
        kill(pid, SIGKILL);
    }
}

//-----------------------------------------------------------------------------------------------------

int main(){

    //sigaction structure to set the signal_handlers to signals
    struct sigaction sigact;

    sigemptyset( &sigact.sa_mask );
    sigact.sa_flags = 0;
    sigact.sa_handler = signal_handler;
    sigaction( SIGINT, &sigact, NULL );
    sigaction( SIGQUIT, &sigact, NULL );
    sigaction( SIGTSTP, &sigact, NULL );


    printf("Send SIGINT (CTRL + C) signal to read Japanese haikus\nSend SIGQUIT (CTRL + \\) signal to read Western haikus.\n");
    printf("To quit send SIGTSTP (CTRL  + Z ) signal\n");

    int shm_id = access_segment();
    //sending signals to the server by randomly choosing them from the array
    while(1){
        pause();
    }

    return 0;
}

//-------------------------------------------------------------------------------


