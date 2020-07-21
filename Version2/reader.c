#include <stdio.h> 
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX 256
  
//-------------------------------------------------------------------------------

/*
 * structure for message queue 
 */
typedef struct message { 
    long mtype;  //message type
    char mtext[MAX]; //message text
}msg; 

//------------------------------------------------------------------------------

/*
 * function to access an already existing message queue
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

//-----------------------------------------------------------------------------------

/*
 * function to read messages from message queue with known type
 */
void read_value(int type){
    msg m; 
    int n, id;

    if ( ( id = access_queue () ) == -1) //accessing an already existing message queue
        exit (0) ;
    else 
        printf ( "\nAccessing Queue with ID : %d \n" , id ); 

    printf("Received message: \n");   

    // receive a message m from a message queue with id = id
    if ( ( n = msgrcv ( id , &m , MAX , type, IPC_NOWAIT ) ) == -1 ){
        perror ( "msgrcv"); 
        exit(-1) ;
    } else {
        m.mtext[n] = '\0';
        printf ( "\t Category: %s\n\t Message: %s" , m.mtype == 1 ? "Japanese" : "Western", m.mtext ) ;
    }

}

//-----------------------------------------------------------------------------------

int main(){
    int category;
    printf("1 for Japanese || 2 for Western\n");
    printf("Enter the category number you want to read the messages from message queue: ");
    while(1){
        scanf("%d", &category);
        if(category <= 2 && category > 0) break;
        else
            printf("The category should be either 1 or 2: ");
    }  
    read_value(category);
    return 0;
}

//-----------------------------------------------------------------------------------
