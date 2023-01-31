/*
 * apex_cpu.c
 * Contains APEX cpu pipeline implementation
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#include <string.h>
#include "UDstructs.h"
// Reference : https://www.zentut.com/c-tutorial/c-linked-list/

/* Converts the PC(4000 series) into array index for code memory
 *
 * Note: You are not supposed to edit this function
 */
static int
get_code_memory_index_from_pc(const int pc)
{
    return (pc - 4000) / 4;
}

static void
print_instruction(const CPU_Stage *stage)
{
    switch (stage->opcode)
    {
    case OPCODE_STR:
    case OPCODE_ADD:
    case OPCODE_SUB:
    case OPCODE_MUL:
    case OPCODE_DIV:
    case OPCODE_AND:
    case OPCODE_OR:
    case OPCODE_XOR:
    case OPCODE_LDR:
    case OPCODE_CMP:
    {
        printf("%s,R%d,R%d,R%d", stage->opcode_str, stage->rd, stage->rs1,
               stage->rs2);
        break;
    }

    case OPCODE_MOVC:
    {
        printf("%s,R%d,#%d", stage->opcode_str, stage->rd, stage->imm);
        break;
    }

    case OPCODE_LOAD:
    case OPCODE_ADDL:
    case OPCODE_SUBL:

    {
        printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rd, stage->rs1,
               stage->imm);
        break;
    }

    case OPCODE_STORE:
    {
        printf("%s,R%d,R%d,#%d ", stage->opcode_str, stage->rs1, stage->rs2,
               stage->imm);
        break;
    }

    case OPCODE_BZ:
    case OPCODE_BNZ:
    {
        printf("%s,#%d ", stage->opcode_str, stage->imm);
        break;
    }
    case OPCODE_JUMP:
    {
        printf("%s R%d,#%d ", stage->opcode_str, stage->rs1, stage->imm);
        break;
    }

    case OPCODE_HALT:
    {
        printf("%s", stage->opcode_str);
        break;
    }

    case OPCODE_NULL:
    {
        printf(" ");
        break;
    }
    case OPCODE_NOP:
        printf("%s", stage->opcode_str);
    }
}

static void
print_instruction_with_renamed_registers(const CPU_Stage *stage)
{
    switch (stage->opcode)
    {
    case OPCODE_STR:
    case OPCODE_ADD:
    case OPCODE_SUB:
    case OPCODE_MUL:
    case OPCODE_DIV:
    case OPCODE_AND:
    case OPCODE_OR:
    case OPCODE_XOR:
    case OPCODE_LDR:
    case OPCODE_CMP:
    {
        printf("%s,R%d,R%d,R%d\t\t%s,P%d,P%d,P%d", stage->opcode_str, stage->rd, stage->rs1,
               stage->rs2, stage->opcode_str, stage->pd, stage->ps1, stage->ps2);
        break;
    }

    case OPCODE_MOVC:
    {
        printf("%s,R%d,#%d\t\t%s,P%d,#%d", stage->opcode_str, stage->rd, stage->imm, stage->opcode_str, stage->pd, stage->imm);
        break;
    }

    case OPCODE_LOAD:
    case OPCODE_ADDL:
    case OPCODE_SUBL:
    {
        printf("%s,R%d,R%d,#%d\t\t%s,P%d,P%d,#%d", stage->opcode_str, stage->rd, stage->rs1,
               stage->imm, stage->opcode_str, stage->pd, stage->ps1, stage->imm);
        break;
    }

    case OPCODE_STORE:
    {
        printf("%s,R%d,R%d,#%d\t%s,P%d,P%d,#%d", stage->opcode_str, stage->rs1, stage->rs2,
               stage->imm, stage->opcode_str, stage->ps1, stage->ps2, stage->imm);
        break;
    }

    case OPCODE_BZ:
    case OPCODE_BNZ:
    {
        printf("%s,#%d", stage->opcode_str, stage->imm);
        break;
    }

    case OPCODE_HALT:
    {
        printf("%s", stage->opcode_str);
        break;
    }
    case OPCODE_JUMP:
    {
        printf("%s R%d,#%d\t", stage->opcode_str, stage->rs1, stage->imm);
        break;
    }

    case OPCODE_NULL:
    {
        printf(" ");
        break;
    }
    case OPCODE_NOP:
        printf("%s", stage->opcode_str);
    }
}

static void
print_stage_content(const char *name, const CPU_Stage *stage)
{
    printf("%-15s: pc(%d) ", name, stage->pc);

    print_instruction_with_renamed_registers(stage);

    printf("\n");
}

static void
print_stage_content_for_fetch(const char *name, const CPU_Stage *stage)
{
    printf("%-15s: pc(%d) ", name, stage->pc);
    print_instruction(stage);
    printf("\n");
}

static void
print_reg_file(const APEX_CPU *cpu)
{
    int i;

    printf("----------\n%s\n----------\n", "Registers:");

    for (int i = 0; i < REG_FILE_SIZE / 2; ++i)
    {
        printf("R%-3d[%-3d] ", i, cpu->regs[i]);
    }

    printf("\n");

    for (i = (REG_FILE_SIZE / 2); i < REG_FILE_SIZE; ++i)
    {
        printf("R%-3d[%-3d] ", i, cpu->regs[i]);
    }

    printf("\n");
}

static void print_btb(btb_buffer *head)
{
    btb_buffer *cursor = head;
    printf("BTB Entries: \n");
    while (cursor != NULL)
    {
        printf("act_flg:%d \t inst_add:%d \t computed_add: %d \t taken_flg: %d \n", cursor->data.active_flag, cursor->data.inst_pc, cursor->data.computed_address, cursor->data.taken);
        cursor = cursor->next;
    }
}

/*
 * Fetch Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_fetch(APEX_CPU *cpu)
{

    APEX_Instruction *current_ins;

    if (cpu->jump_inst == 1)
    {
        cpu->fetch.flush = 1;
        cpu->fetch.stalled = 1;
    }
    else
    {
        cpu->fetch.flush = 0;
        cpu->fetch.stalled = 0;
    }

    if (cpu->decode.stalled == 1)
    {
        cpu->fetch.stalled = 1;
    }
    else
    {
        cpu->fetch.stalled = 0;
    }

    if (cpu->fetch.flush == 1)
    {
        strcpy(cpu->fetch.opcode_str, " ");
        cpu->fetch.opcode = 0x0;
        cpu->fetch.pc = '\0';
        cpu->fetch.flush = 0;
        return;
    }

    if (cpu->fetch_from_next_cycle == TRUE)
    {
        cpu->fetch_from_next_cycle = FALSE;

        return;
    }
    if (count_btb(btb_head) < 1)
    {
        // continue;
    }
    else
    {
        btb_buffer *cursor = btb_head;
        while (cursor != NULL)
        {
            if (cursor->data.active_flag)
            {
                if (cursor->data.taken)
                {
                    cpu->pc = cursor->data.computed_address;
                    dispose_btb(btb_head);
                    btb_head = NULL;
                    break;
                }
                btb_head = remove_any_btb(btb_head, cursor);
            }
            cursor = cursor->next;
        }
    }

    /* Store current PC in fetch latch */
    cpu->fetch.pc = cpu->pc;

    current_ins = &cpu->code_memory[get_code_memory_index_from_pc(cpu->pc)];
    strcpy(cpu->fetch.opcode_str, current_ins->opcode_str);
    cpu->fetch.opcode = current_ins->opcode;
    cpu->fetch.rd = current_ins->rd;
    cpu->fetch.rs1 = current_ins->rs1;
    cpu->fetch.rs2 = current_ins->rs2;
    cpu->fetch.imm = current_ins->imm;
    if (cpu->fetch.opcode == 0)
    {
        strcpy(cpu->fetch.opcode_str, " ");
        cpu->fetch.pc = '\0';
    }

    if (cpu->fetch.has_insn && (!cpu->fetch.stalled))
    {
        /* Update PC for next instruction */
        cpu->pc += 4;

        /* Copy data from fetch latch to decode latch*/
        if (cpu->decode.stalled == 0)
        {
            cpu->decode = cpu->fetch;
        }
    }

    if (ENABLE_DEBUG_MESSAGES && cpu->fetch.opcode != OPCODE_NULL)
    {
        print_stage_content_for_fetch("Fetch", &cpu->fetch);
    }
}

/*
 * Decode Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_decode(APEX_CPU *cpu)
{
    printf("In decode %d\t\n", cpu->decode.pc);
    // if (cpu->branch_taken_stage == 1)
    // {
    //     hasher *temp = search_physical_register2(rfprf, cpu->dispatch.rd);
    //     if (temp != NULL)
    //     {
    //         printf("In if condition %d\t%d\t%d\t\n", cpu->decode.pc, temp->data.rf_code, temp->data.prf_code);
    //         rfprf = remove_any_hasher(rfprf, temp);
    //         phead = prependIntoPreg(phead, temp->data.prf_code);
    //         cpu->pregs_valid[cpu->dispatch.pd] = 1;
    //         cpu->branch_taken_stage = 0;
    //     }
    // }

    if (cpu->decode.flush == 1)
    {
        strcpy(cpu->decode.opcode_str, " ");
        cpu->decode.opcode = 0x0;
        cpu->decode.pc = 0000;
        cpu->decode.flush = 0;
    }
    if (cpu->decode.stalled == 1)
    {
        cpu->dispatch.flush = 1;
    }
    else
    {
        cpu->dispatch.flush = 0;
    }

    if (cpu->decode.has_insn && (!cpu->decode.stalled))
    {
        switch (cpu->decode.opcode)
        {

        case OPCODE_STR:
        {
            cpu->decode.ps1 = search_physical_register(rfprf, cpu->decode.rs1);
            cpu->decode.ps2 = search_physical_register(rfprf, cpu->decode.rs2);
            cpu->decode.pd = search_physical_register(rfprf, cpu->decode.rd);
            cpu->mreadybit[cpu->decode.pc] = 0;
            cpu->lsInstComplete[cpu->decode.pc] = 0;
            break;
        }

        case OPCODE_STORE:
        {
            cpu->decode.ps1 = search_physical_register(rfprf, cpu->decode.rs1);
            cpu->decode.ps2 = search_physical_register(rfprf, cpu->decode.rs2);
            cpu->mreadybit[cpu->decode.pc] = 0;
            cpu->lsInstComplete[cpu->decode.pc] = 0;
            break;
        }

        case OPCODE_LDR:
        {

            cpu->decode.ps1 = search_physical_register(rfprf, cpu->decode.rs1);
            cpu->decode.ps2 = search_physical_register(rfprf, cpu->decode.rs2);
            cpu->decode.pd = phead->data;
            cpu->pregs_valid[cpu->decode.pd] = 0;
            renametable.rf_code = cpu->decode.rd;
            renametable.prf_code = cpu->decode.pd;
            rfprf = prepend_hasher(rfprf, renametable);
            phead = dequeueReg(phead);
            cpu->mreadybit[cpu->decode.pc] = 0;

            break;
        }

        case OPCODE_LOAD:
        {

            cpu->decode.ps1 = search_physical_register(rfprf, cpu->decode.rs1);
            cpu->decode.pd = phead->data;
            cpu->pregs_valid[cpu->decode.pd] = 0;
            renametable.rf_code = cpu->decode.rd;
            renametable.prf_code = cpu->decode.pd;
            rfprf = prepend_hasher(rfprf, renametable);
            phead = dequeueReg(phead);
            cpu->mreadybit[cpu->decode.pc] = 0;

            break;
        }

        case OPCODE_CMP:
        {
            cpu->decode.ps1 = search_physical_register(rfprf, cpu->decode.rs1);
            cpu->decode.ps2 = search_physical_register(rfprf, cpu->decode.rs2);
            cpu->decode.pd = phead->data;
            cpu->pregs_valid[cpu->decode.pd] = 0;
            renametable.rf_code = cpu->decode.rd;
            renametable.prf_code = cpu->decode.pd;
            rfprf = prepend_hasher(rfprf, renametable);
            phead = dequeueReg(phead);
            break;
        }

        case OPCODE_ADD:
        case OPCODE_SUB:
        case OPCODE_MUL:
        case OPCODE_DIV:
        case OPCODE_AND:
        case OPCODE_OR:
        case OPCODE_XOR:
        {

            cpu->decode.ps1 = search_physical_register(rfprf, cpu->decode.rs1);
            cpu->decode.ps2 = search_physical_register(rfprf, cpu->decode.rs2);
            cpu->decode.pd = phead->data;
            cpu->pregs_valid[cpu->decode.pd] = 0;
            renametable.rf_code = cpu->decode.rd;
            renametable.prf_code = cpu->decode.pd;
            rfprf = prepend_hasher(rfprf, renametable);
            phead = dequeueReg(phead);

            break;
        }

        case OPCODE_ADDL:
        case OPCODE_SUBL:
        {
            cpu->decode.ps1 = search_physical_register(rfprf, cpu->decode.rs1);
            cpu->decode.pd = phead->data;
            cpu->pregs_valid[cpu->decode.pd] = 0;
            renametable.rf_code = cpu->decode.rd;
            renametable.prf_code = cpu->decode.pd;
            rfprf = prepend_hasher(rfprf, renametable);
            phead = dequeueReg(phead);
            break;
        }

        case OPCODE_MOVC:
        {

            cpu->decode.pd = phead->data;
            renametable.rf_code = cpu->decode.rd;
            renametable.prf_code = cpu->decode.pd;
            cpu->pregs_valid[cpu->decode.pd] = 0;
            rfprf = prepend_hasher(rfprf, renametable);
            phead = dequeueReg(phead);
            break;
        }

        case OPCODE_BNZ:
        case OPCODE_BZ:
        {
            btb_buffer *new_node = (btb_buffer *)malloc(sizeof(btb_buffer));
            new_node->data.active_flag = 0;
            new_node->data.inst_pc = cpu->decode.pc;
            new_node->data.computed_address = 0;
            new_node->data.taken = 0;
            btb_head = enqueue_btb(btb_head, new_node->data);
            cpu->branchreadybit[cpu->decode.pc] = 0;
            break;
        }

        case OPCODE_JUMP:
        {
            cpu->jump_inst = 1;
            cpu->fetch.flush = 1;
            break;
        }

        case OPCODE_HALT:
        {

            break;
        }
        default:
        {
            break;
        }
        }
    }

    /* Copy data from decode latch to intfu latch*/
    if (cpu->decode.stalled == 0)
    {
        cpu->dispatch = cpu->decode;
        cpu->decode.has_insn = FALSE;
    }
    else
    {
        cpu->decode.stalled = 1;
        cpu->dispatch.flush = 1;
    }

    if (ENABLE_DEBUG_MESSAGES && cpu->decode.opcode != OPCODE_NULL)
    {
        print_stage_content("Decode/RF", &cpu->decode);
    }
}

static void
APEX_dispatch(APEX_CPU *cpu)
{

    if (cpu->branch_taken_stage == 1 && cpu->dispatch.opcode != 0 && (cpu->dispatch.opcode != 9 && cpu->dispatch.opcode != 16))
    {
        hasher *temp = search_physical_register2(rfprf, cpu->dispatch.rd);

        if (temp != NULL)
        {
            int data = temp->data.prf_code;
            rfprf = remove_any_hasher(rfprf, temp);
            phead = enqueueReg(phead, data);
            cpu->pregs_valid[cpu->dispatch.pd] = 1;
            cpu->branch_taken_stage = 0;
        }
    }
    else
    {
        cpu->branch_taken_stage = 0;
    }
    if (cpu->dispatch.flush == 1)
    {
        strcpy(cpu->dispatch.opcode_str, " ");
        cpu->dispatch.opcode = 0x0;
        cpu->dispatch.pc = 0000;
        cpu->dispatch.flush = 0;
    }
    if (cpu->dispatch.stalled == 1)
    {
        cpu->issueq.flush = 1;
        cpu->rob.flush = 1;
        cpu->decode.stalled = 1;
        cpu->lsq.flush = 1;
    }
    else
    {
        cpu->issueq.flush = 0;
        cpu->rob.flush = 0;
        cpu->decode.stalled = 0;
        cpu->lsq.flush = 0;
    }

    if (cpu->dispatch.stalled == 0)
    {
        switch (cpu->dispatch.opcode)
        {
        case OPCODE_LDR:
        case OPCODE_LOAD:
        {
            if (count(iqhead) < 8 && count(robhead) < 16 && count(lsqhead) < 4)
            {
                cpu->issueq = cpu->dispatch;
                cpu->rob = cpu->dispatch;
                cpu->lsq = cpu->dispatch;
            }
            else
            {
                cpu->dispatch.stalled = 1;
            }
            break;
        }

        case OPCODE_STR:
        case OPCODE_STORE:
        {
            if (count(iqhead) < 8 && count(robhead) < 16 && count(lsqhead) < 4)
            {
                cpu->issueq = cpu->dispatch;
                cpu->rob = cpu->dispatch;
                // cpu->rob.flush = 1;
                cpu->lsq = cpu->dispatch;
            }
            else
            {
                cpu->dispatch.stalled = 1;
            }
            break;
        }
        case OPCODE_JUMP:
        {
            if (count(iqhead) < 8 && count(robhead) < 16)
            {
                cpu->issueq = cpu->dispatch;
                cpu->rob = cpu->dispatch;
                cpu->lsq.flush = 1;
                cpu->decode.flush = 1;
            }
            else
            {
                cpu->dispatch.stalled = 1;
            }
            break;
        }

        default:
        {
            if (count(iqhead) < 8 && count(robhead) < 16)
            {
                cpu->issueq = cpu->dispatch;
                cpu->rob = cpu->dispatch;
                cpu->lsq.flush = 1;
            }
            else
            {
                cpu->dispatch.stalled = 1;
            }
            break;
        }
        }

        cpu->dispatch.has_insn = FALSE;
    }
    if (ENABLE_DEBUG_MESSAGES && cpu->dispatch.opcode != OPCODE_NULL)
    {
        print_stage_content("Dispatch/RF", &cpu->dispatch);
    }
}

static void
APEX_lsq(APEX_CPU *cpu)
{
    if (cpu->branch_taken_stage == 1 && cpu->lsq.opcode != 0xc)
    {
        if (cpu->lsq.opcode != 0x0 && (cpu->lsq.opcode == OPCODE_LDR || cpu->lsq.opcode == OPCODE_LOAD))
        {

            hasher *temp = search_physical_register2(rfprf, cpu->lsq.rd);
            if (temp != NULL)
            {

                int data = temp->data.prf_code;
                rfprf = remove_any_hasher(rfprf, temp);
                phead = enqueueReg(phead, data);
                cpu->pregs_valid[cpu->lsq.pd] = 1;
            }
        }
    }

    if (cpu->lsq.flush == 1)
    {

        strcpy(cpu->lsq.opcode_str, " ");
        cpu->lsq.opcode = 0x0;
        cpu->lsq.pc = 0000;
        cpu->lsq.flush = 0;
    }

    if (cpu->lsq.opcode != 0x0 && cpu->lsq.has_insn == TRUE)
    {
        lsqhead = enqueue(lsqhead, cpu->lsq);
    }

    node *cursor = lsqhead;
    while (cursor != NULL)
    {
        if (ENABLE_DEBUG_MESSAGES && cursor->data.opcode != OPCODE_NULL)
        {
            print_stage_content("LSQ", &cursor->data);
        }
        cursor = cursor->next;
    }

    if (count(lsqhead) != 0)
    {

        node *cursor = lsqhead;

        switch (cursor->data.opcode)
        {

        case OPCODE_STR:
        {

            if (cpu->pregs_valid[cursor->data.pd] && cpu->mreadybit[cursor->data.pc])
            {
                cursor->data.result_buffer = cpu->intfu.result_buffer;
                cpu->mem_valid[cpu->renameTableValues[cursor->data.ps1] + cpu->renameTableValues[cursor->data.ps2]] = 0;
                cpu->dcache = cursor->data;
                lsqhead = remove_any(lsqhead, cursor);
            }
            else
            {
                cpu->dcache.flush = 1;
            }
            break;
        }

        case OPCODE_STORE:
        {
            if (cpu->pregs_valid[cursor->data.ps1] && cpu->mreadybit[cursor->data.pc])
            {
                cursor->data.result_buffer = cpu->intfu.result_buffer;
                cpu->mem_valid[cpu->renameTableValues[cursor->data.ps2] + cpu->renameTableValues[cursor->data.imm]] = 0;
                cpu->dcache = cursor->data;
                lsqhead = remove_any(lsqhead, cursor);
            }
            else
            {
                cpu->dcache.flush = 1;
            }
            break;
        }

        case OPCODE_LDR:
        {
            if (cpu->mreadybit[cursor->data.pc])
            {
                cursor->data.result_buffer = cpu->intfu.result_buffer;
                cpu->pregs_valid[cursor->data.pd] = 0;
                cpu->dcache = cursor->data;
                lsqhead = remove_any(lsqhead, cursor);
            }
            else
            {
                cpu->dcache.flush = 1;
            }
            break;
        }

        case OPCODE_LOAD:
        {
            if (cpu->mreadybit[cursor->data.pc])
            {
                cursor->data.result_buffer = cpu->intfu.result_buffer;
                cpu->pregs_valid[cursor->data.pd] = 0;
                cpu->dcache = cursor->data;
                lsqhead = remove_any(lsqhead, cursor);
            }
            else
            {
                cpu->dcache.flush = 1;
            }
            break;
        }

        default:
        {
            lsqhead = remove_any(lsqhead, cursor);
            break;
        }
        }
    }
    cpu->lsq.has_insn = FALSE;
}

static void
APEX_issueq(APEX_CPU *cpu)
{

    if (cpu->branch_taken_stage == 1 && cpu->issueq.opcode != 0xc)
    {
        if (cpu->issueq.opcode != 0x0)
        {

            hasher *temp = search_physical_register2(rfprf, cpu->issueq.rd);
            if (temp != NULL)
            {
                int data = temp->data.prf_code;
                rfprf = remove_any_hasher(rfprf, temp);
                phead = enqueueReg(phead, data);
                cpu->pregs_valid[cpu->issueq.pd] = 1;
            }
        }
    }
    if (cpu->issueq.flush == 1)
    {
        strcpy(cpu->issueq.opcode_str, " ");
        cpu->issueq.opcode = 0x0;
        cpu->issueq.pc = 0000;
        cpu->issueq.flush = 0;
    }

    if (cpu->issueq.opcode != 0x0 && cpu->issueq.has_insn == TRUE)
    {
        iqhead = enqueue(iqhead, cpu->issueq);
    }
    if (cpu->issueq.opcode == 0xc)
    {
        cpu->dispatch.flush = 1;
        cpu->decode.flush = 1;
    }

    node *cursor = iqhead;
    while (cursor != NULL)
    {
        if (ENABLE_DEBUG_MESSAGES && cursor->data.opcode != OPCODE_NULL)
        {
            print_stage_content("Issueq", &cursor->data);
        }
        cursor = cursor->next;
    }
    int intfuBusyFlag = 0;
    int mulfuBusyFlag = 0;
    int logicalBusyFlag = 0;

    if (count(iqhead) != 0)
    {

        node *cursor = iqhead;

        while (cursor != NULL)
        {
            switch (cursor->data.opcode)
            {

            case OPCODE_STR:
            {

                if (cpu->pregs_valid[cursor->data.ps1] && cpu->pregs_valid[cursor->data.ps2] && cpu->pregs_valid[cursor->data.pd] && intfuBusyFlag != 1)
                {
                    cursor->data.ps1_value = cpu->renameTableValues[cursor->data.ps1];
                    cursor->data.ps2_value = cpu->renameTableValues[cursor->data.ps2];
                    cpu->mem_valid[cursor->data.ps1_value + cursor->data.ps2_value] = 0;
                    cpu->mreadybit[cursor->data.pc] = 0;
                    cpu->intfu = cursor->data;
                    iqhead = remove_any(iqhead, cursor);
                    intfuBusyFlag = 1;
                }
                break;
            }

            case OPCODE_STORE:
            {
                if (cpu->pregs_valid[cursor->data.ps2] && intfuBusyFlag != 1)
                {
                    cursor->data.ps2_value = cpu->renameTableValues[cursor->data.ps2];
                    cpu->mem_valid[cursor->data.ps2_value + cursor->data.imm] = 0;
                    cpu->mreadybit[cursor->data.pc] = 0;
                    cpu->intfu = cursor->data;
                    iqhead = remove_any(iqhead, cursor);
                    intfuBusyFlag = 1;
                }
                break;
            }

            case OPCODE_LDR:
            {
                if (cpu->pregs_valid[cursor->data.ps1] && cpu->pregs_valid[cursor->data.ps2] && intfuBusyFlag != 1)
                {
                    cursor->data.ps1_value = cpu->renameTableValues[cursor->data.ps1];
                    cursor->data.ps2_value = cpu->renameTableValues[cursor->data.ps2];
                    cpu->pregs_valid[cursor->data.pd] = 0;
                    cpu->mreadybit[cursor->data.pc] = 0;
                    cpu->intfu = cursor->data;
                    iqhead = remove_any(iqhead, cursor);
                    intfuBusyFlag = 1;
                }
                break;
            }

            case OPCODE_LOAD:
            {
                if (cpu->pregs_valid[cursor->data.ps1] && intfuBusyFlag != 1)
                {
                    cursor->data.ps1_value = cpu->renameTableValues[cursor->data.ps1];
                    cpu->pregs_valid[cursor->data.pd] = 0;
                    cpu->mreadybit[cursor->data.pc] = 0;
                    cpu->intfu = cursor->data;
                    iqhead = remove_any(iqhead, cursor);
                    intfuBusyFlag = 1;
                }
                break;
            }

            case OPCODE_ADD:
            case OPCODE_SUB:
            case OPCODE_DIV:
            case OPCODE_CMP:
            {
                if (cpu->pregs_valid[cursor->data.ps1] && cpu->pregs_valid[cursor->data.ps2] && intfuBusyFlag != 1)
                {
                    cursor->data.ps1_value = cpu->renameTableValues[cursor->data.ps1];
                    cursor->data.ps2_value = cpu->renameTableValues[cursor->data.ps2];
                    cpu->pregs_valid[cursor->data.pd] = 0;
                    cpu->intfu = cursor->data;
                    iqhead = remove_any(iqhead, cursor);

                    intfuBusyFlag = 1;
                }

                break;
            }
            case OPCODE_AND:
            case OPCODE_OR:
            case OPCODE_XOR:
            {
                if (cpu->pregs_valid[cursor->data.ps1] && cpu->pregs_valid[cursor->data.ps2] && logicalBusyFlag != 1)
                {
                    cursor->data.ps1_value = cpu->renameTableValues[cursor->data.ps1];
                    cursor->data.ps2_value = cpu->renameTableValues[cursor->data.ps2];
                    cpu->pregs_valid[cursor->data.pd] = 0;
                    cpu->logicalfu = cursor->data;
                    iqhead = remove_any(iqhead, cursor);
                    logicalBusyFlag = 1;
                }

                break;
            }

            case OPCODE_JUMP:
            {
                if (intfuBusyFlag != 1)
                {

                    cpu->intfu = cursor->data;
                    iqhead = remove_any(iqhead, cursor);
                    intfuBusyFlag = 1;
                }

                break;
            }

            case OPCODE_BNZ:
            case OPCODE_BZ:
            {
                if (intfuBusyFlag != 1)
                {
                    cpu->intfu = cursor->data;
                    iqhead = remove_any(iqhead, cursor);
                    intfuBusyFlag = 1;
                }
                break;
            }

            case OPCODE_MUL:
            {

                if (cpu->pregs_valid[cursor->data.ps1] && cpu->pregs_valid[cursor->data.ps2] && mulfuBusyFlag != 1)
                {
                    cursor->data.ps1_value = cpu->renameTableValues[cursor->data.ps1];
                    cursor->data.ps2_value = cpu->renameTableValues[cursor->data.ps2];
                    cpu->pregs_valid[cursor->data.pd] = 0;
                    cpu->mulfu1 = cursor->data;
                    iqhead = remove_any(iqhead, cursor);
                    mulfuBusyFlag = 1;
                }

                break;
            }
            case OPCODE_ADDL:
            case OPCODE_SUBL:
            {
                if (cpu->pregs_valid[cursor->data.ps1] && intfuBusyFlag != 1)
                {
                    cursor->data.ps1_value = cpu->renameTableValues[cursor->data.ps1];
                    cpu->pregs_valid[cursor->data.pd] = 0;
                    cpu->intfu = cursor->data;
                    iqhead = remove_any(iqhead, cursor);
                    intfuBusyFlag = 1;
                }

                break;
            }

            case OPCODE_MOVC:
            {
                if (intfuBusyFlag != 1)
                {
                    cpu->intfu = cursor->data;
                    cpu->pregs_valid[cursor->data.pd] = 0;
                    iqhead = remove_any(iqhead, cursor);
                    intfuBusyFlag = 1;
                }
                break;
            }

            default:
            {
                iqhead = remove_any(iqhead, cursor);
                break;
            }
            }
            cursor = cursor->next;
        }
    }
    cpu->decode.stalled = 0;
    cpu->issueq.has_insn = FALSE;
}

static void
APEX_intfu(APEX_CPU *cpu)
{

    if (cpu->intfu.flush == 1)
    {
        strcpy(cpu->intfu.opcode_str, " ");
        cpu->intfu.opcode = 0x0;
        cpu->intfu.pc = 0000;
    }
    if (cpu->intfu.has_insn)
    {
        switch (cpu->intfu.opcode)
        {
        case OPCODE_CMP:
        {

            cpu->intfu.result_buffer = cpu->intfu.ps1_value - cpu->intfu.ps2_value;
            cpu->cmpvalue[cpu->intfu.pc] = cpu->intfu.result_buffer;
            cpu->cmp_completed = 1;
            cpu->renameTableValues[cpu->intfu.pd] = 0;
            cpu->pregs_valid[cpu->intfu.pd] = 1;
            if (cpu->intfu.result_buffer == 0)
            {
                cpu->zero_flag = 0;
            }
            else
            {
                cpu->zero_flag = cpu->intfu.result_buffer;
            }
            break;
        }
        case OPCODE_JUMP:
        {
            cpu->intfu.result_buffer = cpu->intfu.pc + cpu->intfu.imm;
            cpu->branch_taken = 1;
            cpu->jumpval[cpu->intfu.pc] = cpu->intfu.result_buffer;
            cpu->jump_inst = 0;
            break;
        }
        case OPCODE_STR:
        {
            cpu->intfu.result_buffer = cpu->intfu.ps1_value + cpu->intfu.ps2_value;
            cpu->mreadybit[cpu->intfu.pc] = 1;
            break;
        }
        case OPCODE_STORE:
        {
            cpu->intfu.result_buffer = cpu->intfu.ps2_value + cpu->intfu.imm;
            cpu->mreadybit[cpu->intfu.pc] = 1;
            break;
        }
        case OPCODE_LDR:
        {
            cpu->intfu.result_buffer = cpu->intfu.ps1_value + cpu->intfu.ps2_value;
            cpu->mreadybit[cpu->intfu.pc] = 1;
            break;
        }
        case OPCODE_LOAD:
        {
            cpu->intfu.result_buffer = cpu->intfu.ps1_value + cpu->intfu.imm;
            cpu->mreadybit[cpu->intfu.pc] = 1;
            break;
        }

        case OPCODE_ADD:
        {

            cpu->intfu.result_buffer = cpu->intfu.ps1_value + cpu->intfu.ps2_value;
            cpu->renameTableValues[cpu->intfu.pd] = cpu->intfu.result_buffer;
            cpu->pregs_valid[cpu->intfu.pd] = 1;
            break;
        }
        case OPCODE_SUB:
        {
            cpu->intfu.result_buffer = cpu->intfu.ps1_value - cpu->intfu.ps2_value;
            cpu->renameTableValues[cpu->intfu.pd] = cpu->intfu.result_buffer;
            cpu->pregs_valid[cpu->intfu.pd] = 1;

            break;
        }

        case OPCODE_DIV:
        {
            cpu->intfu.result_buffer = cpu->intfu.ps1_value / cpu->intfu.ps2_value;
            cpu->renameTableValues[cpu->intfu.pd] = cpu->intfu.result_buffer;
            cpu->pregs_valid[cpu->intfu.pd] = 1;
            break;
        }
        case OPCODE_ADDL:
        {
            cpu->intfu.result_buffer = (cpu->intfu.ps1_value) + cpu->intfu.imm;
            cpu->renameTableValues[cpu->intfu.pd] = cpu->intfu.result_buffer;
            cpu->pregs_valid[cpu->intfu.pd] = 1;
            break;
        }
        case OPCODE_SUBL:
        {
            cpu->intfu.result_buffer = cpu->intfu.ps1_value - cpu->intfu.imm;
            cpu->renameTableValues[cpu->intfu.pd] = cpu->intfu.result_buffer;
            cpu->pregs_valid[cpu->intfu.pd] = 1;
            break;
        }

        case OPCODE_MOVC:
        {

            cpu->intfu.result_buffer = cpu->intfu.imm;
            cpu->renameTableValues[cpu->intfu.pd] = cpu->intfu.result_buffer;
            cpu->pregs_valid[cpu->intfu.pd] = 1;
            break;
        }

        case OPCODE_BZ:
        {
            cpu->intfu.result_buffer = cpu->intfu.pc + cpu->intfu.imm;
            cpu->branchreadybit[cpu->intfu.pc] = 1;

            if (cpu->zero_flag == 0)
            {
                cpu->branch_taken = 1;

                cpu->rob.flush = 1;
                cpu->issueq.flush = 1;
                cpu->lsq.flush = 1;
                cpu->intfu.flush = 1;
                cpu->mulfu1.flush = 1;
                cpu->mulfu2.flush = 1;
                cpu->mulfu3.flush = 1;
                cpu->mulfu4.flush = 1;
                cpu->dcache.flush = 1;
                cpu->dispatch.flush = 1;
                cpu->decode.flush = 1;
                cpu->branch_taken_stage = 1;
            }
            else
            {
                cpu->branch_taken = 0;
            }
            btb_head = insert_address_btb(btb_head, cpu->intfu, cpu->branch_taken);
            break;
        }
        case OPCODE_BNZ:
        {
            cpu->intfu.result_buffer = cpu->intfu.pc + cpu->intfu.imm;
            cpu->branchreadybit[cpu->intfu.pc] = 1;
            if (cpu->zero_flag > 0 || cpu->zero_flag < 0)
            {
                cpu->branch_taken = 1;

                cpu->rob.flush = 1;
                cpu->issueq.flush = 1;
                cpu->lsq.flush = 1;
                cpu->intfu.flush = 1;
                cpu->mulfu1.flush = 1;
                cpu->mulfu2.flush = 1;
                cpu->mulfu3.flush = 1;
                cpu->mulfu4.flush = 1;
                cpu->dcache.flush = 1;
                cpu->dispatch.flush = 1;
                cpu->decode.flush = 1;
                cpu->branch_taken_stage = 1;
            }
            else
            {
                cpu->branch_taken = 0;
            }
            btb_head = insert_address_btb(btb_head, cpu->intfu, cpu->branch_taken);
            break;
        }

        case OPCODE_HALT:
        {
            cpu->decode.flush = 1;
            cpu->fetch.flush = 1;
            break;
        }
        }

        cpu->intfu.has_insn = FALSE;
        if (ENABLE_DEBUG_MESSAGES && cpu->intfu.opcode != OPCODE_NULL)
        {
            print_stage_content("intfu", &cpu->intfu);
        }
    }
}

static void
APEX_dcache(APEX_CPU *cpu)
{
    if (cpu->dcache.flush == 1)
    {
        strcpy(cpu->dcache.opcode_str, " ");
        cpu->dcache.opcode = 0x0;
        cpu->dcache.pc = 0000;
    }
    if (cpu->dcache.has_insn)
    {
        switch (cpu->dcache.opcode)
        {
        case OPCODE_STR:
        {
            cpu->data_memory[cpu->dcache.result_buffer] = cpu->renameTableValues[cpu->dcache.pd];
            cpu->lsInstComplete[cpu->decode.pc] = 1;
            break;
        }
        case OPCODE_STORE:
        {
            cpu->data_memory[cpu->dcache.result_buffer] = cpu->renameTableValues[cpu->dcache.ps1];
            cpu->lsInstComplete[cpu->decode.pc] = 1;
            break;
        }
        case OPCODE_LDR:
        case OPCODE_LOAD:
        {

            cpu->renameTableValues[cpu->dcache.pd] = cpu->data_memory[cpu->dcache.result_buffer];
            cpu->pregs_valid[cpu->dcache.pd] = 1;
            break;
        }
        }

        cpu->dcache.has_insn = FALSE;
        if (ENABLE_DEBUG_MESSAGES && cpu->dcache.opcode != OPCODE_NULL)
        {
            print_stage_content("dcache", &cpu->dcache);
        }
    }
}

static void
APEX_logicalfu(APEX_CPU *cpu)
{
    if (cpu->logicalfu.flush == 1)
    {
        strcpy(cpu->logicalfu.opcode_str, " ");
        cpu->logicalfu.opcode = 0x0;
        cpu->logicalfu.pc = 0000;
    }
    if (cpu->logicalfu.has_insn)
    {
        switch (cpu->logicalfu.opcode)
        {
        case OPCODE_AND:
        {
            cpu->logicalfu.result_buffer = (cpu->logicalfu.ps1_value) & (cpu->logicalfu.ps2_value);
            cpu->renameTableValues[cpu->logicalfu.pd] = cpu->logicalfu.result_buffer;
            cpu->pregs_valid[cpu->logicalfu.pd] = 1;
            break;
        }
        case OPCODE_OR:
        {
            cpu->logicalfu.result_buffer = cpu->logicalfu.ps1_value | cpu->logicalfu.ps2_value;
            cpu->renameTableValues[cpu->logicalfu.pd] = cpu->logicalfu.result_buffer;
            cpu->pregs_valid[cpu->logicalfu.pd] = 1;
            break;
        }
        case OPCODE_XOR:
        {
            cpu->logicalfu.result_buffer = (cpu->logicalfu.ps1_value) ^ (cpu->logicalfu.ps2_value);
            cpu->renameTableValues[cpu->logicalfu.pd] = cpu->logicalfu.result_buffer;
            cpu->pregs_valid[cpu->logicalfu.pd] = 1;
            break;
        }
        }

        cpu->logicalfu.has_insn = FALSE;
        if (ENABLE_DEBUG_MESSAGES && cpu->logicalfu.opcode != OPCODE_NULL)
        {
            print_stage_content("logicalfu", &cpu->logicalfu);
        }
    }
}

static void
APEX_mulfu1(APEX_CPU *cpu)
{
    if (cpu->mulfu1.flush == 1)
    {
        strcpy(cpu->mulfu1.opcode_str, " ");
        cpu->mulfu1.opcode = 0x0;
        cpu->mulfu1.pc = 0000;
    }

    if (cpu->mulfu1.has_insn)
    {

        switch (cpu->mulfu1.opcode)
        {
        case OPCODE_MUL:
        {
            cpu->mulfu1.result_buffer = cpu->mulfu1.ps1_value * cpu->mulfu1.ps2_value;

            break;
        }
        }

        cpu->mulfu2 = cpu->mulfu1;
        cpu->mulfu1.has_insn = FALSE;
        if (ENABLE_DEBUG_MESSAGES && cpu->mulfu1.opcode != OPCODE_NULL)
        {
            print_stage_content("mulfu1", &cpu->mulfu1);
        }
    }
}

static void
APEX_mulfu2(APEX_CPU *cpu)
{
    if (cpu->mulfu2.has_insn)
    {

        cpu->mulfu3 = cpu->mulfu2;
        cpu->mulfu2.has_insn = FALSE;
        if (ENABLE_DEBUG_MESSAGES && cpu->mulfu2.opcode != OPCODE_NULL)
        {
            print_stage_content("mulfu2", &cpu->mulfu2);
        }
    }
}

static void
APEX_mulfu3(APEX_CPU *cpu)
{
    if (cpu->mulfu3.has_insn)
    {

        cpu->mulfu4 = cpu->mulfu3;
        cpu->mulfu3.has_insn = FALSE;
        if (ENABLE_DEBUG_MESSAGES && cpu->mulfu3.opcode != OPCODE_NULL)
        {
            print_stage_content("mulfu3", &cpu->mulfu3);
        }
    }
}

static void
APEX_mulfu4(APEX_CPU *cpu)
{
    if (cpu->mulfu4.has_insn)
    {

        switch (cpu->mulfu4.opcode)
        {
        case OPCODE_MUL:
        {

            cpu->renameTableValues[cpu->mulfu4.pd] = cpu->mulfu4.result_buffer;
            cpu->pregs_valid[cpu->mulfu4.pd] = 1;
            break;
        }
        }

        cpu->mulfu4.has_insn = FALSE;

        if (ENABLE_DEBUG_MESSAGES && cpu->mulfu4.opcode != OPCODE_NULL)
        {
            print_stage_content("mulfu4", &cpu->mulfu4);
        }
    }
}

void validaterob(node *head, APEX_CPU *cpu)
{
    node *cursor = head;
    while (cursor != NULL)
    {
        switch (cursor->data.opcode)
        {

        case OPCODE_JUMP:
        case OPCODE_BZ:
        case OPCODE_BNZ:
        case OPCODE_HALT:
        case OPCODE_NOP:
        case OPCODE_NULL:
        {
            break;
        }
        case OPCODE_STORE:
        {
            cpu->mem_valid[cursor->data.imm + cpu->renameTableValues[cursor->data.ps2]] = 1;
            cpu->mreadybit[cursor->data.pc] = 1;
            cpu->lsInstComplete[cursor->data.opcode] = 1;
            break;
        }
        case OPCODE_STR:
        {
            cpu->mem_valid[cpu->renameTableValues[cursor->data.ps1] + cpu->renameTableValues[cursor->data.ps2]] = 1;
            cpu->mreadybit[cursor->data.pc] = 1;
            cpu->lsInstComplete[cursor->data.pc] = 1;
            break;
        }

        default:
        {
            hasher *temp = search_physical_register2(rfprf, cursor->data.rd);
            if (temp != NULL)
            {
                int data = temp->data.prf_code;
                rfprf = remove_any_hasher(rfprf, temp);
                phead = enqueueReg(phead, data);
                cpu->pregs_valid[cursor->data.pd] = 1;
            }
        }
        }

        cursor = cursor->next;
    }
}

static int
APEX_rob(APEX_CPU *cpu)
{
    if (cpu->rob.flush == 1)
    {
        strcpy(cpu->rob.opcode_str, " ");
        cpu->rob.opcode = 0x0;
        cpu->rob.pc = 0000;
        cpu->rob.flush = 0;
    }

    if (count(robhead) != 0)
    {
        if (robhead->data.opcode == OPCODE_HALT)
        {
            validaterob(robhead, cpu);
            cpu->pregs_valid[cpu->issueq.pd] = 1;
            if (ENABLE_DEBUG_MESSAGES && robhead->data.opcode != OPCODE_NULL)
            {
                print_stage_content("ROB ", &robhead->data);
            }
            return TRUE;
        }
    }
    if (cpu->rob.opcode != 0x0 && cpu->rob.has_insn == TRUE)
    {
        robhead = enqueue(robhead, cpu->rob);
    }
    node *cursor = robhead;
    while (cursor != NULL)
    {
        if (ENABLE_DEBUG_MESSAGES && cursor->data.opcode != OPCODE_NULL)
        {
            print_stage_content("ROB ", &cursor->data);
        }
        cursor = cursor->next;
    }

    if (count(robhead) != 0)
    {

        switch (robhead->data.opcode)
        {
        case OPCODE_ADD:
        case OPCODE_MUL:
        case OPCODE_DIV:
        case OPCODE_AND:
        case OPCODE_OR:
        case OPCODE_XOR:
        case OPCODE_ADDL:
        case OPCODE_LDR:
        case OPCODE_LOAD:
        {
            if (cpu->pregs_valid[robhead->data.pd])
            {

                cpu->regs[robhead->data.rd] = cpu->renameTableValues[robhead->data.pd];
                cpu->regs_valid[robhead->data.rd] = 1;
                phead = enqueueReg(phead, robhead->data.pd);
                robhead = dequeue(robhead);
            }

            break;
        }

        case OPCODE_SUB:
        case OPCODE_SUBL:
        {
            if (cpu->pregs_valid[robhead->data.pd])
            {
                cpu->zero_flag = cpu->renameTableValues[robhead->data.pd];

                cpu->regs[robhead->data.rd] = cpu->renameTableValues[robhead->data.pd];
                cpu->regs_valid[robhead->data.rd] = 1;
                phead = enqueueReg(phead, robhead->data.pd);
                robhead = dequeue(robhead);
            }

            break;
        }
        case OPCODE_BZ:
        case OPCODE_BNZ:
        {
            if (cpu->branchreadybit[robhead->data.pc] == 1)
            {
                robhead = dequeue(robhead);
            }

            break;
        }
        case OPCODE_JUMP:
        {
            if (cpu->branch_taken == 1)
            {
                cpu->pc = cpu->jumpval[robhead->data.pc];
                cpu->jump_inst = 0;
                cpu->intfu.flush = 1;
                cpu->decode.flush = 1;
                cpu->branch_taken = 0;
                validaterob(robhead, cpu);
                dispose(robhead);
                robhead = dequeue(robhead);
                cpu->rob.flush = 1;
                dispose(iqhead);
                iqhead = dequeue(iqhead);
                cpu->issueq.flush = 1;
                dispose(lsqhead);
                lsqhead = dequeue(lsqhead);
                cpu->lsq.flush = 1;
                cpu->intfu.flush = 1;
                cpu->mulfu1.flush = 1;
                cpu->mulfu2.flush = 1;
                cpu->mulfu3.flush = 1;
                cpu->mulfu4.flush = 1;
                cpu->dcache.flush = 1;
                cpu->dispatch.flush = 1;
                cpu->branchcomplete = 0;
            }
            break;
        }

        case OPCODE_STR:
        {

            if (cpu->lsInstComplete[robhead->data.pc])
            {
                robhead = dequeue(robhead);
            }
            break;
        }

        case OPCODE_STORE:
        {

            if (cpu->mreadybit[robhead->data.pc])
            {
                robhead = dequeue(robhead);
            }
            break;
        }

        case OPCODE_CMP:
        {
            if (cpu->cmp_completed == 1)
            {
                cpu->zero_flag = cpu->cmpvalue[robhead->data.pc];
                cpu->regs[robhead->data.rd] = cpu->renameTableValues[robhead->data.pd];
                cpu->regs_valid[robhead->data.rd] = 1;
                phead = enqueueReg(phead, robhead->data.pd);
                robhead = dequeue(robhead);
                cpu->cmp_completed = 0;
            }
            break;
        }

        case OPCODE_MOVC:
        {
            if (cpu->pregs_valid[robhead->data.pd])
            {

                cpu->regs[robhead->data.rd] = cpu->renameTableValues[robhead->data.pd];
                cpu->regs_valid[robhead->data.rd] = 1;
                phead = enqueueReg(phead, robhead->data.pd);
                robhead = dequeue(robhead);
            }
            break;
        }
        case OPCODE_HALT:
        {

            break;
        }
        default:
        {
            robhead = dequeue(robhead);
        }
        }
        cpu->decode.stalled = 0;
        cpu->insn_completed++;
        cpu->rob.has_insn = FALSE;
    }

    /* Default */
    return 0;
}

/*
 * This function creates and initializes APEX cpu.
 *
 * Note: You are free to edit this function according to your implementation
 */
APEX_CPU *
APEX_cpu_init(const char *filename)
{

    int i;
    APEX_CPU *cpu;

    if (!filename)
    {
        return NULL;
    }

    cpu = calloc(1, sizeof(APEX_CPU));

    if (!cpu)
    {
        return NULL;
    }

    /* Initialize PC, Registers and all pipeline stages */
    cpu->pc = 4000;
    memset(cpu->regs, 0, sizeof(int) * REG_FILE_SIZE);
    memset(cpu->renameTableValues, 0, sizeof(int) * (PREGS_FILE_SIZE + 1));

    iqhead = NULL;
    lsqhead = NULL;
    robhead = NULL;
    phead = NULL;
    rfprf = NULL;
    btb_head = NULL;

    memset(cpu->data_memory, 0, sizeof(int) * DATA_MEMORY_SIZE);
    memset(cpu->mreadybit, 0, sizeof(int) * 60000);
    memset(cpu->cmpvalue, 0, sizeof(int) * 60000);
    memset(cpu->branchreadybit, 0, sizeof(int) * 60000);
    memset(cpu->jumpval, 0, sizeof(int) * 60000);
    for (i = 0; i < 4096; i++)
    {
        cpu->mem_valid[i] = 1;
    }
    cpu->zero_flag = -9999;
    cpu->branch_taken = 0;
    cpu->cmp_completed = 0;
    cpu->mulstage = 0;
    cpu->branchcomplete = 0;
    cpu->branch_taken_stage = 0;
    for (i = 0; i < 16; i++)
    {
        cpu->regs_valid[i] = 1;
    }
    for (i = 0; i < 15; i++)
    {
        cpu->pregs_valid[i] = 1;
        phead = enqueueReg(phead, i);
    }

    cpu->single_step = ENABLE_SINGLE_STEP;

    /* Parse input file and create code memory */
    cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);
    if (!cpu->code_memory)
    {
        free(cpu);
        return NULL;
    }

    if (ENABLE_DEBUG_MESSAGES)
    {
        fprintf(stderr,
                "APEX_CPU: Initialized APEX CPU, loaded %d instructions\n",
                cpu->code_memory_size);
        fprintf(stderr, "APEX_CPU: PC initialized to %d\n", cpu->pc);
        fprintf(stderr, "APEX_CPU: Printing Code Memory\n");
        printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode_str", "rd", "rs1", "rs2",
               "imm");

        for (i = 0; i < cpu->code_memory_size; ++i)
        {
            printf("%-9s %-9d %-9d %-9d %-9d\n", cpu->code_memory[i].opcode_str,
                   cpu->code_memory[i].rd, cpu->code_memory[i].rs1,
                   cpu->code_memory[i].rs2, cpu->code_memory[i].imm);
        }
    }

    /* To start fetch stage */
    cpu->fetch.has_insn = TRUE;

    return cpu;
}

void printFile(APEX_CPU *cpu)
{

    printf("\n-----------------REGISTER FILE------------------------------------------------------- \n");
    printf("|Ar Register|Phy. Register| Value | VALID bit\n");
    for (int i = 0; i < 16; i++)
    {
        printf("|R[%d]\t|\tP[%d]\t|\t=%d\t|\t%d\n", i, search_physical_register1(rfprf, i), cpu->renameTableValues[search_physical_register(rfprf, i)], cpu->pregs_valid[search_physical_register1(rfprf, i)]);
    }
    printf("\n-----------------REGISTER FILE------------------------------------------------------- \n");

    printf("\n-----------------DATA MEMORY-------------- \n");
    for (int i = 0; i < 4096; i++)
    {
        if (cpu->data_memory[i] != 0)
        {

            printf("| MEM[%d] | Value=%d | \n", i, cpu->data_memory[i]);
        }
    }
    printf("-----------------DATA MEMORY-------------- \n");
}

/*
 * APEX CPU simulation loop
 *
 * Note: You are free to edit this function according to your implementation
 */
void APEX_cpu_run(APEX_CPU *cpu)
{
    char user_prompt_val;

    while (TRUE)
    {
        if (ENABLE_DEBUG_MESSAGES)
        {
            printf("--------------------------------------------\n");
            printf("Clock Cycle #: %d\n", cpu->clock);
            printf("--------------------------------------------\n");
        }

        APEX_intfu(cpu);
        APEX_logicalfu(cpu);
        APEX_mulfu4(cpu);
        APEX_mulfu3(cpu);
        APEX_mulfu2(cpu);
        APEX_mulfu1(cpu);
        APEX_dcache(cpu);
        if (APEX_rob(cpu))
        {
            printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", cpu->clock + 1, cpu->insn_completed);
            break;
        }
        APEX_lsq(cpu);
        APEX_issueq(cpu);
        APEX_dispatch(cpu);
        APEX_decode(cpu);
        APEX_fetch(cpu);

        print_reg_file(cpu);

        if (cpu->single_step)
        {
            printf("Press any key to advance CPU Clock or <q> to quit:\n");
            scanf("%c", &user_prompt_val);

            if ((user_prompt_val == 'Q') || (user_prompt_val == 'q'))
            {
                printf("APEX_CPU: Simulation Stopped, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
                break;
            }
        }

        cpu->clock++;
    }
}

void APEX_cpu_stop(APEX_CPU *cpu)
{
    free(cpu->code_memory);
    free(cpu);
}