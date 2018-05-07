/*
 * host
 * queue:hosttoctrl,ctrltohost;hosttop1,p1tohost
 * thread:host-R1 1 or 2 ///  host-ctrl 1 or 2
 * choose index by random----have an index table 
 * 
 * 
 * 
 * */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mqueue.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#define MAXMSG 2000//the stable number of mq_attr.mq_maxmsg. 300 is available.
#define MAXMSGCTOP 10//the queue from controller to process .
#define PERM 0766//the authority of m_queue.
#define MAX_TEXT 512  

struct msg_st  
{  
    long int msg_type;  
    char text[MAX_TEXT];  
};  


int main() {
	/*initialization about mqueue*/
	char proname[] = "host";
	

	struct mq_attr attr, attr_ctrl;
	//struct mq_attr q_attr;
	attr.mq_maxmsg = MAXMSG;//maximum is 382.
	attr.mq_msgsize = 2048;
	attr.mq_flags = 0;

	attr_ctrl.mq_maxmsg = MAXMSGCTOP;
	attr_ctrl.mq_msgsize = 2048;
	attr_ctrl.mq_flags = 0;

	int flags = O_CREAT | O_RDWR;
	int flags_ctrl = O_CREAT | O_RDWR | O_NONBLOCK;
	mqd_t mqd_send0top0, mqd_p0top1, mqd_p0top2;
	int mq_return = 0;
	char send0top0[] = "/send0top0";
	

	char msg[BUFSIZ];
    struct msg_st data;  

	mqd_send0top0 = mq_open(send0top0, flags, PERM, &attr);
	//check_return(mqd_send0top0, send0top0, "mq_open");
//	mqd_p0top1 = mq_open(p0top1, flags, PERM, &attr);

	int running =1;
	int len=0;
	//char buffer[2048];//receive
	//struct ndpi_iphdr * iph;
	//long long int i = 0;
	//printf();
	
	//interest pkt to R1
	
	while(running)
	{	
		printf("Enter data : ");  
		fgets(msg, BUFSIZ, stdin);  
		   
		strcpy(data.text, msg);  
		len = strlen(data.text);      
		if(mq_send(mqd_send0top0, (void *)&data, 128, 0) == -1)  
		{  
		    fprintf(stderr, "msgsnd failed\n");  
		    exit(EXIT_FAILURE);  
		}  
		 
		printf("already send:%s",data.text);
		if(strncmp(msg, "end", 3) == 0)  
		    running = 0;  
		usleep(100000);  
	}
	
	

	//send0top0
	mq_return = mq_close(mqd_send0top0);//returns 0 on success, or -1 on error.
	//check_return(mq_return, send0top0, "mq_close");
	mq_return = mq_unlink(send0top0);//returns 0 on success, or -1 on error.
	//check_return(mq_return, send0top0, "mq_unlink");

	

	exit(0);

}







