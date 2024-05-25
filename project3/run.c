/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   CS311 KAIST                                               */
/*   run.c                                                     */
/*                                                             */
/***************************************************************/

#include <stdio.h>

#include "util.h"
#include "run.h"

/***************************************************************/
/*                                                             */
/* Procedure: get_inst_info                                    */
/*                                                             */
/* Purpose: Read insturction information                       */
/*                                                             */
/***************************************************************/
instruction* get_inst_info(uint32_t pc) 
{ 
    return &INST_INFO[(pc - MEM_TEXT_START) >> 2];
}

void flush_IF_ID_regs(CPU_State* state) {
	state->PIPE[IF_STAGE] = 0;
	state->IF_ID_INST = 0;
	state->IF_ID_NPC = 0;
}

void stay_IF(CPU_State* state) {
	state->PIPE[IF_STAGE] = CURRENT_STATE.PIPE[IF_STAGE];
	state->IF_ID_INST = CURRENT_STATE.IF_ID_INST;
	state->IF_ID_NPC = CURRENT_STATE.IF_ID_NPC;
}

void IF_stage(CPU_State* next_state) {
	next_state->IF_PC = CURRENT_STATE.PC;
	if(CURRENT_STATE.PIPE_STALL[IF_STAGE] == TRUE) {
		next_state->PIPE_STALL[IF_STAGE] = FALSE;
		stay_IF(next_state);
		return;
	}
	flush_IF_ID_regs(next_state);
	if((int32_t)CURRENT_STATE.PC < (int32_t)MEM_REGIONS[0].start ||
		(int32_t)CURRENT_STATE.PC >= (int32_t)(MEM_REGIONS[0].start + (NUM_INST * 4))) {
			return;
		}
	
	next_state->PIPE[IF_STAGE] = CURRENT_STATE.PC;
	instruction* inst = get_inst_info(CURRENT_STATE.PC);
	next_state->IF_ID_NPC = CURRENT_STATE.PC + BYTES_PER_WORD;
	next_state->IF_ID_INST = inst;
	next_state->IF_PC = CURRENT_STATE.PC + BYTES_PER_WORD;
}

void flush_ID_EX_regs(CPU_State* state) {
	state->PIPE[ID_STAGE] = 0;
	state->ID_EX_NPC = 0;
	state->ID_EX_REG1 = 0;
	state->ID_EX_REG1_VAL = 0;
	state->ID_EX_REG2 = 0;
	state->ID_EX_REG2_VAL = 0;
	state->ID_EX_IMM = 0;
	state->ID_EX_DEST = 0;
    state->ID_EX_JVAL = 0; 
	state->ID_EX_MEM_READ = FALSE;
	state->ID_EX_MEM_WRITE = FALSE;
	state->ID_EX_ALU_CTRL = NOOP;
	state->ID_EX_SHAMT = 0;
}

void stay_ID(CPU_State* state) {
	state->PIPE[ID_STAGE] = CURRENT_STATE.PIPE[ID_STAGE];
	state->ID_EX_NPC = CURRENT_STATE.ID_EX_NPC;

	// update register values when stalling
	state->ID_EX_REG1 = CURRENT_STATE.ID_EX_REG1;
	if(state->ID_EX_REG1) 
		state->ID_EX_REG1_VAL = state->REGS[state->ID_EX_REG1];
	else 
		state->ID_EX_REG1_VAL = CURRENT_STATE.ID_EX_REG1_VAL;
	
	state->ID_EX_REG2 = CURRENT_STATE.ID_EX_REG2;
	if(state->ID_EX_REG2) 
		state->ID_EX_REG2_VAL = state->REGS[state->ID_EX_REG2];
	else
		state->ID_EX_REG2_VAL = CURRENT_STATE.ID_EX_REG2_VAL;

	state->ID_EX_SHAMT = CURRENT_STATE.ID_EX_SHAMT;
	state->ID_EX_IMM = CURRENT_STATE.ID_EX_IMM;
	state->ID_EX_DEST = CURRENT_STATE.ID_EX_DEST;
    state->ID_EX_JVAL = CURRENT_STATE.ID_EX_JVAL; 
	state->ID_EX_MEM_READ = CURRENT_STATE.ID_EX_MEM_READ;
	state->ID_EX_MEM_WRITE = CURRENT_STATE.ID_EX_MEM_WRITE;
	state->ID_EX_ALU_CTRL = CURRENT_STATE.ID_EX_ALU_CTRL;
}

void ID_stage(CPU_State* next_state) {
	if(CURRENT_STATE.PIPE_STALL[ID_STAGE] == TRUE) {
		next_state->PIPE_STALL[ID_STAGE] = FALSE;
		stay_ID(next_state);
		return;
	}
	flush_ID_EX_regs(next_state);
	if(CURRENT_STATE.JUMP_TAKEN || 
		(int32_t)CURRENT_STATE.PIPE[IF_STAGE] < (int32_t)MEM_REGIONS[0].start ||
		(int32_t)CURRENT_STATE.PIPE[IF_STAGE] >= (int32_t)(MEM_REGIONS[0].start + (NUM_INST * 4))) 
			return;
	
	next_state->PIPE[ID_STAGE] = CURRENT_STATE.PIPE[IF_STAGE];
	next_state->ID_EX_NPC = CURRENT_STATE.IF_ID_NPC;
	instruction* inst = CURRENT_STATE.IF_ID_INST;

	switch(OPCODE(inst)) {
		//Type I
		case 0x8:		//(0x001000)ADDI
			next_state->ID_EX_REG1_VAL = next_state->REGS[RS(inst)];
			next_state->ID_EX_REG1 = RS(inst);
			next_state->ID_EX_REG2_VAL = SIGN_EX(IMM(inst));
			next_state->ID_EX_DEST = RT(inst);
			next_state->ID_EX_ALU_CTRL = ADDOP;
			break;
		case 0xc:		//(0x001100)ANDI
			next_state->ID_EX_REG1_VAL = next_state->REGS[RS(inst)];
			next_state->ID_EX_REG1 = RS(inst);
			next_state->ID_EX_REG2_VAL = IMM(inst);
			next_state->ID_EX_DEST = RT(inst);
			next_state->ID_EX_ALU_CTRL = ANDOP;
			break;
		case 0xf:		//(0x001111)LUI	
			next_state->ID_EX_REG2_VAL = IMM(inst);
			next_state->ID_EX_DEST = RT(inst);
			next_state->ID_EX_ALU_CTRL = SLLOP;
			next_state->ID_EX_SHAMT = LUI_SHAMT;
			break;
		case 0xd:		//(0x001101)ORI
			next_state->ID_EX_REG1_VAL = next_state->REGS[RS(inst)];
			next_state->ID_EX_REG1 = RS(inst);
			next_state->ID_EX_REG2_VAL = IMM(inst);
			next_state->ID_EX_DEST = RT(inst);
			next_state->ID_EX_ALU_CTRL = OROP;
			break;
		case 0xa:		//(0x001010)SLTI
			next_state->ID_EX_REG1_VAL = next_state->REGS[RS(inst)];
			next_state->ID_EX_REG1 = RS(inst);
			next_state->ID_EX_REG2_VAL = SIGN_EX(IMM(inst));
			next_state->ID_EX_DEST = RT(inst);
			next_state->ID_EX_ALU_CTRL = SLTOP;
			break;
		case 0x23:		//(0x100011)LW	
			next_state->ID_EX_REG1_VAL = next_state->REGS[RS(inst)];
			next_state->ID_EX_REG1 = RS(inst);
			next_state->ID_EX_REG2_VAL = SIGN_EX(IMM(inst));
			next_state->ID_EX_DEST = RT(inst);
			next_state->ID_EX_ALU_CTRL = ADDOP;
			next_state->ID_EX_MEM_READ = TRUE;
			break;
		case 0x2b:		//(0x101011)SW
			next_state->ID_EX_REG1_VAL = next_state->REGS[RS(inst)];
			next_state->ID_EX_REG1 = RS(inst);
			next_state->ID_EX_REG2_VAL = SIGN_EX(IMM(inst));
			next_state->ID_EX_DEST = RT(inst);
			next_state->ID_EX_ALU_CTRL = ADDOP;
			next_state->ID_EX_MEM_WRITE = TRUE;
			break;
		case 0x4:		//(0x000100)BEQ
			next_state->ID_EX_REG1_VAL = next_state->REGS[RS(inst)];
			next_state->ID_EX_REG2_VAL = next_state->REGS[RT(inst)];
			next_state->ID_EX_REG1 = RS(inst);
			next_state->ID_EX_REG2 = RT(inst);
			next_state->ID_EX_IMM = SIGN_EX(IMM(inst));
			next_state->ID_EX_ALU_CTRL = BEQOP;
			if(BR_BIT == TRUE) { // take the branch
				next_state->JUMP_PC = (int32_t)CURRENT_STATE.IF_ID_NPC + 
					((int32_t)next_state->ID_EX_IMM << 2);
			}
			break;
		case 0x5:		//(0x000101)BNE
			next_state->ID_EX_REG1_VAL = next_state->REGS[RS(inst)];
			next_state->ID_EX_REG2_VAL = next_state->REGS[RT(inst)];
			next_state->ID_EX_REG1 = RS(inst);
			next_state->ID_EX_REG2 = RT(inst);
			next_state->ID_EX_IMM = SIGN_EX(IMM(inst));
			next_state->ID_EX_ALU_CTRL = BNEOP;
			if(BR_BIT == TRUE) { // take the branch
				next_state->JUMP_PC = (int32_t)CURRENT_STATE.IF_ID_NPC + 
					((int32_t)next_state->ID_EX_IMM << 2);
			}
			break;

		//TYPE R
		case 0x0:		//(0x000000)ADD, AND, NOR, OR, SLT, SLL, SRL, SUB  if JR
			switch(FUNC (inst)) {
				case 0x20:	//ADD
					next_state->ID_EX_REG1_VAL = next_state->REGS[RS(inst)];
					next_state->ID_EX_REG2_VAL = next_state->REGS[RT(inst)];
					next_state->ID_EX_REG1 = RS(inst);
					next_state->ID_EX_REG2 = RT(inst);
					next_state->ID_EX_DEST = RD(inst);
					next_state->ID_EX_ALU_CTRL = ADDOP;
					break;
				case 0x24:	//AND
					next_state->ID_EX_REG1_VAL = next_state->REGS[RS(inst)];
					next_state->ID_EX_REG2_VAL = next_state->REGS[RT(inst)];
					next_state->ID_EX_REG1 = RS(inst);
					next_state->ID_EX_REG2 = RT(inst);
					next_state->ID_EX_DEST = RD(inst);
					next_state->ID_EX_ALU_CTRL = ANDOP;
					break;
				case 0x27:	//NOR
					next_state->ID_EX_REG1_VAL = next_state->REGS[RS(inst)];
					next_state->ID_EX_REG2_VAL = next_state->REGS[RT(inst)];
					next_state->ID_EX_REG1 = RS(inst);
					next_state->ID_EX_REG2 = RT(inst);
					next_state->ID_EX_DEST = RD(inst);
					next_state->ID_EX_ALU_CTRL = NOROP;
					break;
				case 0x25:	//OR
					next_state->ID_EX_REG1_VAL = next_state->REGS[RS(inst)];
					next_state->ID_EX_REG2_VAL = next_state->REGS[RT(inst)];
					next_state->ID_EX_REG1 = RS(inst);
					next_state->ID_EX_REG2 = RT(inst);
					next_state->ID_EX_DEST = RD(inst);
					next_state->ID_EX_ALU_CTRL = OROP;
					break;
				case 0x2A:	//SLT
					next_state->ID_EX_REG1_VAL = next_state->REGS[RS(inst)];
					next_state->ID_EX_REG2_VAL = next_state->REGS[RT(inst)];
					next_state->ID_EX_REG1 = RS(inst);
					next_state->ID_EX_REG2 = RT(inst);
					next_state->ID_EX_DEST = RD(inst);
					next_state->ID_EX_ALU_CTRL = SLTOP;
					break;
				case 0x0:	//SLL
					next_state->ID_EX_REG2_VAL = next_state->REGS[RT(inst)];
					next_state->ID_EX_REG2 = RT(inst);
					next_state->ID_EX_DEST = RD(inst);
					next_state->ID_EX_ALU_CTRL = SLLOP;
					next_state->ID_EX_SHAMT = SHAMT(inst);
					break;
				case 0x2:	//SRL
					next_state->ID_EX_REG2_VAL = next_state->REGS[RT(inst)];
					next_state->ID_EX_REG2 = RT(inst);
					next_state->ID_EX_DEST = RD(inst);
					next_state->ID_EX_ALU_CTRL = SRLOP;
					next_state->ID_EX_SHAMT = SHAMT(inst);
					break;
				case 0x22:	//SUB
					next_state->ID_EX_REG1_VAL = next_state->REGS[RS(inst)];
					next_state->ID_EX_REG2_VAL = next_state->REGS[RT(inst)];
					next_state->ID_EX_REG1 = RS(inst);
					next_state->ID_EX_REG2 = RT(inst);
					next_state->ID_EX_DEST = RD(inst);
					next_state->ID_EX_ALU_CTRL = SUBOP;
					break;
				case 0x8:	//JR
					next_state->JUMP_PC = next_state->REGS[RS(inst)];
					break;

				default:
					printf("Unknown function code type: %d\n", FUNC(inst));
					break;
			}
			break;

		//TYPE J
		case 0x2:		//(0x000010)J
			next_state->JUMP_PC = (CURRENT_STATE.IF_ID_NPC & 0xf0000000) | (TARGET (inst) << 2);
			break;
		case 0x3:		//(0x000011)JAL
			next_state->JUMP_PC = (CURRENT_STATE.IF_ID_NPC & 0xf0000000) | (TARGET (inst) << 2);
			next_state->ID_EX_DEST = JAL_DEST_REG;
			next_state->ID_EX_JVAL = CURRENT_STATE.IF_ID_NPC;
			break;

		default:
			printf("Not available instruction\n");
			assert(0);
    }
	// hazard detect
	if(CURRENT_STATE.ID_EX_MEM_READ == TRUE && CURRENT_STATE.ID_EX_DEST && 
		((CURRENT_STATE.ID_EX_DEST == next_state->ID_EX_REG1) || 
		(CURRENT_STATE.ID_EX_DEST == next_state->ID_EX_REG2))) {
			// stall the pipeline
			next_state->PIPE_STALL[IF_STAGE] = TRUE;
			next_state->PIPE_STALL[ID_STAGE] = TRUE;
			next_state->PIPE_STALL[EX_STAGE] = TRUE;
	}
}

void flush_EX_MEM_regs(CPU_State* state) {
	state->PIPE[EX_STAGE] = 0;
	state->EX_MEM_NPC = 0;
	state->EX_MEM_ALU_OUT = 0;
	state->EX_MEM_W_VALUE = 0;
	state->EX_MEM_BR_TARGET = 0;
	state->EX_MEM_DEST = 0;
	state->EX_MEM_FORWARD_REG = 0;
	state->EX_MEM_FORWARD_VALUE = 0;
	state->EX_MEM_BR_TAKE = 0;
	state->EX_MEM_FORWARD_REG = 0;
	state->EX_MEM_FORWARD_VALUE = 0;
}

void EX_stage(CPU_State* next_state) {
	flush_EX_MEM_regs(next_state);
	if(CURRENT_STATE.PIPE_STALL[EX_STAGE] == TRUE) {
		next_state->PIPE_STALL[EX_STAGE] = FALSE;
		return;
	}
	if((int32_t)CURRENT_STATE.PIPE[ID_STAGE] < (int32_t)MEM_REGIONS[0].start ||
		(int32_t)CURRENT_STATE.PIPE[ID_STAGE] >= (int32_t)(MEM_REGIONS[0].start + (NUM_INST * 4)))
		return;

	next_state->PIPE[EX_STAGE] = CURRENT_STATE.PIPE[ID_STAGE];
	next_state->EX_MEM_NPC = CURRENT_STATE.ID_EX_NPC;
	next_state->EX_MEM_MEM_WRITE = CURRENT_STATE.ID_EX_MEM_WRITE;
	next_state->EX_MEM_MEM_READ = CURRENT_STATE.ID_EX_MEM_READ;
	next_state->EX_MEM_DEST = CURRENT_STATE.ID_EX_DEST;

	if(CURRENT_STATE.EX_MEM_MEM_WRITE == TRUE) {
		next_state->EX_MEM_W_VALUE = CURRENT_STATE.REGS[CURRENT_STATE.ID_EX_DEST];
	}

	if (CURRENT_STATE.ID_EX_ALU_CTRL != NOOP) {
		uint32_t reg1 = CURRENT_STATE.ID_EX_REG1_VAL;
		uint32_t reg2 = CURRENT_STATE.ID_EX_REG2_VAL;

		// EX forward
		if(CURRENT_STATE.EX_MEM_FORWARD_REG) { 
			if(CURRENT_STATE.EX_MEM_FORWARD_REG == CURRENT_STATE.ID_EX_REG1)
				reg1 = CURRENT_STATE.EX_MEM_FORWARD_VALUE;
			if(CURRENT_STATE.EX_MEM_FORWARD_REG == CURRENT_STATE.ID_EX_REG2) 
				reg2 = CURRENT_STATE.EX_MEM_FORWARD_VALUE;
			if(CURRENT_STATE.EX_MEM_MEM_WRITE == TRUE && 
				CURRENT_STATE.EX_MEM_FORWARD_REG == CURRENT_STATE.ID_EX_DEST) {
					next_state->EX_MEM_W_VALUE = CURRENT_STATE.EX_MEM_FORWARD_VALUE;
				}
		}
		// MEM forward
		if(CURRENT_STATE.MEM_WB_FORWARD_REG != CURRENT_STATE.EX_MEM_FORWARD_REG
			&& CURRENT_STATE.MEM_WB_FORWARD_REG) {
			if(CURRENT_STATE.MEM_WB_FORWARD_REG == CURRENT_STATE.ID_EX_REG1)
				reg1 = CURRENT_STATE.MEM_WB_FORWARD_VALUE;
			if(CURRENT_STATE.MEM_WB_FORWARD_REG == CURRENT_STATE.ID_EX_REG2) 
				reg2 = CURRENT_STATE.MEM_WB_FORWARD_VALUE;
			if(CURRENT_STATE.EX_MEM_MEM_WRITE == TRUE &&
				CURRENT_STATE.MEM_WB_FORWARD_REG == CURRENT_STATE.ID_EX_DEST) {
					next_state->EX_MEM_W_VALUE = CURRENT_STATE.MEM_WB_FORWARD_VALUE;
				}
		}

		switch(CURRENT_STATE.ID_EX_ALU_CTRL) {
			case ADDOP:
				next_state->EX_MEM_ALU_OUT = (int32_t)reg1 + (int32_t)reg2;
				break;
			case SUBOP:
				next_state->EX_MEM_ALU_OUT = (int32_t)reg1 - (int32_t)reg2;
				break;
			case ANDOP:
				next_state->EX_MEM_ALU_OUT = reg1 & reg2;
				break;
			case OROP:
				next_state->EX_MEM_ALU_OUT = reg1 | reg2;
				break;
			case SLLOP:
				next_state->EX_MEM_ALU_OUT = reg2 << CURRENT_STATE.ID_EX_SHAMT;
				break;
			case SRLOP:
				next_state->EX_MEM_ALU_OUT = reg2 >> CURRENT_STATE.ID_EX_SHAMT;
				break;
			case BEQOP:
				next_state->EX_MEM_ALU_OUT = (reg1 == reg2? TRUE: FALSE);
				// assumption: target address > 0
				next_state->EX_MEM_BR_TARGET = (int32_t)CURRENT_STATE.ID_EX_NPC + 
					((int32_t)CURRENT_STATE.ID_EX_IMM << 2);
				next_state->EX_MEM_BR_TAKE = TRUE;
				break;
			case BNEOP:
				next_state->EX_MEM_ALU_OUT = (reg1 != reg2? TRUE: FALSE);
				/// assumption: target address > 0
				next_state->EX_MEM_BR_TARGET = (int32_t)CURRENT_STATE.ID_EX_NPC 
					+ ((int32_t)CURRENT_STATE.ID_EX_IMM << 2);
				next_state->EX_MEM_BR_TAKE = TRUE;
				break;
			case SLTOP:
				next_state->EX_MEM_ALU_OUT = ((int32_t)reg1 < (int32_t)reg2);
				break;
			case NOROP:
				next_state->EX_MEM_ALU_OUT = ~(reg1 | reg2);
				break;
			default:
				break;	
		}
		next_state->EX_MEM_FORWARD_REG = CURRENT_STATE.ID_EX_DEST;
		next_state->EX_MEM_FORWARD_VALUE = next_state->EX_MEM_ALU_OUT;
	} else if (CURRENT_STATE.ID_EX_JVAL) {
		next_state->EX_MEM_ALU_OUT = CURRENT_STATE.ID_EX_JVAL;
		next_state->EX_MEM_FORWARD_REG = CURRENT_STATE.ID_EX_DEST;
		next_state->EX_MEM_FORWARD_VALUE = next_state->EX_MEM_ALU_OUT;
	}
}

void flush_MEM_WB_regs(CPU_State* state) {
	state->PIPE[MEM_STAGE] = 0;
	state->MEM_WB_ALU_OUT = 0;
	state->MEM_WB_DEST = 0;
	state->MEM_WB_MEM_READ = FALSE;
	state->MEM_WB_MEM_WRITE = FALSE;
	state->MEM_WB_FORWARD_REG = 0;
	state->MEM_WB_FORWARD_VALUE = 0;
}

void MEM_stage(CPU_State* next_state) {
	flush_MEM_WB_regs(next_state);
	if((int32_t)CURRENT_STATE.PIPE[EX_STAGE] < (int32_t)MEM_REGIONS[0].start ||
		(int32_t)CURRENT_STATE.PIPE[EX_STAGE] >= (int32_t)(MEM_REGIONS[0].start + (NUM_INST * 4)))
		return;
	next_state->PIPE[MEM_STAGE] = CURRENT_STATE.PIPE[EX_STAGE];
	next_state->MEM_WB_ALU_OUT = CURRENT_STATE.EX_MEM_ALU_OUT;
	next_state->MEM_WB_DEST = CURRENT_STATE.EX_MEM_DEST;
	next_state->MEM_WB_FORWARD_REG = CURRENT_STATE.EX_MEM_FORWARD_REG;
	next_state->MEM_WB_FORWARD_VALUE = CURRENT_STATE.EX_MEM_FORWARD_VALUE;

	if(CURRENT_STATE.EX_MEM_MEM_READ == TRUE) {
		next_state->MEM_WB_ALU_OUT = mem_read_32(CURRENT_STATE.EX_MEM_ALU_OUT);
		// MEM-EX forwarding
		next_state->MEM_WB_FORWARD_REG = CURRENT_STATE.EX_MEM_DEST;
		next_state->MEM_WB_FORWARD_VALUE = next_state->MEM_WB_ALU_OUT;

		next_state->MEM_WB_MEM_READ = TRUE;
	}else if(CURRENT_STATE.EX_MEM_MEM_WRITE == TRUE) {
		uint32_t wval = CURRENT_STATE.EX_MEM_W_VALUE;
		// MEM-MEM Forwarding
		if(CURRENT_STATE.MEM_WB_DEST && CURRENT_STATE.MEM_WB_MEM_READ == TRUE &&
			CURRENT_STATE.EX_MEM_DEST == CURRENT_STATE.MEM_WB_DEST) {
			wval = CURRENT_STATE.MEM_WB_ALU_OUT;
		}
		mem_write_32(CURRENT_STATE.EX_MEM_ALU_OUT, wval);
		next_state->MEM_WB_MEM_WRITE = TRUE;
	}
	// taking branch
	if(CURRENT_STATE.EX_MEM_BR_TAKE == TRUE) {
		if(CURRENT_STATE.EX_MEM_ALU_OUT != BR_BIT) {
			BR_BIT = (int)CURRENT_STATE.EX_MEM_ALU_OUT;
			if(CURRENT_STATE.EX_MEM_ALU_OUT == FALSE) {
				next_state->BRANCH_PC = CURRENT_STATE.EX_MEM_NPC;
			} else {
				next_state->BRANCH_PC = CURRENT_STATE.EX_MEM_BR_TARGET;
			}
		}
	}
}

void WB_stage(CPU_State* next_state) {
	next_state->PIPE[WB_STAGE] = 0;
	if((int32_t)CURRENT_STATE.PIPE[MEM_STAGE] < (int32_t)MEM_REGIONS[0].start ||
		(int32_t)CURRENT_STATE.PIPE[MEM_STAGE] >= (int32_t)(MEM_REGIONS[0].start + (NUM_INST * 4)))
		return;
	next_state->PIPE[WB_STAGE] = CURRENT_STATE.PIPE[MEM_STAGE]; 
	if(CURRENT_STATE.MEM_WB_DEST && CURRENT_STATE.MEM_WB_MEM_WRITE == FALSE) {
		next_state->REGS[CURRENT_STATE.MEM_WB_DEST] = CURRENT_STATE.MEM_WB_ALU_OUT;
	}
	++INSTRUCTION_COUNT;
}

void deep_copy(CPU_State* state) {
	for(int i = 0; i < MIPS_REGS; ++i) {
		state->REGS[i] = CURRENT_STATE.REGS[i];
	}
	for(int i = 0; i < PIPE_STAGE; ++i) {
		state->PIPE_STALL[i] = CURRENT_STATE.PIPE_STALL[i];
	}
}

void process_instruction() {
	CPU_State* next_state = (CPU_State*)malloc(sizeof(CPU_State));
	deep_copy(next_state);
	WB_stage(next_state);
	MEM_stage(next_state);
	EX_stage(next_state);
	ID_stage(next_state);
	IF_stage(next_state);

	// choose PC
	next_state->JUMP_TAKEN = FALSE;
	if(next_state->BRANCH_PC) { // known at MEM stage
		flush_IF_ID_regs(next_state);
		flush_ID_EX_regs(next_state);
		flush_EX_MEM_regs(next_state);
		next_state->PC = next_state->BRANCH_PC;
	} else if(next_state->JUMP_PC) { // known at ID stage
		uint32_t pc = next_state->PIPE[IF_STAGE];
		flush_IF_ID_regs(next_state);
		next_state->PIPE[IF_STAGE] = pc;
		next_state->PC = next_state->JUMP_PC;
		next_state->JUMP_TAKEN = TRUE;
	} else {
		next_state->PC = next_state->IF_PC;
	}
	next_state->BRANCH_PC = next_state->JUMP_PC = next_state->IF_PC = 0;
	CURRENT_STATE = *next_state;

	if(RUN_BIT == TRUE) { // check for stop the pipeline
		RUN_BIT = FALSE;
		for(int i = 0; i < WB_STAGE; ++i) {
			if ((int32_t)CURRENT_STATE.PIPE[i] >= (int32_t)MEM_REGIONS[0].start &&
				(int32_t)CURRENT_STATE.PIPE[i] < (int32_t)(MEM_REGIONS[0].start + (NUM_INST * 4))) {
				RUN_BIT = TRUE;
			}
		}
	}
}