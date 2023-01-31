/*
 * apex_cpu.h
 * Contains APEX cpu pipeline declarations
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#ifndef _APEX_CPU_H_
#define _APEX_CPU_H_

#include "apex_macros.h"

/* Format of an APEX instruction  */
typedef struct APEX_Instruction
{
    char opcode_str[128];
    int opcode;
    int rd;
    int rs1;
    int rs2;
    int imm;
} APEX_Instruction;

/* Model of CPU stage latch */
typedef struct CPU_Stage
{
    int trial;
    int pc;
    char opcode_str[128];
    int opcode;
    int rs1;
    int rs2;
    int rd;
    int ps1;
    int ps2;
    int pd;
    int imm;
    int rs1_value;
    int rs2_value;
    int rd_value;
    int ps1_value;
    int ps2_value;
    int result_buffer;
    int memory_address;
    int has_insn;
    int stalled;
    int flush;
    int instype;

    // int zero_flag;

} CPU_Stage;

/* Model of APEX CPU */
typedef struct APEX_CPU
{
    int pc;                  /* Current program counter */
    int clock;               /* Clock cycles elapsed */
    int insn_completed;      /* Instructions retired */
    int regs[REG_FILE_SIZE]; /* Integer register file */
    int regs_valid[REG_FILE_SIZE];
    int pregs_valid[PREGS_FILE_SIZE];
    int mem_valid[4096];
    int code_memory_size;              /* Number of instruction in the input file */
    APEX_Instruction *code_memory;     /* Code Memory */
    int data_memory[DATA_MEMORY_SIZE]; /* Data Memory */
    int single_step;                   /* Wait for user input after every cycle */
    int zero_flag;
    int branch_taken_stage; /* {TRUE, FALSE} Used by BZ and BNZ to branch */
    int fetch_from_next_cycle;
    int renameTableValues[PREGS_FILE_SIZE + 1];
    int branch_taken;
    int jump_inst;
    int jumpval[60000];
    int cmp_completed;
    int mulstage;
    int mreadybit[60000];
    int lsInstComplete[60000];
    int cmpvalue[60000];
    int branchreadybit[60000];
    int intbusy;
    int mulbusy;
    int branchcomplete;
    /* Pipeline stages */
    CPU_Stage fetch;
    CPU_Stage decode;
    CPU_Stage dispatch;
    CPU_Stage issueq;
    CPU_Stage intfu;
    CPU_Stage logicalfu;
    CPU_Stage mulfu1;
    CPU_Stage mulfu2;
    CPU_Stage mulfu3;
    CPU_Stage mulfu4;
    // CPU_Stage lsq;
    // CPU_Stage jbu1;
    // CPU_Stage jbu2;
    CPU_Stage dcache;
    CPU_Stage rob;
    CPU_Stage lsq;
    // CPU_Stage memory2;
} APEX_CPU;

APEX_Instruction *create_code_memory(const char *filename, int *size);
APEX_CPU *APEX_cpu_init(const char *filename);
void APEX_cpu_run(APEX_CPU *cpu);
// int APEX_run_at_choice(APEX_CPU *cpu, int z);
void APEX_cpu_stop(APEX_CPU *cpu);
#endif
