/*
 * R4
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
  //  long int msg_type;  
    char text[MAX_TEXT];  
};  

struct data_msg
{
	//long int msg_type;
	char text[MAX_TEXT];
};

char* find_data(char *index)//index in data out
{
	
	//map later
	char *str=new char[MAX_TEXT];
    
	for(int i=0;i<10;i++)
	{
		str[0]=Index[i][0];
		if(strcmp(index,str)==0)
		{
			cout<<"find index "<< index << "__" << Index[i][1]<<endl;
			//strcpy(datamsg.data,Index[i][1]);
			//return datamsg;
			str[0]=Index[i][1];
			str[1]='\0';
			return str;
		}

	}
	return NULL;
	
}
int main() {
	/*initialization about mqueue*/
	char proname[] = "R4";
	

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
	mqd_t mqd_p3top4, mqd_p4top3, mqd_p2top4, mqd_p4top2;
	
	int mq_return = 0;

	struct interest_pkt *data;  
	struct data_msg *r4tor2,*datamsg;
	//*r4tor3;
	
	char p4top2[] = "/p4top2";
	char p2top4[] = "/p2top4";
	char p4top3[] = "/p4top3";
	char p3top4[] = "/p3top4";
	mqd_p3top4 = mq_open(p3top4, flags, PERM, &attr);
	mqd_p4top3 = mq_open(p4top3, flags, PERM, &attr);
	mqd_p2top4 = mq_open(p2top4, flags, PERM, &attr);
	mqd_p4top2 = mq_open(p4top2, flags, PERM, &attr);

	int running =1;
	int len=0;
	//char buffer[2048];//receive
	//struct ndpi_iphdr * iph;
	//long long int i = 0;
	//printf();
	
	//interest pkt to R1
	char buffer[2048];
	
	pid_t interest_q;
	
    if((interest_q = fork()) == 0)  
    {  
        while(running)
		{	
			
			mq_return = mq_receive(mqd_p2top4, buffer, 2048, 0);
			if(mq_return == -1) {
				cout<<"receive error"<<endl;
				return -1;
			}
			cout<<"begin receive"<<endl;
			data = (struct interest_pkt*) buffer;
			cout<<"receive interest R2->R4:" <<(*data).text<<endl;
			
	/*		cout<<"find data pkt::"<< (find_data((*data).text)) <<endl;
			
			r4tor2 = (struct data_msg*) buffer;
			strncpy((*r4tor2).text,find_data((*data).text), MAX_TEXT);
			
			//send back data pkt R1 -> host
			if(mq_send(mqd_p4top2, (char *)r4tor2, 128, 0) == -1)  
			{  
				cout<<"msgsnd back failed"<<endl;
				exit(EXIT_FAILURE);  
			}  */
			datamsg = (struct data_msg*) buffer;
			//strcpy(found,find_data((*datamsg).text));
			//search cs
			strcpy((*datamsg).text,find_data((*data).text));
			//send back data pkt R4->R2
			if(mq_send(mqd_p4top2, (char *)datamsg, 128, 0) == -1)  
			{  
				cout<<"msgsnd back failed"<<endl;
				exit(EXIT_FAILURE); 
			}
			cout<<"data: R4->R2  " << (*datamsg).text<<endl;
			
			
			/*
			mq_return = mq_receive(mqd_p3top4, buffer, 2048, 0);
			data = (struct msg_st*) buffer;
			printf("R1->R2: %s\n",(*data).text);  
			
			cout<<"find data pkt::"<< (find_data((*data).text)) <<endl;
			strcpy((*data).text,find_data((*data).text));
			//send back data pkt R1 -> host
			if(mq_send(mqd_p4top3, (char *)data, 128, 0) == -1)  
			{  
				cout<<"msgsnd back failed"<<endl;
				exit(EXIT_FAILURE);  
			} */ 
		}
		//p2top4
	mq_return = mq_close(mqd_p2top4);//returns 0 on success, or -1 on error.
	//check_return(mq_return, send0top0, "mq_close");
	mq_return = mq_unlink(p2top4);//returns 0 on success, or -1 on error.
	//check_return(mq_return, send0top0, "mq_unlink");
	
	//p4top2
	mq_return = mq_close(mqd_p4top2);//returns 0 on success, or -1 on error.
	//check_return(mq_return, send0top0, "mq_close");
	mq_return = mq_unlink(p4top2);//returns 0 on success, or -1 on error.
	//check_return(mq_return, send0top0, "mq_unlink");
		
	
	//p3top4
	mq_return = mq_close(mqd_p3top4);//returns 0 on success, or -1 on error.
	//check_return(mq_return, send0top0, "mq_close");
	mq_return = mq_unlink(p3top4);//returns 0 on success, or -1 on error.
	//check_return(mq_return, send0top0, "mq_unlink");
	
	//p4top3
	mq_return = mq_close(mqd_p4top3);//returns 0 on success, or -1 on error.
	//check_return(mq_return, send0top0, "mq_close");
	mq_return = mq_unlink(p4top3);//returns 0 on success, or -1 on error.
	//check_return(mq_return, send0top0, "mq_unlink");
	

		
    }  

	
	
	
	

	

	exit(0);

}








