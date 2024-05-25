# Project 3. MIPS Pipelined Simulator
Skeleton developed by CMU,
modified for KAIST CS311 purpose by THKIM, BKKIM and SHJEON.

## Instructions
There are three files you may modify: `util.h`, `run.h`, and `run.c`.

### 1. util.h

We have setup the basic CPU\_State that is sufficient to implement the project.
However, you may decide to add more variables, and modify/remove any misleading variables.

### 2. run.h

You may add any additional functions that will be called by your implementation of `process_instruction()`.
In fact, we encourage you to split your implementation of `process_instruction()` into many other helping functions.
You may decide to have functions for each stages of the pipeline.
Function(s) to handle flushes (adding bubbles into the pipeline), etc.

### 3. run.c

**Implement** the following function:

    void process_instruction()

The `process_instruction()` function is used by the `cycle()` function to simulate a `cycle` of the pipelined simulator.
Each `cycle()` the pipeline will advance to the next instruction (if there are no stalls/hazards, etc.).
Your internal register, memory, and pipeline register state should be updated according to the instruction
that is being executed at each stage.

### Variable meanings

uint32_t PC;			// program counter for the IF stage

uint32_t REGS[MIPS_REGS];	// register file

uint32_t PIPE[PIPE_STAGE];	// PC being executed at each stage

uint32_t PIPE_STALL[PIPE_STAGE]; // TRUE/FALSE if stage i is stall in current cycle

instruction* IF_ID_INST; // fetched instruction pointer

uint32_t IF_ID_NPC; // next PC

uint32_t ID_EX_NPC; // next PC

unsigned char ID_EX_REG1; // index of register 1

uint32_t ID_EX_REG1_VAL; // value of register 1

unsigned char ID_EX_REG2; // index of register 2

uint32_t ID_EX_REG2_VAL; // value of register 2

short ID_EX_IMM; // immediate value

unsigned char ID_EX_DEST; // index of destination register

uint32_t ID_EX_JVAL; // value of regs 31 for JAL

bool ID_EX_MEM_READ; // control value: read from mem

bool ID_EX_MEM_WRITE; // control value: write into mem

unsigned char ID_EX_ALU_CTRL; // ALU control value: what kind of operation will be executed

unsigned char ID_EX_SHAMT; // shift amount for shift operation

uint32_t EX_MEM_NPC; // next PC

uint32_t EX_MEM_ALU_OUT; // output of ALU operation

uint32_t EX_MEM_W_VALUE; // value to be written into mem

uint32_t EX_MEM_BR_TARGET; // address if branch is taken

bool EX_MEM_MEM_WRITE; // control value: write into mem

bool EX_MEM_MEM_READ; // control value: read from mem

bool EX_MEM_BR_TAKE; // check if this instruction is branch instruction (bne/beq)

unsigned char EX_MEM_DEST; // index of destination register

uint32_t MEM_WB_ALU_OUT; // output of ALU

bool MEM_WB_MEM_WRITE; // control value: write into mem

bool MEM_WB_MEM_READ; // control value: read from mem

unsigned char MEM_WB_DEST; // index of destination register

//Forwarding

unsigned char EX_MEM_FORWARD_REG; // index of forward register

unsigned char MEM_WB_FORWARD_REG; // index of forward register

uint32_t EX_MEM_FORWARD_VALUE; // value of forward register

uint32_t MEM_WB_FORWARD_VALUE; // value of forward register

//To choose right PC

uint32_t IF_PC; // default, pc caculated at IF stage (normally PC + 4)

uint32_t JUMP_PC; // pc if jump is taken (sometimes store branch address when BR_BIT = taken)

uint32_t BRANCH_PC; // pc if branch is taken

bool JUMP_TAKEN; // indicate if JUMP PC is selected in previous cycle