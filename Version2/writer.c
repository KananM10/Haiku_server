#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX 256

//------------------------------------------------------------------------------------

/*
 * structure for message queue 
 */
typedef struct message { 
    long mtype;  //message type
    char mtext[MAX]; //message text
}msg; 

//-------------------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------------------------

/*
 * function to write some text to message queue with category mentioned in the arguments
 */
int write_value( char* text, int category){
    msg msg;
    int id;
    
    if ( (id = create_queue () ) == -1 ) 
        return ( -1);
    else 
        printf ("Entering Queue with ID : %d \n" , id);

    msg.mtype = (long) category;
    strncpy(msg.mtext, text, MAX);
    if ( msgsnd ( id , &msg , strlen(msg.mtext) , 0) == -1 ) {
        perror (" msgsnd failed "); return ( -1) ;}
    else {
        printf ("Message:  %sWith category: %ld has been sent Successfully \n\n" , msg.mtext , msg.mtype );
        return (0) ;
    }
}

//-----------------------------------------------------------------------------------------------

/*
 * function to take lines from both japanese and western files and write them into message queue with 
 * appropriate category type
 */

void readlinesAndwriteToQueue()
{
	char *filename[] = {"./japanese.txt", "./western.txt"};
	char line[MAX];

	for(int i=0; i < 2; i++)// categories
	{

		for(int k=1; k < 4; k++){
			readline(filename[i], k, line);
			write_value(line, i+1);
		}
	}	
}

//-----------------------------------------------------------------------------------------------

int main(){
	readlinesAndwriteToQueue();
	return 0;
}


