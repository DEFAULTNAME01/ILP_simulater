#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include "function.h"
// extern IssueQueue buffer_RWU[];
// extern IssueQueue buffer_JMP[];
// extern IssueQueue buffer_Alu[];

// extern int buffer_RWU_head, buffer_RWU_tail, buffer_RWU_count;
// extern _Bool buffer_RWU_full;

// extern int buffer_JMP_head, buffer_JMP_tail, buffer_Jump_count;
// extern _Bool buffer_Jump_full;

// extern int buffer_Alu_head, buffer_Alu_tail, buffer_Alu_count;
// extern _Bool buffer_Alu_full;
void init_memory() {
    // 初始化 Cache INSTR
    for (int i = 0; i < 512; i++) {
        INSTR[i].INST = 0;
        INSTR[i].MeM_addr = 0;
    }

    // 初始化 RF
    for (int i = 0; i < 32; i++) {
        RF[i] = 0;
    }

    // 初始化 MEM
    for (int i = 0; i < 1024; i++) {
        for (int j = 0; j < 4; j++) {
            MEM[i][j] = 0;
        }
    }
    // 初始化 Reorder Buffer 导致占满了
    // for (int i = 0; i < QUEUE_SIZE; i++) {
        
    //     buffer_Alu[i].Opcode = 0;
    //     buffer_Alu[i].f = 0;
    //     buffer_Alu[i].r = 0;
    //     buffer_Alu[i].s1 = 0;
    //     buffer_Alu[i].s2 = 0;
    //     buffer_JMP[i].Opcode = 0;
    //     buffer_JMP[i].f = 0;
    //     buffer_JMP[i].r = 0;
    //     buffer_JMP[i].s1 = 0;
    //     buffer_JMP[i].s2 = 0;
    //     buffer_RWU[i].Opcode = 0;
    //     buffer_RWU[i].f = 0;
    //     buffer_RWU[i].r = 0;
    //     buffer_RWU[i].s1 = 0;
    //     buffer_RWU[i].s2 = 0;
    // }
    
    // 初始化 Reorder Buffer 头尾指针和计数器
    // int buffer_Alu_head = 0;
    // int buffer_Alu_tail = 0;
    // int buffer_JMP_head = 0;
    // int buffer_JMP_tail = 0;
    // int buffer_RWU_head = 0;
    // int buffer_RWU_tail = 0;
}
int load_program(const char *filename, int MEM[][4], int max_instr) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Cannot open file %s\n", filename);
        return 1;
    }
    
    int i = 0;
    while (i < max_instr && fscanf(file, "%x %x %x %x", &MEM[i][0], &MEM[i][1], &MEM[i][2], &MEM[i][3]) == 4) {
        i++;
    }

    fclose(file);
    return 0;
}

void Ld_cache(int32_t MEM[][4], Cache INSTR[], int P_m) {
    for (int i = 0; i < 4; i++) {
        // 将4个内存单元组合成一个32位值
        INSTR[P_m].INST = (MEM[P_m][0] << 24) | (MEM[P_m][1] << 16) | (MEM[P_m][2] << 8) | MEM[P_m][3];
        INSTR[P_m].MeM_addr = P_m;
        INSTR[P_m].j = 0;
        
    }
    printf("INSTR[%d].INST: %08X\n", P_m , INSTR[P_m].INST);
}
void fetch(Cache INSTR, bool *is_busy , int PC,int32_t *fetched,bool *fetch_valid){
	if (!*is_busy) {
        // 获取新指令
        printf("Fetching instruction at address %d\n", PC);
		*fetched = INSTR.INST;
        printf("Fetched instruction: %08x\n", *fetched);
		INSTR.j=INSTR.j+1;
       *fetch_valid =true; // 设置输出数据为有效状态
    }
    if(*is_busy){
        printf("blocked in buffer");
        *fetch_valid =false;
        usleep(5);
    }
    
}

void decode(int32_t fetched,IssueQueue *Isq,bool fetch_valid) {
    while (!fetch_valid) {} // 等待输入数据
    int op3 = fetched & 0xff;             // 从指令中提取操作数3,直接取8位
    int op2 = (fetched >> 8) & 0xff;     // 从指令中提取操作数2，右移8位取8位
    int op1 = (fetched >> 16) & 0xff;     // 从指令中提取操作数1,右移16位取8位
    int opcode = (fetched >> 24) & 0xff;  // 从指令中提取opcode,右移24位取剩下的


    printf("Decoded opcode: %x\n", opcode);
    switch(opcode)
	{
	case 0x80:Isq->Opcode=NOP,Isq->f=Jump;break;
	case 0x21:Isq->Opcode=ADD,  Isq->f=Alu,  Isq->s1=op1,  Isq->s2=op2,Isq->r=op3;break;//R[s1]+R[s2]=R[r]
    case 0x11:Isq->Opcode=ADDi, Isq->f=Alu,  Isq->s1=op1,  Isq->s2=op2,Isq->r=op3;break;//R[s1]+s2=R[r]
	case 0x22:Isq->Opcode=SUB,  Isq->f=Alu,  Isq->s1=op1,  Isq->s2=op2,Isq->r=op3;break;//R[s1]-R[s2]=R[r]
    case 0x12:Isq->Opcode=SUBT, Isq->f=Alu,  Isq->s1=op1,  Isq->s2=op2,Isq->r=op3;break;//R[s1]-s2=R[r]
                
	case 0x23:Isq->Opcode=MUL,  Isq->f=Alu,  Isq->s1=op1,  Isq->s2=op2,Isq->r=op3;break;//R[s1]xR[s2]=R[r] 
    case 0x13:Isq->Opcode=MULi, Isq->f=Alu,  Isq->s1=op1,  Isq->s2=op2,Isq->r=op3;break;//R[s1]xs2=R[r] 
	case 0x24:Isq->Opcode=DIV,  Isq->f=Alu,  Isq->s1=op1,  Isq->s2=op2,Isq->r=op3;break;//R[s1]/R[s2]=R[r]
    case 0x14:Isq->Opcode=DIVi, Isq->f=Alu,  Isq->s1=op1,  Isq->s2=op2,Isq->r=op3;break;//R[s1]/s2=R[r]
	case 0x25:Isq->Opcode=AND,  Isq->f=Alu,  Isq->s1=op1,  Isq->s2=op2,Isq->r=op3;break;//R[s1]&&R[s2]=R[r]
	case 0x26:Isq->Opcode=OR,   Isq->f=Alu,  Isq->s1=op1,  Isq->s2=op2,Isq->r=op3;break;//R[s1]||R[s2]=R[r]
	case 0x27:Isq->Opcode=XOR,  Isq->f=Alu,  Isq->s1=op1,  Isq->s2=op2,Isq->r=op3;break;//R[s1]^R[s2]=R[r]
	case 0x28:Isq->Opcode=NOT,  Isq->f=Alu,  Isq->s1=op1,  Isq->r=op3; break; //Invert by position store to r
	case 0x29:Isq->Opcode=MOV,  Isq->f=ReadWrite,  Isq->s1=op1,  Isq->s2=op2; break;// move from R[S1] to R[S2]
    case 0x19:Isq->Opcode=MOVi, Isq->f=ReadWrite,  Isq->s1=op1,  Isq->s2=op2; break;// move from R[RF[S1]] to R[RF[S2]]
	case 0x39:Isq->Opcode=LOAD, Isq->f=ReadWrite,  Isq->s1=op1,  Isq->s2=op2,Isq->r=op3;printf("get opcode\n");break;//IF !s2==0, Loadi r to S1 
	case 0x3a:Isq->Opcode=STORE,Isq->f=ReadWrite, Isq->s1=op1,  Isq->s2=op2,Isq->r=op3;break;// move from R[S1]+s2 to M[r]
	case 0x3b:Isq->Opcode=JMP,  Isq->f=Jump,  Isq->s1=op1,  Isq->s2=op2,Isq->r=op3;  break;                     //UNCONDITIONAL_JUMP jump to s1
    case 0x3c:Isq->Opcode=JZL,  Isq->f=Jump,  Isq->s1=op1,  Isq->s2=op2,  Isq->r=op3;break;//CONDITIONAL_JUMP if S1>S2,jump to r
    case 0x2c:Isq->Opcode=JZLi, Isq->f=Jump,  Isq->s1=op1,  Isq->s2=op2,  Isq->r=op3;break;//CONDITIONAL_JUMP if RF[S1]>RF[S2],jump to r
	case 0x3d:Isq->Opcode=JZC,  Isq->f=Jump,  Isq->s1=op1,  Isq->s2=op2,  Isq->r=op3;break; //BRANCH_ZERO iF R[op1]-R[op2]=0,jump to r 
	case 0x1d:Isq->Opcode=JZCi, Isq->f=Jump,   Isq->s1=op1,  Isq->s2=op2,  Isq->r=op3;break; //BRANCH_ZERO iF R[op1]-op2=0,jump to r 
    case 0x3e:Isq->Opcode=JNZ,  Isq->f=Jump,  Isq->s1=op1,  Isq->s2=op2, Isq->r=op3;  break; //BRANCH_NOT_ZERO: if R[S1]-R[S2]!=0 ，jump to r
    case 0x3f:Isq->Opcode=HALT, Isq->f=Jump;   break;
    default:
    printf("Unknown opcode: %d\n", opcode);
    break;
	}
    Isq->valid = 1; // 设置输出数据为有效状态
}


#define QUEUE_SIZE 4

void write_memory_to_file(const char* filename, int (*Mem)[4], uint32_t size) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        printf("Error: Failed to open file for writing\n");
        return;
    }

    for (uint32_t i = 0; i < size; ++i) {
        fprintf(fp, "%x %x %x %x\n", Mem[i][0], Mem[i][1], Mem[i][2], Mem[i][3]);
    }

    fclose(fp);
}
void Execute_ALU(IssueQueue Isq, int *RF, int32_t (*MEM)[4],  _Bool *finished,int *error)

{ 
bool is_busy;
int32_t fetched;
bool fetched_valid;

  printf("OPcode: %d, s1: %d, s2: %d, r: %d\n", Isq.Opcode, Isq.s1, Isq.s2, Isq.r);

  while (!Isq.valid) {} // 等待输入数据
	switch(Isq.Opcode)
	{
		case ADD://21
			RF[Isq.r] = RF[Isq.s1] + RF[Isq.s2];  break;
        case SUB://22
			RF[Isq.r] = RF[Isq.s1] - RF[Isq.s2];  break;    
        case SUBT://12
			RF[Isq.r] = RF[Isq.s1] - Isq.s2; printf("s1: %d,s2:%d\n",Isq.s1,Isq.s2); break; 
		case MUL://23
			RF[Isq.r] = RF[Isq.s1] * RF[Isq.s2];  break;
        case DIV://24
			RF[Isq.r] = RF[Isq.s1] / RF[Isq.s2]; break;
        case ADDi://11
			RF[Isq.r] = RF[Isq.s1] + Isq.s2;  break;
         
		case MULi://13
			RF[Isq.r] = RF[Isq.s1] * Isq.s2;  break;
        case DIVi://14
			RF[Isq.r] = RF[Isq.s1] / Isq.s2;  break;
        case AND://25
			RF[Isq.r] = RF[Isq.s1] && RF[Isq.s2];  break;  
        case OR://26
			RF[Isq.r] = RF[Isq.s1] || RF[Isq.s2];  break;  
             
        case XOR://27
			RF[Isq.r] = RF[Isq.s1] ^ RF[Isq.s2];  break;
        case NOT://28
			RF[Isq.r] = ~RF[Isq.s1] ;  break;
        default:
		printf("Error: OP_mode not recognised.ALU: %d\n",Isq.Opcode);for (int i = 0; i < 32; i++) {
            printf("RF[%d]: %d\n", i, RF[i]);
            } *error = 1; break;
	}
	
}
void Execute_ReadWrite(IssueQueue Isq, int *RF, int32_t (*MEM)[4], _Bool *finished,int *error){
    while (!Isq.valid) {} // 等待输入数据
    switch(Isq.Opcode)
	{case MOV://29
		RF[Isq.s2] =RF[Isq.s1];  break;
         case MOVi://19
                RF[RF[Isq.s2]] = RF[RF[Isq.s1]];  break;//printf("RF[Isq.r]: %x,RF[RF[Isq.r]]:%x\n",RF[Isq.r],RF[(RF[Isq.r])])
		case LOAD://39
        {   
            if (Isq.s2==0)//LOAD  Isq.r to Isq.s1v
            {
                RF[Isq.r] = (MEM[Isq.s1][0] << 24) | (MEM[Isq.s1][1] << 16) | (MEM[Isq.s1][2] << 8) | MEM[Isq.s1][3];
            
            }else{//LOADi//
                RF[Isq.r] = Isq.s1;
            
            }
            
        }
        break;

		case STORE://3a
        {   printf("s1: %d,s2:%d\n",Isq.s1,Isq.s2);
            int32_t mem_addr = RF[Isq.s1] + Isq.s2-1;//save to memory output line : RF[s1]+s2
            int32_t data = RF[Isq.r];
            MEM[mem_addr][0] = (data >> 24) & 0xFF;
            MEM[mem_addr][1] = (data >> 16) & 0xFF;
            MEM[mem_addr][2] = (data >> 8) & 0xFF;
            MEM[mem_addr][3] = data & 0xFF;
            
        }
        break;
        default:
			printf("Error: OP_mode not recognised.RW: %d\n",Isq.Opcode); *error = 1; break;
    }

}
void Jump_Control(IssueQueue Isq, int *RF, int32_t (*MEM)[4], int PC,int *next_PC, _Bool *finished,int *error){
   printf("Start executing instructions from address %d\n", PC);
    while (!Isq.valid) {} // 等待输入数据
    switch(Isq.Opcode){
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
			printf("Error: OP_mode not recognised.JC: %d\n",Isq.Opcode); *error = 1; break;
}
    
}
// void Execute(IssueQueue Isq, int *RF, int32_t (*MEM)[4], int PC,int *next_PC, _Bool *finished,int *error)
// {
	
// 	switch(Isq.Opcode)
// 	{
// 		case ADD://21
// 			RF[Isq.r] = RF[Isq.s1] + RF[Isq.s2]; *next_PC=PC + 1; break;
//         case SUB://22
// 			RF[Isq.r] = RF[Isq.s1] - RF[Isq.s2]; *next_PC=PC + 1; break;    

// 		case MUL://23
// 			RF[Isq.r] = RF[Isq.s1] * RF[Isq.s2]; *next_PC=PC + 1; break;
//         case DIV://24
// 			RF[Isq.r] = RF[Isq.s1] / RF[Isq.s2]; *next_PC=PC + 1; break;
//         case ADDi://11
// 			RF[Isq.r] = RF[Isq.s1] + Isq.s2; *next_PC=PC + 1; break;
//         case SUBT://12
// 			RF[Isq.r] = RF[Isq.s1] - Isq.s2; *next_PC=PC + 1; break;    
// 		case MULi://13
// 			RF[Isq.r] = RF[Isq.s1] * Isq.s2; *next_PC=PC + 1; break;
//         case DIVi://14
// 			RF[Isq.r] = RF[Isq.s1] / Isq.s2; *next_PC=PC + 1; break;
//         case AND://25
// 			RF[Isq.r] = RF[Isq.s1] && RF[Isq.s2]; *next_PC=PC + 1; break; 
//         case OR://26
// 			RF[Isq.r] = RF[Isq.s1] || RF[Isq.s2]; *next_PC=PC + 1; break;  
             
//         case XOR://27
// 			RF[Isq.r] = RF[Isq.s1] ^ RF[Isq.s2]; *next_PC=PC + 1; break;
//         case NOT://28
// 			RF[Isq.r] = ~RF[Isq.s1] ; *next_PC=PC + 1; break;
//         case MOV://29
// 		RF[Isq.s2] =RF[Isq.s1]; *next_PC=PC + 1; break;
//          case MOVi://19
//                 RF[RF[Isq.s2]] = RF[RF[Isq.s1]]; *next_PC=PC + 1; break;//printf("RF[Isq.r]: %x,RF[RF[Isq.r]]:%x\n",RF[Isq.r],RF[(RF[Isq.r])])
// 		case LOAD://39
//         {   
//             if (Isq.s2==0)//LOAD  Isq.r to Isq.s1v
//             {
//                 RF[Isq.r] = (MEM[Isq.s1][0] << 24) | (MEM[Isq.s1][1] << 16) | (MEM[Isq.s1][2] << 8) | MEM[Isq.s1][3];
//             *next_PC = PC + 1;
//             }else{//LOADi//
//                 RF[Isq.r] = Isq.s1;
//             *next_PC = PC + 1;
//             }
            
//         }
//         break;

// 		case STORE://3a
//         {   printf("s1: %d,s2:%d\n",Isq.s1,Isq.s2);
//             int32_t mem_addr = RF[Isq.s1] + Isq.s2-1;//save to memory output line : RF[s1]+s2
//             int32_t data = RF[Isq.r];
//             MEM[mem_addr][0] = (data >> 24) & 0xFF;
//             MEM[mem_addr][1] = (data >> 16) & 0xFF;
//             MEM[mem_addr][2] = (data >> 8) & 0xFF;
//             MEM[mem_addr][3] = data & 0xFF;
//             *next_PC = PC + 1;
//         }
//         break;
// 	    case JMP://UNCONDITIONAL_JUMP//3b
// 			*next_PC = Isq.r; break;

// 		case JZL://CONDITIONAL_JUMP//3C
// 			if (RF[Isq.s1] > RF[Isq.s2]) *next_PC = Isq.r ; else *next_PC=PC + 1; break;
//         case JZLi://CONDITIONAL_JUMP//2C
// 			if (RF[(RF[Isq.s1])] > RF[(RF[Isq.s2])]) *next_PC = Isq.r ; else *next_PC=PC + 1; break;
//         case JZC://CONDITIONAL_JUMP Equal/compare//3D
// 			if (RF[Isq.s1] - RF[Isq.s2] == 0) *next_PC = Isq.r ; else *next_PC=PC + 1; break;
//         case JZCi://CONDITIONAL_JUMP Equal/compar//1d
// 	    if (RF[Isq.s1] - Isq.s2 == 0) *next_PC = Isq.r ; else *next_PC=PC + 1; break;
// 		case JNZ://BRANCH_NOT_ZERO not Equal/compare//3E
// 			if (RF[Isq.s1]-RF[Isq.s2] != 0) *next_PC = Isq.r; else *next_PC=PC + 1; break;

	
// 		case HALT://3F
// 			*finished = 1; *next_PC=PC;printf("Finished OP_mode : HALT") ;break;
//         case NOP://80
//              *next_PC= PC+1; break;
// 		default:
// 			printf("Error: OP_mode not recognised.ex: %d\n",Isq.Opcode); *error = 1; break;
			
// 	}
	
// }