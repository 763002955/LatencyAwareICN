/*
 * host
 * queue:hosttoctrl,ctrltohost;hosttop1,p1tohost
 * thread:host-R1 1 or 2 ///  host-ctrl 1 or 2
 * choose index by random----have an index table 
 * 
 * 
 * 
 * */
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mqueue.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
using namespace std;
#define MAXMSG 2000//the stable number of mq_attr.mq_maxmsg. 300 is available.
#define MAXMSGCTOP 10//the queue from controller to process .
#define PERM 0766//the authority of m_queue.
#define MAX_TEXT 512  

char Index[10][2]={'a','A','b','B','c','C','d','D','e','E','f','F','g','G','h','H','i','I','j','J'};

struct interest_pkt  
{  
	int latency_requirment;
    char text[MAX_TEXT];  
};  
struct data_msg
{
	int msg_type;
	char text[MAX_TEXT];
};
char * randomIndex()
{
	char *randomMSG= new char[MAX_TEXT];
	int num;
	//choose a random index
	srand((unsigned)time(NULL));
	num=rand()%10;
    cout<<"chosse index "<<num<<endl;
    char str[MAX_TEXT];
    str[0]=Index[num][0];
    //cout<<"str"<<str<<endl;
    str[1]='\0';
    strcpy(randomMSG, str);
   // cout<<"text::"<<randomMSG<<endl;
    return randomMSG;
	
}


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
	mqd_t mqd_hosttop1, mqd_p1tohost, mqd_hosttoctrl, mqd_ctrltohost;
	int mq_return = 0;
	char hosttop1[] = "/hosttop1";
	char p1tohost[] = "/p1tohost";
	char hosttoctrl[] = "/hosttoctrl";
	char ctrltohost[] = "/ctrltohost";
	
    struct interest_pkt datatoctrl,*datator1;  
	struct data_msg *datamsg;
	
	//memset(data.text,'\0',sizeof(data.text));
	mqd_hosttop1 = mq_open(hosttop1, flags, PERM, &attr);
	//check_return(mqd_hosttop1, hosttop1, "mq_open");
	mqd_p1tohost = mq_open(p1tohost, flags, PERM, &attr);
	mqd_hosttoctrl = mq_open(hosttoctrl, flags, PERM, &attr);
	mqd_ctrltohost = mq_open(ctrltohost, flags, PERM, &attr);
	int running =1;

	//interest pkt to R1
	char buffer[2048];
	
	pid_t send,receive,sendtoctrl;  
	
	
	if((sendtoctrl = fork()) == 0)  
	{
		while(running)
		{	
			// send interest pkt
			char *str = new char[MAX_TEXT];
			str=randomIndex();
			cout << "str::" << *str <<endl;
			strcpy((datatoctrl).text , str);    
		//	cout<<"already random "<<endl;
			if(mq_send(mqd_hosttoctrl, (char *)&datatoctrl, 128, 0) == -1)  
			{  
				cout<<"msgsnd failed"<<endl;
				exit(EXIT_FAILURE);  
			}  
			cout<<"host : already send"<<(datatoctrl).text<<" to ctrl"<<endl;
			
			usleep(10000000);
		}
	}
	
    if((send = fork()) == 0)  
    {  
        while(running)
		{	
			//receive from the controller
			mq_return = mq_receive(mqd_ctrltohost, buffer, 2048, 0);
			if(mq_return == -1) {
				cout<<"receive error"<<endl;
				return -1;
			}
			datator1 = (struct interest_pkt*) buffer;
			//printf("receive : R1 -> host: %s\n",(*datamsg).text);  
			cout<<"ctrl back :"<<(*datator1).text<<" latency: "<<(*datator1).latency_requirment<<endl;
			
			// send interest pkt to r1
			if(mq_send(mqd_hosttop1, (char *)datator1, 128, 0) == -1)  
			{  
				cout<<"msgsnd failed"<<endl;
				exit(EXIT_FAILURE);  
			}  
			cout<<"already send"<<(*datator1).text<<" to r1 "<<endl;
		}
		
		//hosttop1
		mq_return = mq_close(mqd_hosttop1);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_close");
		mq_return = mq_unlink(hosttop1);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_unlink");
      //  exit(0); // 结束子进程  
    }  
	if((receive = fork()) == 0)  
	{
		cout<<"receive fork ok"<<endl;
		
		while(running)
		{
			//receive data back
			mq_return = mq_receive(mqd_p1tohost, buffer, 2048, 0);
			if(mq_return == -1) {
				cout<<"receive error"<<endl;
				return -1;
			}
			datamsg = (struct data_msg*) buffer;
			//printf("receive : R1 -> host: %s\n",(*datamsg).text);  
			cout<<"databack:"<<(*datamsg).text<<endl;
		}
		
		mq_return = mq_close(mqd_p1tohost);
		mq_return = mq_unlink(p1tohost);
	//	exit(0); // 结束子进程  
	}
	
	
	

	

	

//	exit(0);

}







