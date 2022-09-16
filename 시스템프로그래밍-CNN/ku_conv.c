#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <math.h>


#define conv_firstMQ  "/Conv"
#define conv_secondMQ "/Conv_layered"
#define pooling_firstMQ "/Pool"
#define pooling_secondMQ "/Pool_layered"


void makeMatrix(int** matrix, int X, int Y);
mqd_t createPosixMQ(const char* NAME, int Maxsize, int Msgsize);
int* conv_InputMatrix(int** matrix,int len, int index);
int conv_Filter(int input[]);
int* Mpool_InputMatrix(int input[],int len, int index);
int Mpool_Filter(int input[]);


int main(int argc, char* argv[])
{
	int ** myMatrix;
	int a=atoi(argv[1]);
	int conv_count= (a-2)*(a-2);

	myMatrix=(int**)malloc(sizeof(int*) * a);
	for(int i=0; i<a; i++){
		myMatrix[i]=(int*)malloc(sizeof(int)*a);
	}

	makeMatrix(myMatrix,a,a);

//	for(int i=0; i<a; i++){
//		for(int j=0; j<a; j++){
//			printf("%d ",myMatrix[i][j]);
//		}
//		printf("\n");
//	}


	mqd_t my_mq=createPosixMQ(conv_firstMQ , conv_count, 36);
	mqd_t conv_mq=createPosixMQ(conv_secondMQ, conv_count, 4);
	mqd_t my2_mq=createPosixMQ(pooling_firstMQ, conv_count/4, 16);
	mqd_t pool_mq=createPosixMQ(pooling_secondMQ, conv_count/4, 4);

	unsigned int prio=0;

	pid_t pid[conv_count];
	int conv_filter[conv_count];

	for(int i=0; i<conv_count; i++){
		if((pid[i]=fork()==0)){
		//child process
		unsigned int k;
		int data[9]={0};
		
		if(mq_receive(my_mq, (char*)&data, 36, &k)==-1){
			perror("mq_receive()");
			mq_close(my_mq);
			exit(0);
			}
		else{
//			printf("received msg prio: %d\n", k);
//			for(int i=0; i<9; i++)
//				printf("%d ", data[i]);
//			printf("   ");
//			printf("%d",conv_Filter(data));
			int output=conv_Filter(data);
//			printf("\n");
			mq_close(my_mq);


			if(mq_send(conv_mq,(char*)&output, 4, k)==-1){
				perror("conv_result_send()");
				exit(0);
				}
//			printf("send output: %d\n",output);
			}
		exit(i);
		}	
	}
		//parent process
		int conv_result[conv_count];
		unsigned int index_prio;
		for(prio=0; prio<conv_count; prio++){
			int *sample=conv_InputMatrix(myMatrix, a, prio);

//			printf("send msg prio: %d , value : %ls \n", prio, sample);
			if(mq_send(my_mq, (char*)sample, 36, prio)==-1){
				perror("mq_send()");
				break;
			}
		}
//		printf("\n");

		while(waitpid(-1,NULL,WNOHANG)>0);
		
		int data;
		for(int i=0; i<conv_count; i++){
			if(mq_receive(conv_mq, (char*)&data, 4, &index_prio)==-1){
				perror("conv_result_receive()");
				exit(0);
			}
			conv_result[index_prio]=data;
		}
//		printf("\n\nconv_result: ");
//		for(int i=0; i<conv_count; i++)
//			printf("%d ",conv_result[i]);
//			printf("\n\n");
	
		
	mq_close(conv_mq);
	mq_close(my_mq);
	mq_unlink(conv_firstMQ);
	mq_unlink(conv_secondMQ);


//conv_layer
	
//result_matrix(1 demension)
//	conv_result[conv_count];

	pid_t pid2[conv_count/4];
	for(int i=0; i<conv_count/4; i++){
		if((pid2[i]=fork())==0){
			//child process
			unsigned int k;
			int data[4]={0};

			if(mq_receive(my2_mq, (char*)data , 16, &k)==-1){
				perror("pool_input_receive()");
				mq_close(my2_mq);
				exit(0);
			}
			else{
//				printf("received prio : %d , data: ",k);
//				for(int l=0; l<4; l++)
//					printf("%d ",data[l]);
				int pool_filter=Mpool_Filter(data);
//				printf("	pooled data: %d\n",pool_filter);
				mq_close(my2_mq);
	
				if(mq_send(pool_mq, (char*)&pool_filter , 4, k)==-1){
			        	perror("pool_result_send()");
		       			exit(0);	       
					}
//				printf("send output: %d, prio :%d\n",pool_filter ,k); 
				
			}
		exit(i);
		}
	}

	//parent process
	unsigned int i;
	for(i=0; i<conv_count/4; i++){	
		int * max_pool_input=Mpool_InputMatrix(conv_result,a-2,i);
		
		if(mq_send(my2_mq, (char*)max_pool_input, 16 , i)==-1){
			perror("pool_input_send()");
			mq_close(my2_mq);
			mq_unlink(pooling_firstMQ);
			exit(0);
		}
//		printf("sended prio : %d ,data: ",i);
//		for(int j=0; j<4; j++){
//			printf("%d ",max_pool_input[j]);
//		}
//		printf("\n");
	}
//	printf("\n");

	while(waitpid(-1,NULL,WNOHANG)>0);
	

	int finalresult[conv_count/4];
	int finaldata;
	for(int l=0; l<conv_count/4; l++){
		unsigned int prio_pool;
		if(mq_receive(pool_mq, (char*)&finaldata, 4, &prio_pool)==-1){
			perror("pool_result_receive()");
			exit(0);
		}else{
//			printf("received :%d, prio: %d\n",finaldata,prio_pool);
		finalresult[prio_pool]=finaldata;
		}
	}
	
	


//	printf("\n\nfinal result: ");
	for(int i=0; i<conv_count/4; i++)
		printf("%d ",finalresult[i]);









	mq_close(my2_mq);
	mq_close(pool_mq);
	mq_unlink(pooling_firstMQ);
	mq_unlink(pooling_secondMQ);

	for(int i=0; i<a; i++){
		free(myMatrix[i]);
	}
	free(myMatrix);
}

int Mpool_Filter(int input[])
{
	int max=input[0];
	for(int i=1; i<4; i++){
		if(max<input[i])
			max=input[i];
	}
	return max;
}

int* Mpool_InputMatrix(int input[], int len,int index)
{
	if(len==2)
		return input;
	static int output[4]={0};
	int doubled_index=2*index;
	int row=doubled_index/len;
	int count=0;
	if(doubled_index>=len){
		doubled_index+=(len*row);
	}

	while(count<4){
		for(int i=0; i<2; i++){
			output[count]=input[doubled_index+i];
			count++;
		}
		for(int i=0; i<2; i++){
			output[count]=input[doubled_index+len+i];
			count++;
		}
	}
	return output;
}

int conv_Filter(int input[])
{
	int num= input[4]*8;
	int sum=0;
	for(int i=0; i<9; i++){
		if(i==4) continue;
		sum+=input[i];
	}

	return num-sum;
}

int* conv_InputMatrix(int** matrix,int len, int index)
{
	static int arr[9]={0};
	int count=0,row=0;
	if(index>=len-2){
		row=index/(len-2);
		index=index%(len-2);
	}
	while(count<9){
		for(int i=row; i<3+row; i++){
			for(int j=index; j<3+index; j++){
				arr[count]=matrix[i][j];
				count++;
			}
		}
	}
	return arr;
}
	
mqd_t createPosixMQ(const char* NAME, int maxsize, int msgsize)
{
	struct mq_attr attr;
	mqd_t mq_rtn;
	if(maxsize>=10) maxsize=10;
	attr.mq_maxmsg= maxsize;
	attr.mq_msgsize= msgsize;

	mq_rtn=mq_open(NAME, O_CREAT | O_RDWR, 0600, &attr);
	if(mq_rtn<0){
		perror("mq_open()");
		exit(0);
	}
	return mq_rtn;
}





