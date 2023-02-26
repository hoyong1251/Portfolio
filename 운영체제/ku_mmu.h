void *ku_mmu_init(unsigned int mem_size, unsigned int swap_size); //0:fail

int ku_run_proc(char pid, void ** ku_cr3); //0:success -1:fail

int ku_page_fault(char pid, char va); //0:success -1:fail


unsigned char* ku_mmu_free_pfn=NULL;
unsigned char* ku_mmu_memory=NULL;
unsigned char* ku_mmu_swap_space=NULL;

int FIFO[100]={0,}; // store history
int ku_mmu_pdbr[30]={0,}; //need # of process
int ku_mmu_pmem_size=0;
int ku_mmu_page_size=4;
int ku_mmu_maxPfn=0;
int ku_mmu_swap_size=0;
volatile int front = 0;
volatile int rear = 0;

int swapping();
int getfreePfn(int type);
unsigned char getPte(int k);
int pteTopfn(unsigned char k);
void swap_out(int pfn);
void swap_in(int target, int pmde, int pte_index);

void* ku_mmu_init(unsigned int mem_size, unsigned int swap_size)
{
	ku_mmu_memory=(unsigned char*)malloc(sizeof(unsigned char) * mem_size);
	ku_mmu_pmem_size=mem_size;
	ku_mmu_maxPfn=mem_size/4;
	ku_mmu_free_pfn=(unsigned char*)malloc(sizeof(unsigned char) * ku_mmu_maxPfn);
	
	ku_mmu_swap_space = (unsigned char*)malloc(sizeof(unsigned char) * swap_size);
	ku_mmu_swap_size=swap_size;
	for(int i=0; i<mem_size; i++)
		ku_mmu_memory[i]=0;
	for(int i=0; i<ku_mmu_maxPfn; i++)
		ku_mmu_free_pfn[i]=0;


	return ku_mmu_memory;
}

int ku_run_proc(char pid, void **ku_cr3)
{
	//run new process 
	//create new page_directory
	//change cr3

	if(ku_mmu_pdbr[pid]==0){ //need allocate new page for pagedir
		int page_for_pcb = getfreePfn(2);
		int page_for_pagedir = getfreePfn(2);
		*ku_cr3 = ku_mmu_memory + page_for_pagedir*4;
		ku_mmu_pdbr[pid]= page_for_pagedir; //just like cr3 

		for(int i=0; i<4; i++){
			ku_mmu_memory[page_for_pcb*4+i]=1;
		}
	}
	else{ //no need to alloc new page
		*ku_cr3=ku_mmu_memory + ku_mmu_pdbr[pid]*4;
	}
	return 0;
}

int ku_page_fault(char pid, char va)
{
	int pde_index = va & 192; //11000000
	pde_index = pde_index >> 6;
//	printf("pde_index: %d\n",pde_index);
	int pmde_index = va & 48; //00110000
	pmde_index = pmde_index >> 4;
//	printf("pmde_index: %d\n",pmde_index);
	int pte_index = va & 12; //00001100
	pte_index = pte_index >> 2;
//	printf("pte_index: %d\n",pte_index);
	int pde,pmde,pte;
	
	//pde
	if(ku_mmu_memory[(ku_mmu_pdbr[pid]*4) + pde_index] == 0){
		//create pde
		pde = getfreePfn(2);
		ku_mmu_memory[(ku_mmu_pdbr[pid]*4)+pde_index]=getPte(pde);
	}
	else{
		pde=pteTopfn(ku_mmu_memory[(ku_mmu_pdbr[pid]*4 + pde_index)]);
	}
	
	//pmde
	if(ku_mmu_memory[pde*4 + pmde_index] == 0){
		pmde = getfreePfn(2);
		ku_mmu_memory[pde*4+pmde_index]=getPte(pmde);
	}
	else{
		pmde = pteTopfn(ku_mmu_memory[pde*4 + pmde_index]);
	}
	

	//pte
	if(ku_mmu_memory[pmde*4 + pte_index] == 0){
		pte = getfreePfn(2);
		ku_mmu_memory[pmde*4+pte_index]=getPte(pte);
	}
	else{
		if(ku_mmu_memory[pmde*4 + pte_index] % 2 ==0)
			swap_in(ku_mmu_memory[pmde*4 + pte_index], pmde, pte_index);
		else{
			pte = pteTopfn(ku_mmu_memory[pmde*4 + pte_index]);
		}
	}



	for(int i=0; i<4; i++){
		ku_mmu_memory[pte*4+i]=1;
		ku_mmu_free_pfn[pte]=1; //getfreePfn type ->1
	}
	FIFO[rear]=pte;
	rear++;

//	for(int i=0; i<ku_mmu_maxPfn; i++)
//		printf("free_pfn[%d] : %d \n", i , ku_mmu_free_pfn[i]);
//	for(int i=0; i<ku_mmu_pmem_size; i++){
//		printf("ku_mmu_memory[%d]= %d\n",i,ku_mmu_memory[i]);
//	}



	return 0;
}


int getfreePfn(int type)
{
	int x;

	for(int i=1; i<ku_mmu_maxPfn; i++)
	{
		if(ku_mmu_free_pfn[i]==0)
		{
			ku_mmu_free_pfn[i]=type;
			return i;
		}
	}
	//no free page and have to swap
	//FIFO
	x = swapping();
	ku_mmu_free_pfn[x]=type;
//	printf("complete swap \n");	
	return x;
}

unsigned char getPte(int k)
{
	return (k*4) + 1;
}

int pteTopfn(unsigned char k)
{
	return (k-1) / 4;
}

int swapping()
{
	printf("no free page call swap\n");
//	for(int i=0; i<100; i++)
//		printf("FIFO[%d] : %d \n", i , FIFO[i]);

	int replace_page=FIFO[front++];
	swap_out(replace_page);

	return replace_page;
	
}



void swap_out(int pfn)
{
	int index = getPte(pfn);
//	printf("index :%d \n",index);
	if(pfn==0){
		printf("physical memory is not enough!\n");
		exit(1);
	}
	for(int i=4; i < ku_mmu_pmem_size; i++){
		if(ku_mmu_memory[i]==index){
			ku_mmu_memory[i]-=1; //valid bit  1 -> 0
//			printf("Page_number :%d is swapped out \n", pfn);
		}
	}
	for(int i=0 ; i<4; i++){
		ku_mmu_swap_space[pfn*4 + i] = ku_mmu_memory[pfn*4 + i]; //swap_out
		ku_mmu_memory[pfn*4 +i] = 0;
	}
	ku_mmu_free_pfn[pfn]=0;

//	printf("after swap free_pfn \n");
//	for(int i=0; i<ku_mmu_maxPfn; i++)
//		printf("free_pfn[%d] : %d \n",i,ku_mmu_free_pfn[i]);
//	for(int i=0; i<ku_mmu_pmem_size; i++)
//		printf("ku_mmu_memory[%d] : %d \n",i,ku_mmu_memory[i]);
//	for(int i=0; i<ku_mmu_swap_size; i++)
//		printf("ku_mmu_swap_space[%d] : %d \n" , i, ku_mmu_swap_space[i]);
}

void swap_in(int target_pte, int pmde, int pte_index)
{
	int target = target_pte / 4;
	
	int new_page = getfreePfn(1);
	
//	printf("page frame replace %d -> %d \n", target, new_page);
	for(int i=0; i<4; i++){
		ku_mmu_memory[new_page*4 +i] = ku_mmu_swap_space[target*4 + i]; //swap_in
		ku_mmu_swap_space[target*4 + i] = 0;
	}
	ku_mmu_memory[pmde*4 + pte_index] = getPte(new_page);
}














