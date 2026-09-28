#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "datapath.h"
#include "control.h"

using namespace std;

int main(int argc, char* argv[]) {
    // 1. Load Machine Code from text file
    vector<uint32_t> instructions;
    ifstream file(argv[1]);
    
    if (!file.is_open()) {
        cout << "ERROR: Could not open file " << argv[1] << "!\n";
        cout << "Make sure program.hex is in the same folder as cpu.exe.\n";
        return 1;
    }

    string hexStr;
    while (file >> hexStr) {
        instructions.push_back(stoul(hexStr, nullptr, 16));
    }

    ProgramCounter pc(0);
    InstructionMemory imem(instructions);
    RegisterFile regFile;
    DataMemory dmem(1024); // 1KB data memory
    dmem.writeWord(0, 5, true);
    dmem.writeWord(4, 10, true);

    // 3. Fetch-Decode-Execute Loop
    while (pc.get() / 4 < instructions.size()) {
        uint32_t current_pc = pc.get();
        
        // FETCH
        uint32_t instr = imem.read(current_pc);
        
        // DECODE
        DecodedFields fields = decode_fields(instr);
        ControlSignals ctrl = generate_control(fields.opcode, fields.funct3, fields.funct7);
        
        // Read Registers
        int32_t readData1 = regFile.read(fields.rs1);
        int32_t readData2 = regFile.read(fields.rs2);
        
        // Generate Immediate
        char instrType = 'R'; // Default
        if (fields.opcode == 3) instrType = 'I'; // lw
        else if (fields.opcode == 35) instrType = 'S'; // sw
        else if (fields.opcode == 101) instrType = 'B'; // beq
        int32_t imm = generate_immediate(instr, instrType);
        
        // EXECUTE (ALU)
        int32_t aluOperand2 = ctrl.ALUSrc ? imm : readData2;
        ALUResult aluRes = alu_execute(readData1, aluOperand2, ctrl.aluOp);
        
        // MEMORY
        int32_t memReadData = dmem.readWord(aluRes.result, ctrl.MemRead);
        dmem.writeWord(aluRes.result, readData2, ctrl.MemWrite);
        
        // WRITEBACK
        int32_t writeBackData = ctrl.MemtoReg ? memReadData : aluRes.result;
        regFile.write(fields.rd, writeBackData, ctrl.RegWrite);
        
        // PC UPDATE
        if (ctrl.Branch && aluRes.zero) {
            pc.set(imm); // Branch taken
        } else {
            pc.set(4);   // PC + 4
        }
    }

    // 4. Print Final State
    cout << "--- Final Register State ---\n";
    regFile.dump();
    cout << "\n--- Final Data Memory State ---\n";
    dmem.dumpNonZero();

    return 0;
}