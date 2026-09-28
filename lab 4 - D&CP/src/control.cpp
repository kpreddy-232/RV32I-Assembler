#include "control.h"

ControlSignals generate_control(uint32_t opcode, uint32_t funct3, uint32_t funct7) {
    ControlSignals cSig;

    // OPCODE = 0110011 (51) 
    // R Type Ops
    if ((int)opcode == 51) {
        cSig.RegWrite = true;
        if (funct7 == 0x20) { // Check if funct7 is 0100000 (SUB)
            if (funct3 == 0x0) cSig.aluOp = ALU_SUB;
        } else {
            if (funct3 == 0x0) cSig.aluOp = ALU_ADD; 
            if (funct3 == 0x7) cSig.aluOp = ALU_AND;
            if (funct3 == 0x6) cSig.aluOp = ALU_OR;
        }
    }

    // OPCODE = 0000011 (3) 
    // I Type Load Ops only
    if ((int)opcode == 3) {
        cSig.RegWrite = true;
        cSig.ALUSrc = true;
        cSig.MemRead = true;
        cSig.MemtoReg = true; 
        cSig.aluOp = ALU_ADD; 
    }   
    
    // OPCODE = 0100011 (35) 
    // Store Ops only
    if ((int)opcode == 35) {
        cSig.ALUSrc = true;
        cSig.MemWrite = true;
        cSig.aluOp = ALU_ADD;  
    }  

    // OPCODE = 1100011 (99 in decimal, not 101) 
    // Condn Branch Ops only
    if ((int)opcode == 99) { 
        cSig.Branch = true;
        if (funct3 == 0x0) cSig.aluOp = ALU_SUB; 
    }  

    return cSig;
}