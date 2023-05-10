#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "function.h"
#include <stdlib.h>
#include <time.h>


int32_t MEM[1024][4];//the memory 4 channel 

Cache INSTR[512];
int32_t RF[32];
int main(int argc, char *argv[]){
    // Variables initial
    init_memory();
    init_globals();
   
    int executed_instructions = 0; // 3.1 the number of instructions executed
    int total_cycles = 0;  //3.2 the number of cycles taken for the whole run
    double per_cycles =0;
    bool continuous_run = false;
    int ch;
    bool is_busy = false;
    DecodedInstruction decoded;
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
    while (!finished ) {
        //pipelines
        fetch(INSTR[PC], &is_busy, PC, &fetched);
        total_cycles++;
        decode(fetched, &Isq);
        total_cycles++;
        Execute(Isq, RF, MEM, PC, &next_PC, &finished, &error);
        printf("RF[1]:%d,RF[2]:%d,RF[3]:%d",RF[1],RF[2],RF[3]);
        executed_instructions++;
        total_cycles++;
        if (error == 1) {
            // Handle the error, for example, print an error message and exit the program
            printf("An error occurred during instruction execution.");
            finished = 1;
        }

        // refresh PC register state machine 更新程序计数器（PC）寄存器
        PC = next_PC;
      
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
    printf("executed_instructions : %d\n",executed_instructions);
    per_cycles =(float)executed_instructions/total_cycles;
    printf("instructions per cycles : %f\n",per_cycles);
    ///correct_branch_rate
    free(rename_table);
    free(regs);
    return 0;
}
