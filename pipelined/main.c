#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "function.h"
#include <stdlib.h>
#include <time.h>
// 定义流水线寄存器和信号量
// ...

bool continuous_run = false;

bool program_terminated = false;
int32_t MEM[1024][4];//the memory 4 channel 

Cache INSTR[512];
int32_t RF[32];


typedef struct {
    Cache INSTR;
    bool *is_busy ;
    int PC;
    int *next_PC;
    int32_t *fetched;
} FetchArgs;

FetchArgs fetch_args;

typedef struct {
    int32_t fetched;
    bool valid;
} FStructure;
FStructure fetch_register;

typedef struct {
    int32_t fetched; 
    IssueQueue *Isq;
} DecodeArgs;

DecodeArgs decode_args;

typedef struct {
    int32_t fetched;
    bool valid;
}DStructure;

DStructure decode_register;

typedef struct {
   IssueQueue Isq;
   int RF;
   int32_t (*MEM)[4]; 
   int PC;
   int *next_PC;
    bool *finished;
   int *error;
} ExecuteArgs;


ExecuteArgs execute_args;

typedef struct {
    IssueQueue Isq;
    bool valid;
}EStructure;

EStructure execute_register;

sem_t sem_decode, sem_execute,sem_fetch;


void* fetch(Cache INSTR, int PC, int *next_PC,int32_t *fetched) {
    while (continuous_run) {
        sem_wait(&sem_fetch);
        // Fetch逻辑
        // 获取新指令
        printf("Fetching instruction at address %d\n", PC);
		*fetched = INSTR.INST;
        printf("Fetched instruction: %08x\n", *fetched);
		INSTR.j=INSTR.j+1;
        // ...
        fetch_register.valid = true;
    
        printf("Data in pipeline is now valid\n");
        execute_register.valid = false;
        PC=*next_PC;
        sem_post(&sem_decode);
    }
    return NULL;
}

void* decode(int32_t fetched, IssueQueue *Isq) {
    while (continuous_run) {
         while (!fetch_register.valid) {
            // Wait for data to become valid
        }
        sem_wait(&sem_decode);
        // Decode逻辑
        int op3 = fetched & 0xff;             // 从指令中提取操作数3,直接取8位
        int op2 = (fetched >> 8) & 0xff;     // 从指令中提取操作数2，右移8位取8位
        int op1 = (fetched >> 16) & 0xff;     // 从指令中提取操作数1,右移16位取8位
        int opcode = (fetched >> 24) & 0xff;  // 从指令中提取opcode,右移24位取剩下的


    printf("Decoded opcode: %x\n", opcode);
    switch(opcode)
	{
	case 0x80:Isq->Opcode=NOP;break;
	case 0x21:Isq->Opcode=ADD,    Isq->s1=op1,  Isq->s2=op2,Isq->r=op3;break;//R[s1]+R[s2]=R[r]
    case 0x11:Isq->Opcode=ADDi,   Isq->s1=op1,  Isq->s2=op2,Isq->r=op3;break;//R[s1]+s2=R[r]
	case 0x22:Isq->Opcode=SUB,    Isq->s1=op1,  Isq->s2=op2,Isq->r=op3;break;//R[s1]-R[s2]=R[r]
    case 0x12:Isq->Opcode=SUBi,   Isq->s1=op1,  Isq->s2=op2,Isq->r=op3;printf("SUBi");break;//R[s1]-s2=R[r]
                
	case 0x23:Isq->Opcode=MUL,    Isq->s1=op1,  Isq->s2=op2,Isq->r=op3;break;//R[s1]xR[s2]=R[r] 
    case 0x13:Isq->Opcode=MULi,   Isq->s1=op1,  Isq->s2=op2,Isq->r=op3;break;//R[s1]xs2=R[r] 
	case 0x24:Isq->Opcode=DIV,    Isq->s1=op1,  Isq->s2=op2,Isq->r=op3;break;//R[s1]/R[s2]=R[r]
    case 0x14:Isq->Opcode=DIVi,   Isq->s1=op1,  Isq->s2=op2,Isq->r=op3;break;//R[s1]/s2=R[r]
	case 0x25:Isq->Opcode=AND,    Isq->s1=op1,  Isq->s2=op2,Isq->r=op3;break;//R[s1]&&R[s2]=R[r]
	case 0x26:Isq->Opcode=OR,     Isq->s1=op1,  Isq->s2=op2,Isq->r=op3;break;//R[s1]||R[s2]=R[r]
	case 0x27:Isq->Opcode=XOR,    Isq->s1=op1,  Isq->s2=op2,Isq->r=op3;break;//R[s1]^R[s2]=R[r]
	case 0x28:Isq->Opcode=NOT,    Isq->s1=op1,  Isq->r=op3; break; //Invert by position store to r
	case 0x29:Isq->Opcode=MOV,    Isq->s1=op1,  Isq->s2=op2; break;// move from R[S1] to R[S2]
    case 0x19:Isq->Opcode=MOVi,   Isq->s1=op1,  Isq->s2=op2; break;// move from R[RF[S1]] to R[RF[S2]]
	case 0x39:Isq->Opcode=LOAD,   Isq->s1=op1,  Isq->s2=op2,Isq->r=op3;break;//IF !s2==0, Loadi r to S1 
	case 0x3a:Isq->Opcode=STORE,  Isq->s1=op1,  Isq->s2=op2,Isq->r=op3;break;// move from R[S1]+s2 to M[r]
	case 0x3b:Isq->Opcode=JMP,    Isq->s1=op1,  Isq->s2=op2,Isq->r=op3;  break;                     //UNCONDITIONAL_JUMP jump to s1
    case 0x3c:Isq->Opcode=JZL,    Isq->s1=op1,  Isq->s2=op2,  Isq->r=op3;break;//CONDITIONAL_JUMP if S1>S2,jump to r
    case 0x2c:Isq->Opcode=JZLi,    Isq->s1=op1,  Isq->s2=op2,  Isq->r=op3;break;//CONDITIONAL_JUMP if RF[S1]>RF[S2],jump to r
	case 0x3d:Isq->Opcode=JZC,    Isq->s1=op1,  Isq->s2=op2,  Isq->r=op3;break; //BRANCH_ZERO iF R[op1]-R[op2]=0,jump to r 
	case 0x1d:Isq->Opcode=JZCi,    Isq->s1=op1,  Isq->s2=op2,  Isq->r=op3;break; //BRANCH_ZERO iF R[op1]-op2=0,jump to r 
    case 0x3e:Isq->Opcode=JNZ,    Isq->s1=op1,  Isq->s2=op2, Isq->r=op3;  break; //BRANCH_NOT_ZERO: if R[S1]-R[S2]!=0 ，jump to r
    case 0x3f:Isq->Opcode=HALT;     break;
    default:
    printf("Unknown opcode: %d\n", opcode);
    break;
	}
        // ...
         fetch_register.valid = false;
        sem_post(&sem_execute);
    }
    return NULL;
}

void* execute(IssueQueue Isq, int *RF, int32_t (*MEM)[4], int PC,int *next_PC, _Bool *finished,int *error) {
    char ch;

    while (!finished) {
        if (!continuous_run) {
           while (!decode_register.valid) {
            // Wait for data to become valid
        }

        sem_wait(&sem_execute);
        // Execute逻辑
        switch(Isq.Opcode)
	{
		case ADD://21
			RF[Isq.r] = RF[Isq.s1] + RF[Isq.s2]; *next_PC=PC + 1; break;
        case SUB://22
			RF[Isq.r] = RF[Isq.s1] - RF[Isq.s2]; *next_PC=PC + 1; break;    

		case MUL://23
			RF[Isq.r] = RF[Isq.s1] * RF[Isq.s2]; *next_PC=PC + 1; break;
        case DIV://24
			RF[Isq.r] = RF[Isq.s1] / RF[Isq.s2]; *next_PC=PC + 1; break;
        case ADDi://11
			RF[Isq.r] = RF[Isq.s1] + Isq.s2; *next_PC=PC + 1; break;
        case SUBi://12
			RF[Isq.r] = RF[Isq.s1] - Isq.s2; *next_PC=PC + 1; break;    
		case MULi://13
			RF[Isq.r] = RF[Isq.s1] * Isq.s2; *next_PC=PC + 1; break;
        case DIVi://14
			RF[Isq.r] = RF[Isq.s1] / Isq.s2; *next_PC=PC + 1; break;
        case AND://25
			RF[Isq.r] = RF[Isq.s1] && RF[Isq.s2]; *next_PC=PC + 1; break; 
        case OR://26
			RF[Isq.r] = RF[Isq.s1] || RF[Isq.s2]; *next_PC=PC + 1; break;  
             
        case XOR://27
			RF[Isq.r] = RF[Isq.s1] ^ RF[Isq.s2]; *next_PC=PC + 1; break;
        case NOT://28
			RF[Isq.r] = ~RF[Isq.s1] ; *next_PC=PC + 1; break;
        case MOV://29
		RF[Isq.s2] =RF[Isq.s1]; *next_PC=PC + 1; break;
         case MOVi://19
                RF[RF[Isq.s2]] = RF[RF[Isq.s1]]; *next_PC=PC + 1; break;//printf("RF[Isq.r]: %x,RF[RF[Isq.r]]:%x\n",RF[Isq.r],RF[(RF[Isq.r])])
		case LOAD://39
        {   
            if (Isq.s2==0)//LOAD  Isq.r to Isq.s1v
            {
                RF[Isq.r] = (MEM[Isq.s1][0] << 24) | (MEM[Isq.s1][1] << 16) | (MEM[Isq.s1][2] << 8) | MEM[Isq.s1][3];
            *next_PC = PC + 1;
            }else{//LOADi//
                RF[Isq.r] = Isq.s1;
            *next_PC = PC + 1;
            }
            
        }
        break;

		case STORE://3a
        {   printf("s1: %d,s2:%d\n,r:%d\n",Isq.s1,Isq.s2,Isq.r);
            int32_t mem_addr = RF[Isq.s1] + Isq.s2-1;//save to memory output line : RF[s1]+s2
            int32_t data = RF[Isq.r];
            MEM[mem_addr][0] = (data >> 24) & 0xFF;
            MEM[mem_addr][1] = (data >> 16) & 0xFF;
            MEM[mem_addr][2] = (data >> 8) & 0xFF;
            MEM[mem_addr][3] = data & 0xFF;
            *next_PC = PC + 1;
        }
        break;
	    case JMP://UNCONDITIONAL_JUMP//3b
			*next_PC = Isq.r; break;

		case JZL://CONDITIONAL_JUMP//3C
			if (RF[Isq.s1] > RF[Isq.s2]) *next_PC = Isq.r ; else *next_PC=PC + 1; break;
        case JZLi://CONDITIONAL_JUMP//2C
			if (RF[(RF[Isq.s1])] > RF[(RF[Isq.s2])]) *next_PC = Isq.r ; else *next_PC=PC + 1; break;
        case JZC://CONDITIONAL_JUMP Equal/compare//3D
			if (RF[Isq.s1] - RF[Isq.s2] == 0) *next_PC = Isq.r ; else *next_PC=PC + 1; break;
        case JZCi://CONDITIONAL_JUMP Equal/compar//1d
	    if (RF[Isq.s1] - Isq.s2 == 0) *next_PC = Isq.r ; else *next_PC=PC + 1; break;
		case JNZ://BRANCH_NOT_ZERO not Equal/compare//3E
			if (RF[Isq.s1]-RF[Isq.s2] != 0) *next_PC = Isq.r; else *next_PC=PC + 1; break;

	
		case HALT://3F
			*finished = 1; *next_PC=PC;printf("Finished OP_mode : HALT") ;break;
        case NOP://80
             *next_PC= PC+1; break;
		default:
			printf("Error: OP_mode not recognised: %d\n",Isq.Opcode); *error = 1; break;
			
	}
        }
        // ...
        decode_register.valid =false;
        sem_post(&sem_fetch);

        // 处理错误和其他逻辑
        // ...
    }
    return NULL;
}

void* control_function(void* arg) {
    while (true) {
        int ch = getchar();
        if (ch == 's' || ch == 'S') {
            continuous_run = false;
            program_terminated = true;
            break;
        }
        if (ch == 'p' || ch == 'P') {
            continuous_run = true;
            sem_post(&sem_fetch);
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    int ch;
    bool is_busy = false;
    pthread_t thread_fetch, thread_decode, thread_execute,thread_control;
    // 在程序开头记录开始时间
    time_t start_time = clock();
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
    
    // 初始化信号量
    sem_init(&sem_fetch, 0, 0);
    sem_init(&sem_decode, 0, 0);
    sem_init(&sem_execute, 0, 0);
    // ...
     // Wait for user input to start the program
    while (true) {
        ch = getchar();
        if (ch == 'p' || ch == 'P') {
            continuous_run = true;
            sem_post(&sem_fetch);
            break;
        }
    }
    // 创建线程
    pthread_create(&thread_fetch, NULL, fetch, (void*)&fetch_args);
    pthread_create(&thread_decode, NULL, decode,(void*)&decode_args);
    pthread_create(&thread_execute, NULL, execute, (void*)&execute_args);
    pthread_create(&thread_control, NULL, control_function, NULL);
   

    // 等待线程结束
    pthread_join(thread_fetch, NULL);
    pthread_join(thread_decode, NULL);
    pthread_join(thread_execute, NULL);
    pthread_join(thread_control, NULL);
    // 程序结束时记录结束时间
    

    // 计算按下'p'到程序结束的时间差
   

    // 显示结果
    time_t end_time = clock();
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC * 1000.0;

    // Display results
    printf("running_times : %.4f ms\n", elapsed_time);
    // printf("total_cycles : %d\n",total_cycles);

    // 销毁信号量
    // ...
    sem_destroy(&sem_fetch);
    sem_destroy(&sem_decode);
    sem_destroy(&sem_execute);
    
    return 0;
}