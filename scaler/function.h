#ifndef FUNCTION_H
#define FUNCTION_H




#define QUEUE_SIZE 4

void init_memory();
extern int32_t MEM[1024][4];//the memory 4 channel 
typedef struct {
  int32_t INST;
  uint16_t MeM_addr;
  uint16_t j;
  int valid;
} Cache;


extern Cache INSTR[512];


extern int32_t RF[64];

typedef enum {
    NOP,ADD,SUB,MUL,DIV,AND,OR,XOR,NOT,MOV,LOAD,STORE,JZC,JNZ,JZL,JMP, HALT,
    ADDi,SUBT,MULi,DIVi,JZCi,MOVi,JZLi,
} OpCode;

typedef enum {
  ReadWrite,Jump,Alu
 }  DEPAETURE;

typedef struct {
    OpCode Opcode;
    int s1;//RF addr1
    int s2;//RF addr2
    int r;//result
    int valid;
    DEPAETURE f;
}IssueQueue;



// The reorder buffer

void* execute_instructions(void* arg) ;



int load_program(const char *filename, int MEM[][4], int max_instr);
void Ld_cache(int32_t MEM[][4],Cache INSTR[], int P_m);
void fetch(Cache INSTR, bool *is_busy,int PC,int32_t *fetched,bool *fetch_valid);
void decode(int32_t fetched, IssueQueue *Isq,bool fetch_valid);
void write_memory_to_file(const char* filename, int (*Mem)[4], uint32_t size);
void write_memory_to_file(const char* filename, int (*Mem)[4], uint32_t size);
//void Execute(IssueQueue Isq, int *RF, int32_t (*MEM)[4], int PC,int *next_PC, _Bool *finished,int *error);

void Execute_ReadWrite(IssueQueue Isq, int *RF, int32_t (*MEM)[4], _Bool *finished,int *error);
void Jump_Control(IssueQueue Isq, int *RF, int32_t (*MEM)[4], int PC,int *next_PC, _Bool *finished,int *error);
void Execute_ALU(IssueQueue Isq, int *RF, int32_t (*MEM)[4], _Bool *finished,int *error);



#endif