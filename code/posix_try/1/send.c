#include <unistd.h>  
#include <stdlib.h>  
#include <stdio.h>  
#include <string.h>  
#include <sys/msg.h>  
#include <errno.h>  
  
#define MAX_TEXT 512  
struct msg_st  
{  
    long int msg_type;  
    char text[MAX_TEXT];  
};  
  
int main()  
{  
    int running = 1;  
    struct msg_st data;  
    char buffer[BUFSIZ];  
    int msgid = -1;  
    int len;  
  
    msgid = msgget((key_t)1234, 0666 | IPC_CREAT);  
    if(msgid == -1)  
    {  
        fprintf(stderr, "msgget failed with error: %d\n", errno);  
        exit(EXIT_FAILURE);  
    }  
  
    while(running)  
    {  
          
        printf("Enter data : ");  
        fgets(buffer, BUFSIZ, stdin);  
        data.msg_type = 5;      
        strcpy(data.text, buffer);  
        len = strlen(data.text);      
        if(msgsnd(msgid, (void*)&data, len-1, 0) == -1)  
        {  
            fprintf(stderr, "msgsnd failed\n");  
            exit(EXIT_FAILURE);  
        }  
          
        if(strncmp(buffer, "end", 3) == 0)  
            running = 0;  
        usleep(100000);  
    }  
    exit(EXIT_SUCCESS);  
}  
