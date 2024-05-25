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

/***************************************************************/
/*                                                             */
/* Procedure: process_instruction                              */
/*                                                             */
/* Purpose: Process one instruction                            */
/*                                                             */
/***************************************************************/
void process_instruction(){
	if (((CURRENT_STATE.PC - MEM_TEXT_START) >> 2) >= NUM_INST) {
		RUN_BIT = FALSE;
		return;
	}
	
    instruction* ins = get_inst_info(CURRENT_STATE.PC);
    CURRENT_STATE.PC += BYTES_PER_WORD;

    switch(OPCODE(ins)) {
	    //Type I
	    case 0x8:		// ADDI
			CURRENT_STATE.REGS[RT(ins)] = (int32_t)CURRENT_STATE.REGS[RS(ins)] + (int32_t)SIGN_EX(IMM(ins));
			break;
		
	    case 0xc:		// ANDI
			CURRENT_STATE.REGS[RT(ins)] = CURRENT_STATE.REGS[RS(ins)] & IMM(ins);
			break;

	    case 0xf:		// LUI	
			CURRENT_STATE.REGS[RT(ins)] = IMM(ins) << 16;
			break;
		
	    case 0xd:		// ORI
			CURRENT_STATE.REGS[RT(ins)] = CURRENT_STATE.REGS[RS(ins)] | IMM(ins);
			break;
		
	    case 0xa:		// SLTI
			CURRENT_STATE.REGS[RT(ins)] = (int32_t)CURRENT_STATE.REGS[RS(ins)] < (int32_t)SIGN_EX(IMM(ins));
			break;
		
	    case 0x23:		// LW
			CURRENT_STATE.REGS[RT(ins)] = mem_read_32(CURRENT_STATE.REGS[RS(ins)] + SIGN_EX(IMM(ins)));
			break;
		
	    case 0x2b:		// SW
			mem_write_32(CURRENT_STATE.REGS[RS(ins)] + SIGN_EX(IMM(ins)), CURRENT_STATE.REGS[RT(ins)]);
			break;
		
	    case 0x4:		// BEQ
			if(CURRENT_STATE.REGS[RS(ins)] == CURRENT_STATE.REGS[RT(ins)]) 
				CURRENT_STATE.PC += (SIGN_EX(IMM(ins)) << 2);
			break;
		
	    case 0x5:		// BNE
			if(CURRENT_STATE.REGS[RS(ins)] != CURRENT_STATE.REGS[RT(ins)]) 
				CURRENT_STATE.PC += (SIGN_EX(IMM(ins)) << 2);
			break;

    	//TYPE R
	    case 0x0:
			switch (FUNC(ins)) {
				case 0x20: // ADD
					CURRENT_STATE.REGS[RD(ins)] = (int32_t)CURRENT_STATE.REGS[RS(ins)] + (int32_t)CURRENT_STATE.REGS[RT(ins)];
					break;

				case 0x24: // AND
					CURRENT_STATE.REGS[RD(ins)] = CURRENT_STATE.REGS[RS(ins)] & CURRENT_STATE.REGS[RT(ins)];
					break;

				case 0x8: // JR
					CURRENT_STATE.PC = CURRENT_STATE.REGS[RS(ins)];
					break;
				
				case 0x27: // NOR
					CURRENT_STATE.REGS[RD(ins)] = ~(CURRENT_STATE.REGS[RS(ins)] | CURRENT_STATE.REGS[RT(ins)]);
					break;
				
				case 0x25: // OR
					CURRENT_STATE.REGS[RD(ins)] = CURRENT_STATE.REGS[RS(ins)] | CURRENT_STATE.REGS[RT(ins)];
					break;
				
				case 0x2a: // SLT
					CURRENT_STATE.REGS[RD(ins)] = (int32_t)CURRENT_STATE.REGS[RS(ins)] < (int32_t)CURRENT_STATE.REGS[RT(ins)];
					break;
				
				case 0x0: // SLL, shift left logical
					CURRENT_STATE.REGS[RD(ins)] = CURRENT_STATE.REGS[RT(ins)] << SHAMT(ins);
					break;
				
				case 0x2: // SRL
					CURRENT_STATE.REGS[RD(ins)] = CURRENT_STATE.REGS[RT(ins)] >> SHAMT(ins);
					break;
				
				case 0x22: // SUB
					CURRENT_STATE.REGS[RD(ins)] = (int32_t)CURRENT_STATE.REGS[RS(ins)] - (int32_t)CURRENT_STATE.REGS[RT(ins)];
					break;

				default:
					printf("Not available instruction\n");
					break;
			}
			break;

    	//TYPE J
	    case 0x3:		// JAL, fall through
			CURRENT_STATE.REGS[31] = CURRENT_STATE.PC;
	    
		case 0x2:		// J
			CURRENT_STATE.PC = ((CURRENT_STATE.PC >> 28) << 28) | (TARGET(ins) << 2);
			break;

	    default:
			printf("Not available instruction\n");
			break;
	}
}

// TODO: dont see where the registers are initialized to 0, but they are 0