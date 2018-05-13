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
#include <sys/time.h>  
using namespace std;
#define MAXMSG 200//the stable number of mq_attr.mq_maxmsg. 300 is available.
#define MAXMSGCTOP 10//the queue from controller to process .
#define PERM 0766//the authority of m_queue.
#define MAX_TEXT 512 
#define DATANUM 26
string interest[DATANUM] = {
				"a","b","c","d","e","f","g","h","i","j","k","l","m",
				"n","o","p","q","r","s","t","u","v","w","x","y","z"	
				};
				int flags = O_CREAT | O_RDWR;
	int flags_ctrl = O_CREAT | O_RDWR | O_NONBLOCK;
	mqd_t mqd_hosttop1, mqd_p1tohost, mqd_hosttoctrl, mqd_ctrltohost, mqd_latencyHostToR1;
char hosttop1[] = "/hosttop1";
	char p1tohost[] = "/p1tohost";
	char hosttoctrl[] = "/hosttoctrl";
	char ctrltohost[] = "/ctrltohost";
	char latencyHostToR1[] = "/latencyHostToR1";
	struct timeval t_start,t_end; 
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

struct LatencyTestPKT
{
	int route;//(host->)r1->r2->r4 / (host->)r1->r3->r4 标志R2还是R3
	long  start_sec,start_usec;
	unsigned long duration[3];// host -> r1  r1 -> r2/3   r2/3 -> r4
};
struct interest_pkt datatoctrl,*datator1;  
struct data_msg *datamsg;
struct LatencyTestPKT latencyTest;
char buffer_ctrl[2048],buffer_R1[2048];
int mq_return = 0;
int msg_no = 0;
char * randomIndex()
{
	char *randomMSG= new char[MAX_TEXT];
	int num;
	//choose a random index
	srand((unsigned)time(NULL));
	num=rand()%26;
	num=20;
    string str;
    str = interest[num];
   int len = str.length();
		randomMSG = (char *)malloc((len+1)*sizeof(char));
		str.copy(randomMSG,len,0);
		cout<<"chosse interest: "<<randomMSG<<endl;
		return randomMSG;
   
}

void *sendtoctrl(void *ptr)
{
	while(1)
		{	
			// send interest pkt
			char *str = new char[MAX_TEXT];
			str=randomIndex();
			cout << "str::" << *str <<endl;
			strcpy((datatoctrl).text , str);   
			datatoctrl.latency_requirment = 50;
			if(mq_send(mqd_hosttoctrl, (char *)&datatoctrl, 128, 0) == -1)  
			{  
				cout<<"host to ctrl msgsnd failed"<<strerror(errno)<<"  "<< errno<<endl;
				exit(EXIT_FAILURE);  
			}  
			cout<<"host : already send "<<(datatoctrl).text<<" to ctrl"<<endl;
			
			usleep(20000000);
		}
		//hosttop1
		mq_return = mq_close(mqd_hosttop1);
		mq_return = mq_unlink(hosttop1);
}
void *send(void *ptr)  
    {  
        while(1)
		{	
			//receive from the controller
			if((mq_receive(mqd_ctrltohost, buffer_ctrl, 2048, 0)) == -1) {
				cout<<"receive error"<<endl;
//				return -1;
			}
			msg_no++;
			datator1 = (struct interest_pkt*) buffer_ctrl;
			(*datator1).latency_requirment = msg_no;
			//printf("receive : R1 -> host: %s\n",(*datamsg).text);  
			cout<<"host receive ctrl back :"<<(*datator1).text<<" latency: "<<(*datator1).latency_requirment<<endl;
			if(strcmp((*datator1).text,"no")==0)
				cout<<"find " <<(*datator1).text<< " failed!"<<endl;
			else
			{
				// send interest pkt to r1
				if(mq_send(mqd_hosttop1, (char *)datator1, 128, 0) == -1)  
				{  
					cout<<"host to p1 msgsnd failed"<<strerror(errno)<<"  "<< errno<<endl;
					exit(EXIT_FAILURE);  
				}  
				cout<<"host already send"<<(*datator1).text<<" to r1 "<<endl;
			}
		}
		
		//hosttop1
		mq_return = mq_close(mqd_hosttop1);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_close");
		mq_return = mq_unlink(hosttop1);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_unlink");
      //  exit(0); // 结束子进程  
    }  
    
void  *receive(void *ptr)
	{
		cout<<"receive fork ok"<<endl;
		
		while(1)
		{
			//receive data back
			if(mq_receive(mqd_p1tohost, buffer_R1, 2048, 0) == -1) {
				cout<<"receive error"<<endl;
//				return -1;
			}
			datamsg = (struct data_msg*) buffer_R1;
			//printf("receive : R1 -> host: %s\n",(*datamsg).text);  
			cout<<"DDDDDataback:"<<(*datamsg).text<<endl;
		}
		
		mq_return = mq_close(mqd_p1tohost);
		mq_return = mq_unlink(p1tohost);
	//	exit(0); // 结束子进程  
	}

void  *latency_test(void *ptr)
	{
		
		int routeChoose = 2;
		while(1)
		{
			//cout<<"latency fork ok"<<endl;
			//prepare latency pkt
			//route
			
		//	if(routeChoose %2 == 0)
				(latencyTest).route = 2;//R2
		//	else
	//			(latencyTest).route = 3;//R3
			//cout<<"route ok"<<endl;
			//start time
			//(*latencyTest).start = (*latencyTest).ends = clock();
			
			gettimeofday(&t_start, NULL); 
			
			(latencyTest).start_sec = t_start.tv_sec;
			(latencyTest).start_usec = t_start.tv_usec;
			//cout<<t_start.tv_sec<<"  "<<t_start.tv_usec<<endl;
			//cout<<"start_Sec ok"<<endl;
			if(mq_send(mqd_latencyHostToR1, (char *)&latencyTest, 128, 0) == -1)  
			{  
					cout<<"msgsnd failed"<<strerror(errno)<<"  "<< errno<<endl;
					exit(EXIT_FAILURE);  
			}  
			//<<"start time : "<<(*latencyTest).start
			cout<<"host send latency pkt to R1 :"<<(latencyTest).route<<endl;
			sleep(100);
			routeChoose++;
		}
		mq_return = mq_close(mqd_latencyHostToR1);
		mq_return = mq_unlink(latencyHostToR1);
	//	exit(0); // 结束子进程  
	}


int main() {
	/*initialization about mqueue*/
	char proname[] = "host";
	int running =1;
	struct mq_attr attr, attr_ctrl;
	//struct mq_attr q_attr;
	attr.mq_maxmsg = MAXMSG;//maximum is 382.
	attr.mq_msgsize = 2048;
	attr.mq_flags = 0;

	attr_ctrl.mq_maxmsg = MAXMSGCTOP;
	attr_ctrl.mq_msgsize = 2048;
	attr_ctrl.mq_flags = 0;

	
	int mq_return = 0;
	
	
    
	mqd_hosttop1 = mq_open(hosttop1, flags, PERM, &attr);
	//check_return(mqd_hosttop1, hosttop1, "mq_open");
	mqd_p1tohost = mq_open(p1tohost, flags, PERM, &attr);
	mqd_hosttoctrl = mq_open(hosttoctrl, flags, PERM, &attr);
	mqd_ctrltohost = mq_open(ctrltohost, flags, PERM, &attr);
	mqd_latencyHostToR1 = mq_open(latencyHostToR1, flags, PERM, &attr);
	
	
	
	
	
	pthread_t send_id,receive_id,sendtoctrl_id,latency_test_id;  
	
	int ret = pthread_create(&send_id, NULL, send, NULL);
    if(ret) {
        cout << "host Create send pthread error!" << endl;
        return 1;
    }
    ret = pthread_create(&receive_id, NULL, receive, NULL);
    if(ret) {
        cout << "host Create receive pthread error!" << endl;
        return 1;
    }
    ret = pthread_create(&sendtoctrl_id, NULL, sendtoctrl, NULL);
    if(ret) {
        cout << "host Create sendtoctrl pthread error!" << endl;
        return 1;
    }
    ret = pthread_create(&latency_test_id, NULL, latency_test, NULL);
    if(ret) {
        cout << "host Create latency_test pthread error!" << endl;
        return 1;
    }
    pthread_join(send_id, NULL);
    pthread_join(receive_id, NULL);
    pthread_join(sendtoctrl_id, NULL);
    pthread_join(latency_test_id, NULL);
//	exit(0);

}







