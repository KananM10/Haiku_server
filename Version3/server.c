#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX 256
#define NTAB(t) sizeof(t)/sizeof(POSIX_SIGNAL)


//-----------------------------------------------------------------------------------------------------

/*
 * structure for signal
 */
typedef struct {
	int signo;
	int catnum;
}POSIX_SIGNAL; 

//-----------------------------------------------------------------------------------------------------

/*
 * structure for message queue 
 */
typedef struct message { 
    long mtype;  //message type
    char mtext[MAX]; //message text
}msg; 

//------------------------------------------------------------------------------------------------------

POSIX_SIGNAL tab[] = {
	{ SIGINT, 1 },
	{ SIGQUIT, 2 } 
};

//------------------------------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------------------------------

/*
 * function to read a certain line from the file
 */
void readline(char *filename, int linenumber, char line[]){

    FILE* file = fopen(filename, "r");
    // char line[256];
    int i = 0;
    while (fgets(line, MAX, file)) {
        i++;
        if(i == linenumber ){
            break;  
        }
    }
    fclose(file);
}

//-----------------------------------------------------------------------------------------------------

/*
 * the function to create queue
 */
int create_queue(){
    key_t key; // variable to hold IPC key of message queue
    int id; // variable to hold message queue id
    key = ftok("/dev/null", 65); // converts a pathname and a project identifier to a (Sys_V) IPC-key (unique key)
    if (key == -1) 
        perror (" ftok fails ");
    
    //creates  message queue
    // and returns a message queue identifier associated with the value of the key argument
    id = msgget (key, IPC_CREAT | 0666);
    if (id == -1) 
        perror (" msgget fails ");
    return id;
}

//------------------------------------------------------------------------------------------------------

/*
 * function for accessing an already existing message queue
 */
int access_queue(){
    key_t key; // variable to hold IPC key of message queue
    int id; // variable to hold message queue id
    key = ftok ("/dev/null", 65); // converts a pathname and a project identifier to a (Sys_V) IPC-key (unique key)
    if (key == -1) 
        perror (" ftok fails ");
    
    //looks for a existing message queue
    // and returns a message queue identifier associated with the value of the key argument
    id = msgget (key, 0);
    if (id == -1) 
        perror (" msgget fails ");
    return id;
}

//--------------------------------------------------------------------------------------------------------

/*
 * function to write some text to message queue with category mentioned in the arguments
 */
int write_value(int id, char* text, int category){
    msg msg;

    msg.mtype = (long) category;
    strncpy(msg.mtext, text, MAX);
    if ( msgsnd ( id , &msg , strlen(msg.mtext) , 0) == -1 ) {
        perror (" msgsnd failed "); return ( -1) ;}
    else {
        // printf ("Message:  %sWith category: %ld has been sent Successfully \n\n" , msg.mtext , msg.mtype );
        return (0) ;
    }
}

//--------------------------------------------------------------------------------------------------------

/*
 * function to take lines from both japanese and western files and write them into message queue with 
 * appropriate category type
 */

void readlinesAndwriteToQueue(int id, int cat)
{
	char *filename[] = {"./japanese.txt", "./western.txt"};
	char line[MAX];

	for(int k=1; k < 9; k++){

        if(cat == 2 && k>6) break;
		readline(filename[cat-1], k, line);
		write_value(id, line, cat);
	}
	
}
//---------------------------------------------------------------------------------------------------------

/*
 * function to read messages from message queue with known type
 */
void read_value(int id, int type){
    msg m; 
    int n;

    printf ( "Accessing Queue with ID : %d \n" , id ); 

    printf("Received message: \n");   

    // receive a message m from a message queue with id = id
    if ( ( n = msgrcv ( id , &m , MAX , type, IPC_NOWAIT ) ) == -1 ){
        perror ( "msgrcv");
        readlinesAndwriteToQueue(id, type);
        printf("Loading messages to queue...\n");
        printf("Repeat the action\n\n"); 
        // exit(-1) ;
    } else {
        m.mtext[n] = '\0';
        printf ( "\t Category: %s\n\t Message: %s\n" , m.mtype == 1 ? "Japanese" : "Western", m.mtext ) ;
    }

}

//---------------------------------------------------------------------------------

// funtion to remove message queue after communication

void remove_queue(int id){
    // to destroy the message queue by taking the id of message queue
    int r = msgctl(id , IPC_RMID , NULL );
    if (r == -1) perror ("msgctl");
}

//--------------------------------------------------------------------------------------------------------


/*
 * Function that handles SIGINT and SIGQUIT signals  by printing the corrisponding haiku category
 */
void signal_handler(int sig){
    int category;

    int id = access_queue();
    if(id == -1)
        exit(0);


    for(int i = 0; i < NTAB(tab); i++){   
        if(tab[i].signo == sig){
            category = tab[i].catnum;
            break;
        }
    }

    if(sig == SIGTSTP){
        remove_queue(id);
        kill(getpid(), SIGKILL);
    }

    read_value(id, category);
}

//---------------------------------------------------------------------------------------------------------

int main(){

    int q_id;
	pid_t serverPid =  getpid(); // getting the pid of the process to send it to the client
	
	// creating shared memory segment to seng pid to the client through it
	int shm_id = create_segment();
	int *p = (int *) shmat (shm_id, NULL, 0);   
    *p = 0;
    *p += serverPid;

    if(q_id = create_queue() == -1)
        return (-1);

    //sigaction structure to set the signal_handlers to signals
    struct sigaction sigact;

    sigemptyset( &sigact.sa_mask );
    sigact.sa_flags = 0;
    sigact.sa_handler = signal_handler;
    sigaction( SIGINT, &sigact, NULL );
    sigaction( SIGQUIT, &sigact, NULL );
    sigaction( SIGTSTP, &sigact, NULL );

   	
    //waiting for signals
    while(1) {
    	pause();	
    }

	return 0;
}


//----------------------------------------------------------------------------------------------------------
