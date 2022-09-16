#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>


int* conv_InputMatrix(int** matrix,int len, int index);
int conv_Filter(int input[]);
int* Mpool_InputMatrix(int input[],int len, int index);
int Mpool_Filter(int input[]);
void* conv_thread(void *);
void* pool_thread(void *);
void read_file(int fd,int** matrix,int len, int start);
void write_file(int fd, int result[],int size, int len);

volatile int global_index=-1;
int** myMatrix=NULL;
int* conv_result=NULL;
int a=0;

pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;

typedef struct{
	int i;
	int result;
}my_ret;


int main(int argc, char* argv[])
{

	char buf;
	char* row_buffer;
	int count=0;
	int file_d;
	file_d=open(argv[1],O_RDONLY);
	if(file_d<0){
		perror("file_open");
		exit(0);
	}
	while(1)
	{
		read(file_d,&buf,1);
		if(buf=='\n')
			break; // count is number of digit in first line
		count++;
	}
	row_buffer=malloc(sizeof(char)*count);
	pread(file_d,row_buffer,count,0);
	a=atoi(row_buffer); //global var

	int conv_thread_count= (a-2)*(a-2);
	int pool_thread_count=conv_thread_count/4;

	pthread_t conv_thread_id[conv_thread_count];
	pthread_t pool_thread_id[pool_thread_count];

	int pool_result[pool_thread_count];

	int status;
	
	//global var
	myMatrix=(int**)malloc(sizeof(int*) * a);
	for(int i=0; i<a; i++){
		myMatrix[i]=(int*)malloc(sizeof(int)*a);
	}
	conv_result=(int*)malloc(sizeof(int) * conv_thread_count);


	read_file(file_d, myMatrix, a,count);
	close(file_d);

	//print input matrix
//	for(int i=0; i<a; i++){
//		for(int j=0; j<a; j++){
//			printf("%d ",myMatrix[i][j]);
//		}
//		printf("\n");
//	}

///////////conv layer/////////////

	my_ret *ret;

	for(int i=0; i<conv_thread_count; i++)
	{
		status=pthread_create(&conv_thread_id[i], NULL,conv_thread ,NULL);
		if(status!=0){
			perror("pthread_create");
			exit(0);
		}
	}

	for(int i=0; i<conv_thread_count; i++)
	{
		pthread_join(conv_thread_id[i],(void**)&ret);
//		printf("ret_index= %d , ret_result= %d\n",ret->i,ret->result);       //print Parallel workers
		conv_result[ret->i]=ret->result;

	}

	//print conv_Layer result
//	for(int i=0; i<conv_thread_count; i++)
//		printf("%d ",conv_result[i]);
//	printf("\n");


//////////pool layer//////////////

	global_index=-1;   //re-indexing of global var
	my_ret *ret2;
	for(int i=0; i<pool_thread_count; i++)
	{
		status=pthread_create(&pool_thread_id[i], NULL, pool_thread , NULL);
		if(status != 0){
			perror("pthread_create");
			exit(0);
		}
	}

	for(int i=0; i<pool_thread_count; i++)
	{
		pthread_join(pool_thread_id[i],(void**)&ret2);
//		printf("ret_index = %d , ret_result= %d\n",ret2->i,ret2->result);       //print Parallel workers
		pool_result[ret2->i]=ret2->result;
	}
	
	//print Mpool_Layer result
//	for(int i=0; i<pool_thread_count; i++)
//		printf("%d ",pool_result[i]);
//	printf("\n");



	file_d=creat(argv[2],S_IRWXU);
	if(file_d<0){
		perror("write_file open");
		exit(0);
	}
	write_file(file_d,pool_result,pool_thread_count,(a-2)/2);
	
	
	
	
	for(int i=0; i<a; i++){
		free(myMatrix[i]);
	}
	free(row_buffer);
	free(conv_result);
	free(myMatrix);
	close(file_d);
	pthread_mutex_destroy(&mutex);	

}

void output_formatting(char buf[],int num)
{
	snprintf(buf,5,"%4d",num);
}

void write_file(int fd, int result[] ,int size, int len)
{
	char buf[5];
	char newline[]="\n";
	char blank=' ';
	int offset=0;
	int line=0;
	for(int i=0; i<size; i++)
	{
		output_formatting(buf,result[i]);
		pwrite(fd,buf,4,offset);
		offset+=4;
		if(line==(len-1)){
			pwrite(fd,&newline,1,offset);
			offset+=1;
			line=0;
		}
		else{
			pwrite(fd,&blank,1,offset);
			offset+=1;
			line++;
		}
	}
	pwrite(fd,&newline,1,offset);

}

void read_file(int fd, int** matrix, int len, int count)
{
	char* buf;
	buf=(char*)malloc(sizeof(char)*2);
	int i=0,j=0;
	int offset=count+1; //end of first line is \n
	for(int i=0; i<len; i++)
	{
		for(int j=0; j<len; j++)
		{
			pread(fd,buf,2,offset);
			matrix[i][j]=atoi(buf);
			offset+=3;
		}
	}		
}


void* conv_thread(void* data)
{
	my_ret* ret=malloc(sizeof(my_ret));
	int* input_arr=malloc(sizeof(int)*9);
	int* result=malloc(sizeof(int));

	pthread_mutex_lock(&mutex); //mutex
	global_index++;
	ret->i=global_index;
	pthread_mutex_unlock(&mutex);

	input_arr=conv_InputMatrix(myMatrix,a,ret->i);
	*result=conv_Filter(input_arr);
	ret->result=*result;
	return (void*)ret;
}


void* pool_thread(void* data)
{
	my_ret* ret2=malloc(sizeof(my_ret));
	int* arr=malloc(sizeof(int)*4);
	int* result2=malloc(sizeof(int));

	pthread_mutex_lock(&mutex);
	global_index++;
	ret2->i=global_index;
	pthread_mutex_unlock(&mutex);
	arr=Mpool_InputMatrix(conv_result,a-2,ret2->i);
	*result2= Mpool_Filter(arr);
	ret2->result=*result2;
	return (void*)ret2;	
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
	static int arr[9];
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
	
