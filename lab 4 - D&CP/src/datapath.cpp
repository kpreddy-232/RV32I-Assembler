#include "datapath.h"
#include <iostream>

using namespace std;

// ---------- Program Counter ----------
ProgramCounter::ProgramCounter(uint32_t initPC) {
    pc = initPC;
}

uint32_t ProgramCounter::get() const {
    return pc;
}

void ProgramCounter::set(uint32_t value) {
    pc += value;
}

// ---------- Instruction Memory ----------
InstructionMemory::InstructionMemory(vector<uint32_t>& instr) {
    memory = instr;
}

uint32_t InstructionMemory::read(uint32_t addr) const {
    return memory[addr / 4];
}

// ---------- Register File ----------
RegisterFile::RegisterFile() {
    for (int i = 0; i < 32; i++) regs[i] = 0;
}    

int32_t RegisterFile::read(int reg) const {
    if (reg == 0) return 0;
    return regs[reg];
}

void RegisterFile::write(int reg, int32_t value, bool RegWrite) {
    if (!RegWrite || reg == 0) return;
    regs[reg] = value;
}

void RegisterFile::dump() const {
    for (int i = 0; i < 32; i++) {
        cout << "x" << i << " : " << regs[i] << "\n";
    }
}

// ---------- ALU ----------
ALUResult alu_execute(int32_t a, int32_t b, ALUOp op) {
    ALUResult res;
    res.result = 0;
    res.zero = false;
    switch(op) {
        case ALU_ADD:
            res.result = a + b;
            break;
        case ALU_SUB:
            res.result = a - b;
            break;			
        case ALU_AND:
            res.result = a & b;
            break;
        case ALU_OR:
            res.result = a | b;
            break;
        default:
            res.result = 0;
            break;
    }
    if (res.result == 0) res.zero = true;
    return res;
}

// ---------- Data Memory ----------
DataMemory::DataMemory(size_t sizeBytes) {
    memory.resize(sizeBytes, 0);
}

int32_t DataMemory::readWord(uint32_t addr, bool memRead) const {
    if (!memRead) return 0;
    return (memory[addr] | memory[addr + 1] << 8 | memory[addr + 2] << 16 | memory[addr + 3] << 24);
}

void DataMemory::writeWord(uint32_t addr, int32_t value, bool memWrite) {
    if (!memWrite) return;
    
    memory[addr] = value & 0xFF;
    memory[addr + 1] = (value >> 8) & 0xFF;
    memory[addr + 2] = (value >> 16) & 0xFF;
    memory[addr + 3] = (value >> 24) & 0xFF;
}

void DataMemory::dumpNonZero() const {
    for (size_t i = 0; i < memory.size(); i++) {
        if (memory[i]) {
            cout << "Address " << i << " : " << static_cast<int>(memory[i]) << "\n"; 
        }
    }
}

// ---------- Immediate Generator ----------
int32_t generate_immediate(uint32_t instruction, char instrType) {
    int32_t imm = 0x0;
    
    if (instrType == 'I') {
        imm = (int32_t)instruction >> 20; 
    }

    if (instrType == 'S') { 
        imm = ((instruction >> 25) << 5) | ((instruction >> 7) & 0x1F); 
        if (imm & 0x800) imm |= 0xFFFFF000;
        if (imm & 0x1000) imm |= 0xFFFFE000;
    }
    
    if (instrType == 'B') {
        int32_t bit12  = (instruction >> 31) & 0x1;
        int32_t bit11  = (instruction >> 7) & 0x1;
        int32_t bits10_5 = (instruction >> 25) & 0x3F;
        int32_t bits4_1  = (instruction >> 8) & 0xF;
        
        imm = (bit12 << 12) | (bit11 << 11) | (bits10_5 << 5) | (bits4_1 << 1);
        if (imm & 0x1000) imm |= 0xFFFFE000;
    }
    return imm; 
}

// ---------- Field decode ----------
DecodedFields decode_fields(uint32_t instruction) {
    DecodedFields df;
    
    df.opcode = instruction & 0x7F;
    df.rd = (instruction >> 7) & 0x1F;
    df.funct3 = (instruction >> 12) & 0x7;
    df.rs1 = (instruction >> 15) & 0x1F;
    df.rs2 = (instruction >> 20) & 0x1F;
    df.funct7 = (instruction >> 25) & 0x7F;
    
    return df;
}