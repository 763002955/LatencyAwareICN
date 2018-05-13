#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mqueue.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <cstdlib>
#include <map>
#include <sys/time.h>
using namespace std;

#define MAXMSG 200//the stable number of mq_attr.mq_maxmsg. 300 is available.
#define MAXMSGCTOP 10//the queue from controller to process .
#define PERM 0766//the authority of m_queue.
#define MAX_TEXT 512  
#define DATANUM 26

map<string,string > resource_R1;
string interest[DATANUM] = {
				"a","b","c","d","e","f","g","h","i","j","k","l","m",
				"n","o","p","q","r","s","t","u","v","w","x","y","z"	
				};
string total_data[DATANUM] = {"A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z"};

int flags = O_CREAT | O_RDWR;
	int flags_ctrl = O_CREAT | O_RDWR | O_NONBLOCK;
	mqd_t mqd_hosttop1, mqd_p1tohost, mqd_p1top2, mqd_p1top3, mqd_p2top1,mqd_p3top1;
	mqd_t mqd_latencyR1ToR2, mqd_latencyR1ToR3, mqd_latencyHostToR1;
	mqd_t mqd_CtrlToR1;
	int mq_return = 0;
	char hosttop1[] = "/hosttop1";
	char p1tohost[] = "/p1tohost";
	char p1top2[] = "/p1top2";
	char p1top3[] = "/p1top3";
	char p2top1[] = "/p2top1";
	char p3top1[] = "/p3top1";
	char latencyR1ToR2[] = "/latencyR1ToR2";
	char latencyR1ToR3[] = "/latencyR1ToR3";
	char latencyHostToR1[] = "/latencyHostToR1";
	char CtrlToR1[] = "/CtrlToR1";
int pit_record_num=0;
void init_resource()
{
	for(int i = 0; i < 10; i++){
		resource_R1[interest[i]] = total_data[i];
	}
}

void print_cs()
{
	cout<<"cs in r1"<<endl;
	map<string,string>::iterator iter;
	for(iter = resource_R1.begin(); iter != resource_R1.end(); iter++)  
		cout<<iter->first<<' '<<iter->second<<endl;  
}

struct interest_pkt  
{  
	int latency_requirment;  
    char text[MAX_TEXT];  
};  

struct data_msg
{
	int latency_requirment;  
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
	char buffer[2048];
	int running = 1;
	int port = 0;
	char *found = new char[MAX_TEXT];
	struct interest_pkt *data;  
	struct data_msg *datamsg;
	struct data_msg *r2tor1,*r3tor1;
	struct interest_pkt *command;
	struct LatencyTestPKT *latencyTest;
	struct timeval t_start,t_end; 
	struct cmd pit[10];
	
void print_pit()
{
	cout<<"pit in r1"<<endl;
	for(int i = 0;i<10;i++)
		cout<<"text:"<<pit[i].text<<" "<<endl;
}
char* find_data(char *index)//index in data out
{
	//struct data_msg datamsg;
	//map later
	string str;

	char * data;
	map<string,string>::iterator iter;
	iter = resource_R1.find(index);
	if(iter != resource_R1.end())
	{
		str = iter->second;
		int len = str.length();
		data = (char *)malloc((len+1)*sizeof(char));
		str.copy(data,len,0);
		cout<< "found in R1 " << *data <<endl;
		return data;
	}
	
	else {cout<< "no found in R1！"<<endl;return NULL;}
}


void  *interest_q(void *ptr)
    {
		while(running)
		{
			//memset(&data.text, 0, 128); 
			mq_return = mq_receive(mqd_hosttop1, buffer, 2048, 0);
			if(mq_return == -1) {
				cout<<"receive error"<<endl;
//				return -1;
			}
			data = (struct interest_pkt*) buffer;
			datamsg = (struct data_msg*) buffer;
			printf("R1 receive from host : %s\n",(*datamsg).text);  
			found = find_data((*datamsg).text);
			
			
			if(found != NULL)
			{
				
				strncpy((*datamsg).text,found, MAX_TEXT);
			
				if(mq_send(mqd_p1tohost, (char *)datamsg, 128, 0) == -1)  
				{  
					cout<<"msgsnd back failed"<<endl;
					exit(EXIT_FAILURE);  
				}  
				cout<< "R1 send back to host: " << (*datamsg).text << endl;
			}
			else
			{
				//find port
				/*
				if(strcmp(found,"k")==0||strcmp(found,"l")==0||strcmp(found,"m")==0||strcmp(found,"n")==0||strcmp(found,"0")==0)
				{	port=1;cout<<"111111"<<endl;}
				else port = 2;*/
				if((*datamsg).latency_requirment %2 == 0)
					port = 2;
				else port = 1;
				//srand((unsigned)time(NULL));
				//port=rand()%2+1;
				//port=2;
				//cout<<"choose port?"<<port<<endl;
				if(port == 1)//to R2
				{
					//cout<<"R1->R2!"<<endl;
					cout<<"to R2"<<endl;
					if(mq_send(mqd_p1top2, (char *)data, 128, 0) == -1)  
					{  
						cout<<"msgsnd back failed"<<endl;
						exit(EXIT_FAILURE);  
					}  
				}
				else if (port == 2)//to R3
				{
					cout<<"to R3"<<endl;
					if(mq_send(mqd_p1top3, (char *)data, 128, 0) == -1)  
					{  
						cout<<"msgsnd back failed"<<endl;
						exit(EXIT_FAILURE);  
					}  
				}
				else 
					cout << "error port " << endl;
				
				cout << "send interest " << (*data).text << "to R "<< port+1 <<endl;
			}

		}
	
		//hosttop1
		mq_return = mq_close(mqd_hosttop1);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_close");
		mq_return = mq_unlink(hosttop1);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_unlink");
		
		//p1tohost
		mq_return = mq_close(mqd_p1tohost);
		mq_return = mq_unlink(p1tohost);
		
		//p1top2
		mq_return = mq_close(mqd_p1top2);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_close");
		mq_return = mq_unlink(p1top2);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_unlink");
		
		//p1top3
		mq_return = mq_close(mqd_p1top3);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_close");
		mq_return = mq_unlink(p1top3);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_unlink");
		
		//exit(0);
	}
	
void  *data_q_r2tor1(void *ptr)
	{
		
		//use 1 thread to receive R2->R1 pkt
			while(running)
			{
				//R2->R1
				mq_return = mq_receive(mqd_p2top1, buffer, 2048, 0);
				r2tor1 = (struct data_msg*) buffer;
				printf("data : R2 back ->R1: %s\n",(*r2tor1).text);  
				
				//检查是否需要缓存,缓存好之后把cmd记录删掉
				for(int i = 0; i < 10; i++) 
				{
					cout << i <<": "<<pit[i].text<<" - "<<pit[i].flag<<"---"<<(strcmp(pit[i].text, (*r2tor1).text))<<endl;
					if ((strcmp(pit[i].text, (*r2tor1).text) == 32) && (pit[i].flag == 1))
					{
						//缓存 insert 进 cs map
						resource_R1.insert(map<string, string>::value_type (pit[i].text, (*r2tor1).text));
						print_cs();
						pit[i].flag = 0;
						
						break;
					}
				}
				
				//......
				if(mq_send(mqd_p1tohost, (char *)r2tor1, 128, 0) == -1)  
				{  
					cout<<"msgsnd back failed"<<endl;
					exit(EXIT_FAILURE);  
				}  
				
				
			}
		
		//p2top1
		mq_return = mq_close(mqd_p2top1);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_close");
		mq_return = mq_unlink(p2top1);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_unlink");
		//	exit(0);
	}	
	
void  *data_q_r3tor1(void *ptr)
	{
		
		//use 1 thread to receive R3->R1 pkt
			while(running)
			{
				//R3->R1
				mq_return = mq_receive(mqd_p3top1, buffer, 3048, 0);
				r3tor1 = (struct data_msg*) buffer;
				printf("data : R3 back ->R1: %s\n",(*r3tor1).text);  
				
				//检查是否需要缓存,缓存好之后把cmd记录删掉
				for(int i = 0; i < 10; i++) 
				{
					if ((strcmp(pit[i].text, (*r3tor1).text) == 0) && (pit[i].flag == 1))
					{
						//缓存 insert 进 cs map
						resource_R1.insert(map<string, string>::value_type (pit[i].text, (*r3tor1).text));
						print_cs();
						pit[i].flag = 0;
						break;
					}
				}
				
				
				//......
				if(mq_send(mqd_p1tohost, (char *)r3tor1, 138, 0) == -1)  
				{  
					cout<<"msgsnd back failed"<<endl;
					exit(EXIT_FAILURE);  
				}  
				
				
			}
		
		//p3top1
		mq_return = mq_close(mqd_p3top1);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_close");
		mq_return = mq_unlink(p3top1);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_unlink");
		//	exit(0);
	}	
	
void  *latency_test(void *ptr)
	{
		 unsigned long cost_time=0;  
		while(running)
		{
			//receive latency pkt from latencyhosttor1
			mq_return = mq_receive(mqd_latencyHostToR1, buffer, 2048, 0);
			latencyTest = (struct LatencyTestPKT*) buffer;
			
			gettimeofday(&t_end, NULL); 
			//calculate time slot 
			cost_time = 10000*(t_end.tv_sec-(*latencyTest).start_sec)+(t_end.tv_usec-(*latencyTest).start_usec);  
			(*latencyTest).duration[0] = cost_time;
			//(*latencyTest).duration[0] *= 1000;
			cout << "R1 receive latency pkt: host <-> R1 duration : " <<(*latencyTest).duration[0] << " send to R"<<(*latencyTest).route <<endl;
			
			//更新start
			gettimeofday(&t_start, NULL); 
			(*latencyTest).start_sec = t_start.tv_sec;
			(*latencyTest).start_usec = t_start.tv_usec;
			
			if((*latencyTest).route == 2)//send to R2
			{
				if(mq_send(mqd_latencyR1ToR2, (char *)latencyTest, 128, 0) == -1)  
				{  
					cout<<"msgsnd back failed"<<endl;
					exit(EXIT_FAILURE);  
				} 
			}
			else if ((*latencyTest).route == 3)//send to R3
			{
				if(mq_send(mqd_latencyR1ToR3, (char *)latencyTest, 128, 0) == -1)  
				{  
					cout<<"msgsnd back failed"<<endl;
					exit(EXIT_FAILURE);  
				} 
			}
		}
		mq_return = mq_close(mqd_latencyR1ToR2);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_close");
		mq_return = mq_unlink(latencyR1ToR2);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_unlink");
		
		mq_return = mq_close(mqd_latencyR1ToR3);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_close");
		mq_return = mq_unlink(latencyR1ToR3);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_unlink");
		
	}
void  *cmd_ctrltor1(void *ptr)
	{
		//receive cmd from ctrl
		
		while(running)
		{
			mq_return = mq_receive(mqd_CtrlToR1, buffer, 2048, 0);
			command = (struct interest_pkt*) buffer;
			cout<<"------command from ctrl: "<< (*command).text << " - "<< (*command).latency_requirment<<endl;
			
			//change pit
			strcpy(pit[pit_record_num%10].text,(*command).text);
			pit[pit_record_num%10].flag = 1;
			pit_record_num++;
			
		}
		mq_return = mq_close(mqd_CtrlToR1);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_close");
		mq_return = mq_unlink(CtrlToR1);//returns 0 on success, or -1 on error.
		//check_return(mq_return, hosttop1, "mq_unlink");
}
//set a fib table to determine port
int main() {
	/*initialization about mqueue*/
	char proname[] = "R1";
//	string pit[30];
	int pit_record_num = 0;
	
	struct mq_attr attr, attr_ctrl;
	
	//struct mq_attr q_attr;
	attr.mq_maxmsg = MAXMSG;//maximum is 382
	attr.mq_msgsize = 2048;
	attr.mq_flags = 0;

	attr_ctrl.mq_maxmsg = MAXMSGCTOP;
	attr_ctrl.mq_msgsize = 2048;
	attr_ctrl.mq_flags = 0;

	
	
	mqd_hosttop1 = mq_open(hosttop1, flags, PERM, &attr);
	mqd_p1tohost = mq_open(p1tohost, flags, PERM, &attr);
	mqd_p1top2 = mq_open(p1top2, flags, PERM, &attr);
	mqd_p1top3 = mq_open(p1top3, flags, PERM, &attr);
	mqd_p2top1 = mq_open(p2top1, flags, PERM, &attr);
	mqd_p3top1 = mq_open(p3top1, flags, PERM, &attr);
	mqd_latencyR1ToR2 = mq_open(latencyR1ToR2, flags, PERM, &attr);
	mqd_latencyR1ToR3 = mq_open(latencyR1ToR3, flags, PERM, &attr);
	mqd_latencyHostToR1 = mq_open(latencyHostToR1, flags, PERM, &attr);
	mqd_CtrlToR1 = mq_open(CtrlToR1, flags, PERM, &attr);
	
	init_resource();
	
	
	pthread_t interest_q_id,data_q_r2tor1_id,data_q_r3tor1_id,latency_test_id,cmd_ctrltor1_id;
	int ret = pthread_create(&interest_q_id, NULL, interest_q, NULL);
    if(ret) {
        cout << "host Create send pthread error!" << endl;
        return 1;
    }
    ret = pthread_create(&data_q_r2tor1_id, NULL, data_q_r2tor1, NULL);
    if(ret) {
        cout << "host Create receive pthread error!" << endl;
        return 1;
    }
    ret = pthread_create(&data_q_r3tor1_id, NULL, data_q_r3tor1, NULL);
    if(ret) {
        cout << "host Create sendtoctrl pthread error!" << endl;
        return 1;
    }
    ret = pthread_create(&latency_test_id, NULL, latency_test, NULL);
    if(ret) {
        cout << "host Create latency_test pthread error!" << endl;
        return 1;
    }
    ret = pthread_create(&cmd_ctrltor1_id, NULL, cmd_ctrltor1, NULL);
    if(ret) {
        cout << "host Create cmd_ctrltor1_id pthread error!" << endl;
        return 1;
    }
    pthread_join(interest_q_id, NULL);
    pthread_join(data_q_r2tor1_id, NULL);
    pthread_join(data_q_r3tor1_id, NULL);
    pthread_join(latency_test_id, NULL);
	pthread_join(cmd_ctrltor1_id, NULL);
    
	exit(0);

}







