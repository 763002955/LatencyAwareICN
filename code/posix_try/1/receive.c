#include <unistd.h>  
#include <stdlib.h>  
#include <stdio.h>  
#include <string.h>  
#include <errno.h>  
#include <sys/msg.h>  
  
struct msg_st  
{  
    long int msg_type;  
    char text[128];  
};  
  
int main()  
{  
    int running = 1;  
    int msgid = -1;  
    int len = 0;  
    long int msgtype = 5;   
    struct msg_st data;  
          
    msgid = msgget((key_t)1234, 0666 | IPC_CREAT);  
    if(-1 == msgid )  
    {  
        fprintf(stderr, "msgget failed with error: %d\n", errno);  
        exit(EXIT_FAILURE);  
    }  
      
    while(running)  
    {  
        memset(&data.text, 0, 128);  
        len = msgrcv(msgid, (void*)&data, 128, msgtype, 0);  
        if(-1 == len)  
        {  
            fprintf(stderr, "msgrcv failed with errno: %d\n", errno);  
            exit(EXIT_FAILURE);  
        }  
        printf("You wrote: %s\n",data.text);  
  
        if(0 == strncmp(data.text, "end", 3))  
            running = 0;  
    }  
  
    //remove message queue  
    if(-1 == msgctl(msgid, IPC_RMID, 0))  
    {  
        fprintf(stderr, "msgctl(IPC_RMID) failed\n");  
        exit(EXIT_FAILURE);  
    }  
    exit(EXIT_SUCCESS);  
}  
