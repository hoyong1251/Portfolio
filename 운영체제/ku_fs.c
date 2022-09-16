#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct inodes{
	unsigned int fsize;
	unsigned int block;
	unsigned int pointer[12];
	unsigned int unused[50];
};
struct i_blocks{
	struct inodes inode[16];
};
struct d_blocks{
	unsigned char data[4096];
};

struct bit{
	unsigned char b1 : 1;
	unsigned char b2 : 1;
	unsigned char b3 : 1;
	unsigned char b4 : 1;
	unsigned char b5 : 1;
	unsigned char b6 : 1;
	unsigned char b7 : 1;
	unsigned char b8 : 1;
};

struct inode_table{
	unsigned char inum;
	char name[3];
};
struct partition{
	struct d_blocks super_block;
	struct bit i_bmap[4096];
	struct bit d_bmap[4096];
	struct i_blocks i_block[5];
	struct d_blocks d_block[56];
}partition;

void printeof()
{
	for(int i=0; i<4096*64; i++)
		printf("%.2x ",*((unsigned char*)&partition + i));
}

int getfreeinum()
{
	for (int i = 0; i < 10; i++)
	{
		if (partition.i_bmap[i].b8 == 0) {
			partition.i_bmap[i].b8 = 1;
			return i * 8 + 0;
		}
		else if (partition.i_bmap[i].b7 == 0) {
			partition.i_bmap[i].b7 = 1;
			return i * 8 + 1;
		}
		else if (partition.i_bmap[i].b6 == 0) {
			partition.i_bmap[i].b6 = 1;
			return i * 8 + 2;
		}
		else if (partition.i_bmap[i].b5 == 0) {
			partition.i_bmap[i].b5 = 1;
			return i * 8 + 3;
		}
		else if (partition.i_bmap[i].b4 == 0) {
			partition.i_bmap[i].b4 = 1;
			return i * 8 + 4;
		}
		else if (partition.i_bmap[i].b3 == 0) {
			partition.i_bmap[i].b3 = 1;
			return i * 8 + 5;
		}
		else if (partition.i_bmap[i].b2 == 0) {
			partition.i_bmap[i].b2 = 1;
			return i * 8 + 6;
		}
		else if (partition.i_bmap[i].b1 == 0) {
			partition.i_bmap[i].b1 = 1;
			return i * 8 + 7;
		}
	}
	return 0;
}
int getfreednum()
{
	for (int i = 0; i < 7; i++)
	{
		if(partition.d_bmap[i].b8 ==0){
			partition.d_bmap[i].b8 = 1;
			return i * 8 + 0;
		}
		else if (partition.d_bmap[i].b7 == 0) {
			partition.d_bmap[i].b7 = 1;
			return i * 8 + 1;
		}
		else if (partition.d_bmap[i].b6 == 0) {
			partition.d_bmap[i].b6 = 1;
			return i * 8 + 2;
		}
		else if (partition.d_bmap[i].b5 == 0) {
			partition.d_bmap[i].b5 = 1;
			return i * 8 + 3;
		}
		else if (partition.d_bmap[i].b4 == 0) {
			partition.d_bmap[i].b4 = 1;
			return i * 8 + 4;
		}
		else if (partition.d_bmap[i].b3 == 0) {
			partition.d_bmap[i].b3 = 1;
			return i * 8 + 5;
		}
		else if (partition.d_bmap[i].b2 == 0) {
			partition.d_bmap[i].b2 = 1;
			return i * 8 + 6;
		}
		else if (partition.d_bmap[i].b1 == 0) {
			partition.d_bmap[i].b1 = 1;
			return i * 8 + 7;
		}
	}
	return 0;
}
struct inodes root_inode;
struct inode_table inode_table[1024] ={0,}; //use only 56 tables

int do_write(const char* fname, int len , int count)
{
	char writing_data = fname[0];
	int need_block,inum,dnum;
//	printf("%s\n" ,fname);
	for(int i=0; i<count; i++)
	{
		if(strcmp(((struct inode_table*)&partition.d_block[0])[i].name, fname)==0){
			printf("error: Existing file\n");
			return 0;
		}
	}
	inum = getfreeinum();
//	printf("%d\n",inum);
	((struct inode_table*)&partition.d_block[0])[count].inum = inum;
	strcpy(((struct inode_table*)&partition.d_block[0])[count].name, fname);
	partition.i_block[inum/16].inode[inum%16].fsize = len;
	
	if(len%4096 == 0 && len !=0 )
		need_block = len/4096;
	else
		need_block = len/4096 + 1;

	partition.i_block[inum/16].inode[inum%16].block = need_block;
	int k=0;
	while(need_block>0)
	{
		dnum = getfreednum();
		if(dnum == 0){
			printf("error : no file space\n");
			return 0;
		}
		partition.i_block[inum/16].inode[inum%16].pointer[k]=dnum;
		k++;
		if(need_block==1){
			for(int j=0; j<len; j++)
				partition.d_block[dnum].data[j]=writing_data;
			need_block--;
		}
		else{
			for(int j=0; j<4096; j++)
				partition.d_block[dnum].data[j]=writing_data;
			len-=4096;
			need_block--;
		}
	}

}
int do_read(const char* fname, int len)
{
	int target_inum=0;
	int target_dnum, num_block;
	int read_block;
	for(int i=0; i<80; i++)
	{
		if(strcmp(((struct inode_table*)&partition.d_block[0])[i].name , fname) ==0){
			target_inum = ((struct inode_table *)&partition.d_block[0])[i].inum;
		}
	}
	if(!target_inum){
		printf("No such File\n");
		return 0;
	}
	num_block = partition.i_block[target_inum/16].inode[target_inum%16].block;
	target_dnum = partition.i_block[target_inum/16].inode[target_inum%16].pointer[0];
	
	read_block = len/4096 +1;
	while(read_block >0)
	{
		//printf("reading block by blocknumber %d\n",target_dnum);
		if(read_block == 1){
			for(int i=0; i<len; i++)
			{
				printf("%c ",partition.d_block[target_dnum].data[i]);
			}
			printf("\n");
			read_block--;
		}
		else{
			for(int i=0; i<4096; i++)
			{
				printf("%c ",partition.d_block[target_dnum].data[i]);
			}
			printf("\n");
			len-=4096;
			target_dnum++;
			read_block--;
		}
	}
	
}
int do_delete(const char* fname)
{
	int tar_inum=0;
	for(int i=0; i<80; i++)
	{
		if(strcmp(((struct inode_table*)&partition.d_block[0])[i].name , fname) ==0){
			tar_inum = ((struct inode_table *)&partition.d_block[0])[i].inum;
			((struct inode_table *)&partition.d_block[0])[i].inum = 0;
		}
	}

	if(!tar_inum){
		printf("no such file\n");
		return 0;
	}
	int bit_index = tar_inum/8;

	if(tar_inum%8 ==0)
		partition.i_bmap[bit_index].b8 = 0;
	else if(tar_inum%8 ==1)
		partition.i_bmap[bit_index].b7 = 0;
	else if(tar_inum%8 ==2)
		partition.i_bmap[bit_index].b6 = 0;
	else if(tar_inum%8 ==3)
		partition.i_bmap[bit_index].b5 = 0;
	else if(tar_inum%8 ==4)
		partition.i_bmap[bit_index].b4 = 0;
	else if(tar_inum%8 ==5)
		partition.i_bmap[bit_index].b3 = 0;
	else if(tar_inum%8 ==6)
		partition.i_bmap[bit_index].b2 = 0;
	else if(tar_inum%8 ==7)
		partition.i_bmap[bit_index].b1 = 0;
}

void do_work(FILE * fd)
{
	char * file_name;
	char command;
	int len,count=0;
	while(fscanf(fd , "%s %c ", file_name, &command) != EOF)
	{
		if(command == 'w' || command == 'r')
		{
			fscanf(fd,"%d\n",&len);
		//	printf("%s %c %d \n",file_name, command,len);
			if(command == 'w'){
				do_write(file_name,len,count);
				count++;
			}
			else{
				do_read(file_name,len);
			}

		}
		else if(command=='d')
		{
		//	printf("%s %c\n",file_name,command);
			do_delete(file_name);
		}
		
	}
}

int main(int argc, char * argv[])
{
	FILE * fd= NULL;

	//initialize
	partition.i_bmap[0].b8 = 1;
	partition.i_bmap[0].b7 = 1;
	partition.i_bmap[0].b6 = 1;

	partition.d_bmap[0].b8 = 1;

	root_inode.fsize = 320;
	root_inode.block = 1;
	for (int i = 0; i < 12; i++)
		root_inode.pointer[i] = 0;
	for (int i = 0; i < 50; i++)
		root_inode.unused[i] = 0;

	partition.i_block[0].inode[0] = root_inode;
	partition.d_block[0] = *((struct d_blocks *)&inode_table);

	//do work
	fd = fopen(argv[1],"r");
	do_work(fd);
	
	//finish work
	printeof();
	fclose(fd);
}
