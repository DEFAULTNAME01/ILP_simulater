#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "function.h"
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

int32_t MEM[1024][4];//the memory 4 channel 

Cache INSTR[512];
int32_t RF[64];
#define QUEUE_SIZE 4
#define THREAD_NUM 4

_Bool full = 0;

 IssueQueue buffer_Alu[QUEUE_SIZE];

 IssueQueue buffer_JMP[QUEUE_SIZE];


 IssueQueue buffer_RWU[QUEUE_SIZE];

int buffer_Alu_head, buffer_Alu_tail;
int buffer_JMP_head, buffer_JMP_tail ;
int buffer_RWU_head, buffer_RWU_tail ;
// The reorder buffer



// pthread_mutex_t lock;
IssueQueue Isq;
int PC,next_PC;


// void* execute_instructions(void *arg) {
//     _Bool finished = 0;
// int error = 0;
//   pthread_mutex_lock(&lock);
//   Execute_ALU(Isq, RF, MEM,  &finished, &error);
//   pthread_mutex_unlock(&lock);
//   return NULL;
// }

// void* execute_readwrite(void *arg) {
//     _Bool finished = 0;
// int error = 0;
//   pthread_mutex_lock(&lock);
//   Execute_ReadWrite(Isq, RF, MEM, &finished, &error);
//   pthread_mutex_unlock(&lock);
//   return NULL;
// }

// void* jump_control(void *arg) {
//     _Bool finished = 0;
// int error = 0;
//   pthread_mutex_lock(&lock);
//   Jump_Control(Isq, RF, MEM, PC, &next_PC, &finished, &error);
//   pthread_mutex_unlock(&lock);
//   return NULL;
// }
void insert(IssueQueue* buffer,IssueQueue* Isq, int* head, int* tail, int* count, _Bool* full) {
    // 如果队列未满，则可以插入新的指令
    if (*count < QUEUE_SIZE&& Isq->valid) {
        // TODO: 插入指令到队列中
        buffer[*tail].Opcode = Isq->Opcode;
        buffer[*tail].s1 = Isq->s1;
        buffer[*tail].s2 = Isq->s2;
        buffer[*tail].r = Isq->r;
        buffer[*tail].valid = 1;
        buffer[*tail].f = Isq->f;
        // 更新队列指针和计数器
        *tail = (*tail + 1) % QUEUE_SIZE;
        (*count)++;
    }
    // 如果队列已满，则输出full信号
    else{
    
        printf("full\n");
        *full = 1;
    }
    
    // TODO: 执行指令并更新队列中指令的valid标志位
}

void isq_remove(IssueQueue* buffer, int* head, int* tail, int* count, _Bool* full) {
    // 如果队列非空，则可以删除队列中的元素
    if (*count > 0) {
        // TODO: 从队列中取出元素
        buffer[*head].valid = 0;
        *head = (*head + 1) % QUEUE_SIZE;
        (*count)--;
    }
    // TODO: 执行指令并更新队列中指令的valid标志位
    
    // 如果队列已被清空，则输出ib_invalid信号
    if (*count == 0 && *full) {
        printf("ib_invalid\n");
        *full = 0;
    }
}
int main(int argc, char *argv[]){
    // Variables initial
    init_memory();
    
 
   bool  buffer_Alu_full=0,buffer_Jump_full=0,buffer_RWU_full=0;
   int buffer_Alu_count,buffer_Jump_count,buffer_RWU_count;
    int executed_instructions=0;
    int executeda,executedb,executedc = 0; // 3.1 the number of instructions executed
    int total_cycles = 0;  //3.2 the number of cycles taken for the whole run
    double per_cycles =0;
    bool continuous_run = false;
    int ch;
    bool is_busy,fetched_valid = false;
   
    uint32_t opcode, op1, op2, op3;
    bool finished = false;
    int cycles = 0;
    int instructions = 0;
    int PC = 0;//where the program start the initial address
    int next_PC,error =0;
    int32_t fetched=0;
    IssueQueue Isq;
    if (argc < 2) {
        printf("Usage: %s <instructions_file>\n", argv[0]);
        return 1;
    }
    // Load instructions from the file
    if (load_program(argv[1], MEM, 1024) != 0) {
        printf("Error: Failed to load program\n");
        return 1;
    }

    // // Call fetch for all instructions in MEM
    for (int i = 0; i < 1024; i++) {
        if (MEM[i][0] == 0 && MEM[i][1] == 0 && MEM[i][2] == 0 && MEM[i][3] == 0) {
            break; // Stop calling Ld_cache when an empty instruction is encountered
        }
        Ld_cache(MEM, INSTR, i);
    }

    //Print INSTR array contents 打印整个缓存
    for (int i = 0; i < 512; i++) {
        printf("%02X ", INSTR[i].INST);
        printf("%02X ", INSTR[i].MeM_addr);
    }
    
  
   
    pthread_t threads[THREAD_NUM];
    int input_data, i, j;
 
    // Outer loop for handling user input
   
    // Wait for user input
     ch = getchar();
    if (ch == 'p'||ch == 'P') {
        continuous_run = true;
    } else if (ch == 'L'||ch == 'l') {
        continuous_run = false;
    }

    // 在程序开头记录开始时间
    time_t start_time = clock();
     // Main loop
    // Main loop
    while (!finished ) {
        int PC_RW,PC_Alu,PC_Jmp=0;
    
        //pipelines
        fetch(INSTR[PC], &is_busy, PC, &fetched,&fetched_valid);
        total_cycles++;
        decode(fetched, &Isq,fetched_valid);
        total_cycles++;
        
        switch(Isq.f){
            case (ReadWrite):
                while (!buffer_RWU_full) {
                insert( buffer_RWU, &Isq, &buffer_RWU_head,&buffer_RWU_tail, &buffer_RWU_count, &buffer_RWU_full);}
                Execute_ReadWrite(buffer_RWU[buffer_RWU_head], RF, MEM, &finished, &error);executeda++,PC_RW=1;
                 isq_remove( buffer_RWU,  &buffer_RWU_head,&buffer_RWU_tail, &buffer_RWU_count, &buffer_RWU_full);break;
                 
            case (Jump):
                while (!buffer_Jump_full)
                {
                 insert( buffer_JMP, &Isq, &buffer_JMP_head,&buffer_JMP_tail, &buffer_Jump_count, &buffer_Jump_full);
                }
                Jump_Control(buffer_JMP[buffer_JMP_head], RF, MEM, PC, &next_PC, &finished, &error);executedb++;PC_Jmp=1;
                isq_remove(buffer_JMP,&buffer_JMP_head, &buffer_JMP_tail, &buffer_Jump_count, &buffer_Jump_full);break;
            case (Alu):
                while (!buffer_Jump_full)
                {     
                insert( buffer_Alu, &Isq, &buffer_Alu_head,&buffer_Alu_tail, &buffer_Alu_count, &buffer_Alu_full); }
                Execute_ALU(buffer_Alu[buffer_Alu_head], RF, MEM, &finished, &error);executedc++;PC_Alu=1;
                isq_remove(buffer_Alu,&buffer_Alu_head, &buffer_Alu_tail, &buffer_Alu_count, &buffer_Alu_full);break;
                
            default:
            printf("Unknown Isq.F: %d\n", Isq.f);
            break;
	    }
       total_cycles++;
        printf("PC:%d,PC_RW:%d",PC,PC_RW);
               next_PC =PC+PC_RW+PC_Alu;

        //Execute(Isq, RF, MEM, PC, &next_PC, &finished, &error);

        PC=next_PC;
        
       // printf("RF[1]:%d,RF[2]:%d",RF[1],RF[2]);
      
        if (error == 1) {
            // Handle the error, for example, print an error message and exit the program
            printf("An error occurred during instruction execution.");
            finished = 1;
        }

        // refresh PC register state machine 更新程序计数器（PC）寄存器
        
      
        write_memory_to_file("output.txt", MEM, 1024);

        if (ch == 'L'||ch == 'l') {//Breakpoint running
            if (getchar()=='k'){//exit 
           
            break;
            }
        }else if (ch == 'p'||ch == 'P') {//Continuous operation
        continue;
        } 
    
    }
    // 程序结束时记录结束时间
    time_t end_time = clock();
    // 计算按下'p'到程序结束的时间差
     double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC * 1000.0;
    // Display results
     printf("running_times : %.4f ms\n",elapsed_time);
    //printf("Calculate result : %02X\n", RF[03]); 
    printf("total_cycles : %d\n",total_cycles);
    executed_instructions=(float)executeda+executedb+executedc;
    printf("executeda : %d\n",executeda);
    printf("executed_instructions : %d\n",executed_instructions);
    per_cycles =(float)executed_instructions/total_cycles;
    printf("instructions per cycles : %f\n",per_cycles);
    ///correct_branch_rate
    //free(rename_table);
    //free(regs);
    return 0;
}