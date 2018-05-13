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
#include <map>
#include <sys/time.h>
using namespace std;
#define MAXMSG 200//the stable number of mq_attr.mq_maxmsg. 300 is available.
#define MAXMSGCTOP 10//the queue from controller to process .
#define PERM 0766//the authority of m_queue.
#define MAX_TEXT 512  
#define DATANUM 26

map<string,string > resource_R4;
string interest[DATANUM] = {
				"a","b","c","d","e","f","g","h","i","j","k","l","m",
				"n","o","p","q","r","s","t","u","v","w","x","y","z"	
				};
string total_data[DATANUM] = {"A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z"};
void init_resource()
{
	for(int i = 18; i < 26; i++){
		resource_R4[interest[i]] = total_data[i];
	}
}

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

struct cmd
{
	char text[MAX_TEXT];
	int flag;//1-add 0-delete
};

struct LatencyTestPKT
{
	int route;//(host->)r1->r2->r4 / (host->)r1->r3->r4 标志R2还是R3
	long start_sec,start_usec;
	long duration[3];// host -> r1  r1 -> r2/3   r2/3 -> r4
};

char* find_data(char *index)//index in data out
{
	//struct data_msg datamsg;
	//map later
	string str;
	init_resource();
	char * data;
	map<string,string>::iterator iter;
	iter = resource_R4.find(index);
	if(iter != resource_R4.end())
	{
		str = iter->second;
		int len = str.length();
		data = (char *)malloc((len+1)*sizeof(char));
		str.copy(data,len,0);
		return data;
	}
	else return NULL;
}


void print_cs()
{
	cout<<"cs in r4"<<endl;
	map<string,string>::iterator iter;
	for(iter = resource_R4.begin(); iter != resource_R4.end(); iter++)  
		cout<<iter->first<<' '<<iter->second<<endl;  
}

int flags = O_CREAT | O_RDWR;
	int flags_ctrl = O_CREAT | O_RDWR | O_NONBLOCK;
	mqd_t mqd_p3top4, mqd_p4top3, mqd_p2top4, mqd_p4top2;
	mqd_t mqd_latencyR3ToR4, mqd_latencyR2ToR4;
	mqd_t mqd_CtrlToR4,mqd_R4ToCtrl;
		mqd_t mqd_CtrlToR3;
	int mq_return = 0;

	struct interest_pkt *data_fromr2,*data_fromr3;  
	struct data_msg *r4tor2,*r4tor3;
	struct LatencyTestPKT *latencyTest;
	struct interest_pkt *command;
	struct timeval t_start,t_end; 
	char p4top2[] = "/p4top2";
	char p2top4[] = "/p2top4";
	char p4top3[] = "/p4top3";
	char p3top4[] = "/p3top4";
	char latencyR2ToR4[] = "/latencyR2ToR4";
	char latencyR3ToR4[] = "/latencyR3ToR4";
	char CtrlToR4[] = "/CtrlToR4";
	char R4ToCtrl[] = "/R4ToCtrl";
	int running =1;	
	int msg_no = 0;
	char buffer2[2048],buffer3[2048],bufferl2[2048],bufferl3[2048],buffer[2048];

//use 1 thread receive interest pkt from r3 and send back

void  *interest_q_r3tor4(void *ptr)
    {  
        while(running)
		{	
			//mq_return = mq_receive(mqd_p3top4, buffer, 2048, 0);
			
			if((mq_receive(mqd_p3top4, buffer3, 2048, 0)) == -1) {
				cout<<"r3 to r4 receive error"<<strerror(errno)<<"  "<< errno<<endl;
//				return -1;
			}
			//cout<<"begin receive"<<endl;
			data_fromr3 = (struct interest_pkt*) buffer3;
			cout<<"R4 receive interest from R3 : " <<(*data_fromr3).text<<endl;

			r4tor3 = (struct data_msg*) buffer3;
			
			//search cs
			strcpy((*r4tor3).text,find_data((*data_fromr3).text));
			//send back data pkt R4->R3
			if(mq_send(mqd_p4top3, (char *)r4tor3, 128, 0) == -1)  
			{  
				cout<<"msgsnd back failed"<<endl;
				exit(EXIT_FAILURE); 
			}
			cout<<"R4 send data back to R3  " << (*r4tor3).text<<endl;
			
			
		}
	
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
   void  *interest_q_r2tor4(void *ptr) 
    {  
        while(running)
		{	
	
				if((mq_receive(mqd_p2top4, buffer2, 2048, 0)) == -1) {
					cout<<"r2 to r4 receive error"<<strerror(errno)<<"  "<< errno<<endl;
	//				return -1;
				}
				//cout<<"begin receive"<<endl;
				data_fromr2 = (struct interest_pkt*) buffer2;
				cout<<"R4 receive interest from R2 : " <<(*data_fromr2).text<<endl;

				r4tor2 = (struct data_msg*) buffer2;
				
				//search cs
				strcpy((*r4tor2).text,find_data((*data_fromr2).text));
				//send back data pkt R4->R2
				if(mq_send(mqd_p4top2, (char *)r4tor2, 128, 0) == -1)  
				{  
					cout<<"msgsnd back failed"<<endl;
					exit(EXIT_FAILURE); 
				}
				cout<<"R4 send data back to R2  " << (*r4tor2).text<<endl;
		
			
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

void  *latency_test_R2(void *ptr)
	{
		while(running)
		{
			//receive latency pkt from latencyr1toR3
			//mq_return = mq_receive(mqd_latencyR2ToR4, buffer, 2048, 0);
			if((mq_receive(mqd_latencyR2ToR4, bufferl2, 2048, 0)) == -1) {
				cout<<"r2 to r4 receive error"<<strerror(errno)<<"  "<< errno<<endl;
//				return -1;
			}
			latencyTest = (struct LatencyTestPKT*) bufferl2;
			gettimeofday(&t_end, NULL); 
			//calculate time slot 
			(*latencyTest).duration[2] = 10000*(t_end.tv_sec-(*latencyTest).start_sec)+(t_end.tv_usec-(*latencyTest).start_usec);  
			 //= cost_time;
			//(*latencyTest).duration[0] *= 1000;
			cout << "R4 receive latency pkt: host <-> R1 duration : " <<(*latencyTest).duration[2] << " send to R"<<(*latencyTest).route <<endl;
						
			//SEND TO CTRL
		}
		mq_return = mq_close(mqd_latencyR2ToR4);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_close");
		mq_return = mq_unlink(latencyR2ToR4);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_unlink");
		
	}

void  *latency_test_R3(void *ptr)
	{
		while(running)
		{
			//receive latency pkt from latencyr1toR3
			if((mq_receive(mqd_latencyR3ToR4, bufferl3, 2048, 0)) == -1) {
				cout<<"r2 to r4 receive error"<<strerror(errno)<<"  "<< errno<<endl;
//				return -1;
			}
			latencyTest = (struct LatencyTestPKT*) bufferl3;
			gettimeofday(&t_end, NULL); 
			//calculate time slot 
			(*latencyTest).duration[2] = 10000*(t_end.tv_sec-(*latencyTest).start_sec)+(t_end.tv_usec-(*latencyTest).start_usec);  
			 //= cost_time;
			//(*latencyTest).duration[0] *= 1000;
			cout << "R4 receive latency pkt: host <-> R1 duration : " <<(*latencyTest).duration[2] << " send to R"<<(*latencyTest).route <<endl;
			
			//SEND TO CTRL  mqd_R4ToCtrl
			if(mq_send(mqd_R4ToCtrl, (char *)latencyTest, 128, 0) == -1)  
			{  
				cout<<"msgsnd back failed"<<endl;
				exit(EXIT_FAILURE); 
			}
			cout<<"R4 send Latency to ctrl :" <<(*latencyTest).route<<endl;
			
			
		}
		mq_return = mq_close(mqd_latencyR3ToR4);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_close");
		mq_return = mq_unlink(latencyR3ToR4);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_unlink");
		
		mq_return = mq_close(mqd_R4ToCtrl);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_close");
		mq_return = mq_unlink(R4ToCtrl);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_unlink");
		
}
/*
void  *cmd_ctrltor4(void *ptr)
{
		while(1)
		{
			mq_return = mq_receive(mqd_CtrlToR4, buffer, 2048, 0);
			command = (struct interest_pkt*) buffer;
			cout<<"command from ctrl: "<< (*command).text << " - "<< (*command).latency_requirment<<endl;
			
		}
		
		mq_return = mq_close(mqd_CtrlToR4);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_close");
		mq_return = mq_unlink(CtrlToR4);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_unlink");
}*/
	
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

	
	init_resource();
	mqd_p3top4 = mq_open(p3top4, flags, PERM, &attr);
	mqd_p4top3 = mq_open(p4top3, flags, PERM, &attr);
	mqd_p2top4 = mq_open(p2top4, flags, PERM, &attr);
	mqd_p4top2 = mq_open(p4top2, flags, PERM, &attr);
	mqd_latencyR3ToR4 = mq_open(latencyR3ToR4, flags, PERM, &attr);
	mqd_latencyR2ToR4 = mq_open(latencyR2ToR4, flags, PERM, &attr);
	mqd_R4ToCtrl= mq_open(R4ToCtrl, flags, PERM, &attr);
	mqd_CtrlToR4= mq_open(CtrlToR4, flags, PERM, &attr);
	
	pthread_t interest_q_r2tor4_id, interest_q_r3tor4_id, latency_test_R3_id, latency_test_R2_id, cmd_ctrltor4_id;
	int ret = pthread_create(&interest_q_r2tor4_id, NULL, interest_q_r2tor4, NULL);
    if(ret) {
        cout << "host Create send pthread error!" << endl;
        return 1;
    }
    ret = pthread_create(&interest_q_r3tor4_id, NULL, interest_q_r3tor4, NULL);
    if(ret) {
        cout << "host Create receive pthread error!" << endl;
        return 1;
    }
    ret = pthread_create(&latency_test_R3_id, NULL, latency_test_R3, NULL);
    if(ret) {
        cout << "host Create sendtoctrl pthread error!" << endl;
        return 1;
    }
    ret = pthread_create(&latency_test_R2_id, NULL, latency_test_R2, NULL);
    if(ret) {
        cout << "host Create latency_test pthread error!" << endl;
        return 1;
    }
  /*  ret = pthread_create(&cmd_ctrltor4_id, NULL, cmd_ctrltor4, NULL);
    if(ret) {
        cout << "host Create cmd_ctrltor4_id pthread error!" << endl;
        return 1;
    }*/
    //cout<<
    pthread_join(interest_q_r2tor4_id, NULL);
    pthread_join(interest_q_r3tor4_id, NULL);
    pthread_join(latency_test_R3_id, NULL);
    pthread_join(latency_test_R2_id, NULL);
	//pthread_join(cmd_ctrltor4_id, NULL);
	

	exit(0);

}








