/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   CS311 KAIST                                               */
/*   parse.c                                                   */
/*                                                             */
/***************************************************************/

#include <stdio.h>

#include "util.h"
#include "parse.h"

const int WORD_SIZE = 32;
/// position in input order
const int OP_POS = 0;
const int RS_POS = 6;
const int RT_POS = 11;
const int RD_POS = 16;
const int SFT_POS = 21;
const int FT_POS = 26;

int text_size;
int data_size;

static int text_current_address = MEM_TEXT_START;
static int data_current_address = MEM_DATA_START;

int substrto2complement(const char *buffer, const int s, const int t, short coef) {
	int ans = 0;
	ans = coef * (1 << (t - s - 1)) * (buffer[s] - '0');
	for(int i = s + 1; i < t; ++i) {
		ans += (1 << (t - i - 1)) * (buffer[i] - '0');
	}
	return ans;
}

int classify_instruction(int opcode) {
	switch(opcode) {
	    //Type I
	    case 0x8:		// ADDI
	    case 0xc:		// ANDI
	    case 0xf:		// LUI	
	    case 0xd:		// ORI
	    case 0xa:		// SLTI
	    case 0x23:		// LW	
	    case 0x2b:		// SW
	    case 0x4:		// BEQ
	    case 0x5:		// BNE
			return 1;

    	//TYPE R
	    case 0x0:		// ADD, AND, NOR, OR, SLT, SLL, SRL, SUB, JR
			return 0;

    	//TYPE J
	    case 0x2:		// J
	    case 0x3:		// JAL
			return 2;

	    default:
			printf("Not available instruction\n");
			return 0;
	}
}

instruction parsing_instr(const char *buffer, const int index)
{
    instruction instr;
    instr.opcode = substrto2complement(buffer, OP_POS, RS_POS, 1);
	if (classify_instruction(instr.opcode) == 0) {
		instr.r_t.r_i.rs = substrto2complement(buffer, RS_POS, RT_POS, 1);
		instr.r_t.r_i.rt = substrto2complement(buffer, RT_POS, RD_POS, 1);
		instr.r_t.r_i.r_i.r.rd = substrto2complement(buffer, RD_POS, SFT_POS, 1);
		instr.r_t.r_i.r_i.r.shamt = substrto2complement(buffer, SFT_POS, FT_POS, 1);
		instr.func_code = substrto2complement(buffer, FT_POS, WORD_SIZE, 1);
	} else if(classify_instruction(instr.opcode) == 1) {
		instr.r_t.r_i.rs = substrto2complement(buffer, RS_POS, RT_POS, 1);
		instr.r_t.r_i.rt = substrto2complement(buffer, RT_POS, RD_POS, 1);
		instr.r_t.r_i.r_i.imm = substrto2complement(buffer, RD_POS, WORD_SIZE, -1);
	} else {
		instr.r_t.target = substrto2complement(buffer, RS_POS, WORD_SIZE, 1);
	}
	mem_write_32(text_current_address, fromBinary((char *) buffer));
	text_current_address += BYTES_PER_WORD;
    return instr;
}

void parsing_data(const char *buffer, const int index)
{
	mem_write_32(data_current_address, fromBinary((char *) buffer));
	data_current_address += BYTES_PER_WORD;
}

void print_parse_result()
{
    int i;
    printf("Instruction Information\n");

    for(i = 0; i < text_size/4; i++)
    {
	printf("INST_INFO[%d].value : %x\n",i, INST_INFO[i].value);
	printf("INST_INFO[%d].opcode : %d\n",i, INST_INFO[i].opcode);

	switch(INST_INFO[i].opcode)
	{
	    //Type I
	    case 0x8:		// ADDI
	    case 0xc:		// ANDI
	    case 0xf:		// LUI	
	    case 0xd:		// ORI
	    case 0xa:		// SLTI
	    case 0x23:		// LW	
	    case 0x2b:		// SW
	    case 0x4:		// BEQ
	    case 0x5:		// BNE
		printf("INST_INFO[%d].rs : %d\n",i, INST_INFO[i].r_t.r_i.rs);
		printf("INST_INFO[%d].rt : %d\n",i, INST_INFO[i].r_t.r_i.rt);
		printf("INST_INFO[%d].imm : %d\n",i, INST_INFO[i].r_t.r_i.r_i.imm);
		break;

    	//TYPE R
	    case 0x0:		// ADD, AND, NOR, OR, SLT, SLL, SRL, SUB, JR
		printf("INST_INFO[%d].func_code : %d\n",i, INST_INFO[i].func_code);
		printf("INST_INFO[%d].rs : %d\n",i, INST_INFO[i].r_t.r_i.rs);
		printf("INST_INFO[%d].rt : %d\n",i, INST_INFO[i].r_t.r_i.rt);
		printf("INST_INFO[%d].rd : %d\n",i, INST_INFO[i].r_t.r_i.r_i.r.rd);
		printf("INST_INFO[%d].shamt : %d\n",i, INST_INFO[i].r_t.r_i.r_i.r.shamt);
		break;

    	//TYPE J
	    case 0x2:		// J
	    case 0x3:		// JAL
		printf("INST_INFO[%d].target : %d\n",i, INST_INFO[i].r_t.target);
		break;

	    default:
		printf("Not available instruction\n");
		assert(0);
	}
    }

    printf("Memory Dump - Text Segment\n");
    for(i = 0; i < text_size; i+=4)
	printf("text_seg[%d] : %x\n", i, mem_read_32(MEM_TEXT_START + i));
    for(i = 0; i < data_size; i+=4)
	printf("data_seg[%d] : %x\n", i, mem_read_32(MEM_DATA_START + i));
    printf("Current PC: %x\n", CURRENT_STATE.PC);
}
