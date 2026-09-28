#ifndef DATAPATH_H
#define DATAPATH_H

#include <cstdint>
#include <vector>

// ---------- Program Counter ----------
class ProgramCounter {
public: 
    ProgramCounter(uint32_t initPC);
    uint32_t get() const;
    void set(uint32_t value);
private:
    uint32_t pc;
};

// ---------- Instruction Memory ----------
class InstructionMemory {
public: 
    InstructionMemory(std::vector<uint32_t>& instr);
    uint32_t read(uint32_t addr) const;
private:
    std::vector<uint32_t> memory;
};

// ---------- Register File ----------
class RegisterFile {
public:
    RegisterFile();
    int32_t read(int reg) const;
    void write(int reg, int32_t value, bool regWrite);
    void dump() const;
private:
    int32_t regs[32];
};

// ---------- ALU ----------
enum ALUOp { ALU_ADD, ALU_SUB, ALU_AND, ALU_OR };

struct ALUResult {
    int32_t result;
    bool zero;
};

ALUResult alu_execute(int32_t a, int32_t b, ALUOp op);

// ---------- Data Memory ----------
class DataMemory {
public:
    DataMemory(size_t sizeBytes);
    int32_t readWord(uint32_t addr, bool memRead) const;
    void writeWord(uint32_t addr, int32_t value, bool memWrite);
    void dumpNonZero() const;
private:
    std::vector<uint8_t> memory; 
};

// ---------- Immediate Generator ----------
int32_t generate_immediate(uint32_t instruction, char instrType);

// ---------- Field decode ----------
struct DecodedFields {
    uint32_t opcode, rd, funct3, rs1, rs2, funct7;
};

DecodedFields decode_fields(uint32_t instruction);

#endif