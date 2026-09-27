#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <cstdint>
#include <stdexcept>

// ============================================================================
// 1. DATA STRUCTURES
// ============================================================================

enum class InstFormat { R_TYPE, I_TYPE, S_TYPE, B_TYPE, U_TYPE, J_TYPE, UNKNOWN };

struct InstructionData {
    InstFormat format;
    uint8_t opcode; 
    uint8_t funct3; 
    uint8_t funct7; 
};

struct ParsedLine {
    std::string label;               
    std::vector<std::string> tokens; 
};

// ============================================================================
// 2. ARCHITECTURE CONFIGURATION (Encapsulates ISA Maps)
// ============================================================================
class Architecture {
private:
    std::unordered_map<std::string, uint8_t> registerMap;
    std::unordered_map<std::string, InstructionData> isaMap;

public:
    Architecture() {
        initializeRegisters();
        initializeISA();
    }

    uint8_t getRegister(const std::string& regName) const {
        return registerMap.at(regName);
    }

    InstructionData getInstruction(const std::string& mnemonic) const {
        return isaMap.at(mnemonic);
    }

    bool instructionExists(const std::string& mnemonic) const {
        return isaMap.find(mnemonic) != isaMap.end();
    }

private:
    void initializeRegisters() {
        registerMap = {
            {"x0", 0},   {"zero", 0}, {"x1", 1},   {"ra", 1},   {"x2", 2},   {"sp", 2},   
            {"x3", 3},   {"gp", 3},   {"x4", 4},   {"tp", 4},   {"x5", 5},   {"t0", 5},   
            {"x6", 6},   {"t1", 6},   {"x7", 7},   {"t2", 7},   {"x8", 8},   {"s0", 8},   
            {"fp", 8},   {"x9", 9},   {"s1", 9},   {"x10", 10}, {"a0", 10},  {"x11", 11}, 
            {"a1", 11},  {"x12", 12}, {"a2", 12},  {"x13", 13}, {"a3", 13},  {"x14", 14}, 
            {"a4", 14},  {"x15", 15}, {"a5", 15},  {"x16", 16}, {"a6", 16},  {"x17", 17}, 
            {"a7", 17},  {"x18", 18}, {"s2", 18},  {"x19", 19}, {"s3", 19},  {"x20", 20}, 
            {"s4", 20},  {"x21", 21}, {"s5", 21},  {"x22", 22}, {"s6", 22},  {"x23", 23}, 
            {"s7", 23},  {"x24", 24}, {"s8", 24},  {"x25", 25}, {"s9", 25},  {"x26", 26}, 
            {"s10", 26}, {"x27", 27}, {"s11", 27}, {"x28", 28}, {"t3", 28},  {"x29", 29}, 
            {"t4", 29},  {"x30", 30}, {"t5", 30},  {"x31", 31}, {"t6", 31}
        };
    }

    void initializeISA() {
        isaMap["add"]  = {InstFormat::R_TYPE, 0x33, 0x0, 0x00}; 
        isaMap["sub"]  = {InstFormat::R_TYPE, 0x33, 0x0, 0x20}; 
        isaMap["addi"] = {InstFormat::I_TYPE, 0x13, 0x0, 0x00}; 
        isaMap["lw"]   = {InstFormat::I_TYPE, 0x03, 0x2, 0x00};
        isaMap["sw"]   = {InstFormat::S_TYPE, 0x23, 0x2, 0x00}; 
        isaMap["beq"]  = {InstFormat::B_TYPE, 0x63, 0x0, 0x00}; 
        isaMap["jal"]  = {InstFormat::J_TYPE, 0x6F, 0x0, 0x00}; 
        isaMap["lui"]  = {InstFormat::U_TYPE, 0x37, 0x0, 0x00}; 
        isaMap["jalr"] = {InstFormat::I_TYPE, 0x67, 0x0, 0x00};
        isaMap["mul"]  = {InstFormat::R_TYPE, 0x33, 0x0, 0x01};
    }
};

// ============================================================================
// 3. LEXER (Handles File I/O and Text Parsing)
// ============================================================================
class Lexer {
public:
    static std::vector<ParsedLine> tokenizeFile(const std::string& filename) {
        std::vector<ParsedLine> program;
        std::ifstream file(filename);
        std::string line;

        if (!file.is_open()) {
            throw std::runtime_error("Could not open file: " + filename);
        }

        while (std::getline(file, line)) {
            ParsedLine parsed = parseLine(line);
            if (!parsed.label.empty() || !parsed.tokens.empty()) {
                program.push_back(parsed);
            }
        }
        return program;
    }

private:
    static std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t");
        if (first == std::string::npos) return ""; 
        size_t last = str.find_last_not_of(" \t\r\n");
        return str.substr(first, (last - first + 1));
    }

    static ParsedLine parseLine(std::string line) {
        ParsedLine parsed;
        
        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos) line = line.substr(0, commentPos);
        line = trim(line);
        if (line.empty()) return parsed;

        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            parsed.label = trim(line.substr(0, colonPos)); 
            line = line.substr(colonPos + 1); 
        }

        std::replace(line.begin(), line.end(), ',', ' ');
        std::replace(line.begin(), line.end(), '(', ' ');
        std::replace(line.begin(), line.end(), ')', ' ');

        std::stringstream ss(line);
        std::string token;
        while (ss >> token) parsed.tokens.push_back(token);

        return parsed;
    }
};

// ============================================================================
// 4. ENCODER (Stateless Bitwise Packing)
// ============================================================================
class Encoder {
public:
    static uint32_t encodeR(uint8_t op, uint8_t rd, uint8_t f3, uint8_t rs1, uint8_t rs2, uint8_t f7) {
        return (op & 0x7F) | ((rd & 0x1F) << 7) | ((f3 & 0x7) << 12) | 
               ((rs1 & 0x1F) << 15) | ((rs2 & 0x1F) << 20) | ((f7 & 0x7F) << 25);
    }

    static uint32_t encodeI(uint8_t op, uint8_t rd, uint8_t f3, uint8_t rs1, uint16_t imm) {
        return (op & 0x7F) | ((rd & 0x1F) << 7) | ((f3 & 0x7) << 12) | 
               ((rs1 & 0x1F) << 15) | ((imm & 0xFFF) << 20);
    }

    static uint32_t encodeS(uint8_t op, uint8_t f3, uint8_t rs1, uint8_t rs2, uint16_t imm) {
        return (op & 0x7F) | ((imm & 0x1F) << 7) | ((f3 & 0x7) << 12) | 
               ((rs1 & 0x1F) << 15) | ((rs2 & 0x1F) << 20) | (((imm >> 5) & 0x7F) << 25);
    }

    static uint32_t encodeB(uint8_t op, uint8_t f3, uint8_t rs1, uint8_t rs2, uint16_t imm) {
        return (op & 0x7F) | (((imm >> 11) & 0x1) << 7) | (((imm >> 1) & 0xF) << 8) | 
               ((f3 & 0x7) << 12) | ((rs1 & 0x1F) << 15) | ((rs2 & 0x1F) << 20) | 
               (((imm >> 5) & 0x3F) << 25) | (((imm >> 12) & 0x1) << 31);
    }

    static uint32_t encodeU(uint8_t op, uint8_t rd, uint32_t imm) {
        return (op & 0x7F) | ((rd & 0x1F) << 7) | (imm & 0xFFFFF000);
    }

    static uint32_t encodeJ(uint8_t op, uint8_t rd, uint32_t imm) {
        return (op & 0x7F) | ((rd & 0x1F) << 7) | (((imm >> 12) & 0xFF) << 12) | 
               (((imm >> 11) & 0x1) << 20) | (((imm >> 1) & 0x3FF) << 21) | (((imm >> 20) & 0x1) << 31);
    }
};

// ============================================================================
// 5. CORE ASSEMBLER (Manages state and passes)
// ============================================================================
class Assembler {
private:
    Architecture arch;
    std::unordered_map<std::string, uint32_t> symbolTable;
    std::vector<ParsedLine> programTokens;
    uint32_t programCounter;

public:
    void assemble(const std::string& inputFile, const std::string& outputFile) {
        try {
            programTokens = Lexer::tokenizeFile(inputFile);
            passOne();
            passTwo(outputFile);
        } catch (const std::exception& e) {
            std::cerr << "Assembly Failed: " << e.what() << "\n";
        }
    }

private:
    void passOne() {
        programCounter = 0x00000000;
        for (const auto& line : programTokens) {
            if (!line.label.empty()) {
                symbolTable[line.label] = programCounter;
            }
            if (!line.tokens.empty()) {
                programCounter += 4;
            }
        }
    }

    void passTwo(const std::string& outputFile) {
        programCounter = 0x00000000;
        std::ofstream out(outputFile, std::ios::binary);
        
        if (!out.is_open()) {
            throw std::runtime_error("Could not create output file: " + outputFile);
        }

        for (const auto& line : programTokens) {
            if (line.tokens.empty()) continue;

            std::string mnemonic = line.tokens[0];
            
            if (!arch.instructionExists(mnemonic)) {
                std::cout << "Notice: Skipping unknown instruction/directive: " << mnemonic << "\n";
                continue; 
            }

            InstructionData data = arch.getInstruction(mnemonic);
            uint32_t machineInstruction = 0;

            try {
                machineInstruction = buildInstruction(data, line);
            } 
            catch (const std::out_of_range& e) {
                std::cerr << "Error: Missing operand or unknown register/label for: " << mnemonic << "\n";
                continue; 
            }
            catch (const std::invalid_argument& e) {
                std::cerr << "Error: Invalid immediate value for: " << mnemonic << "\n";
                continue;
            }

            out.write(reinterpret_cast<const char*>(&machineInstruction), sizeof(machineInstruction));
            programCounter += 4;
        }
        
        out.close();
        std::cout << "Successfully generated " << outputFile << "!\n";
    }

    uint32_t buildInstruction(const InstructionData& data, const ParsedLine& line) {
        switch (data.format) {
            case InstFormat::R_TYPE:
                return Encoder::encodeR(data.opcode, arch.getRegister(line.tokens.at(1)), 
                                        data.funct3, arch.getRegister(line.tokens.at(2)), 
                                        arch.getRegister(line.tokens.at(3)), data.funct7);
            case InstFormat::I_TYPE:
                return Encoder::encodeI(data.opcode, arch.getRegister(line.tokens.at(1)), 
                                        data.funct3, arch.getRegister(line.tokens.at(2)), 
                                        (uint16_t)std::stoi(line.tokens.at(3)));
            case InstFormat::S_TYPE:
                return Encoder::encodeS(data.opcode, data.funct3, arch.getRegister(line.tokens.at(3)), 
                                        arch.getRegister(line.tokens.at(1)), (uint16_t)std::stoi(line.tokens.at(2)));
            case InstFormat::B_TYPE:
                return Encoder::encodeB(data.opcode, data.funct3, arch.getRegister(line.tokens.at(1)), 
                                        arch.getRegister(line.tokens.at(2)), 
                                        (uint16_t)(symbolTable.at(line.tokens.at(3)) - programCounter));
            case InstFormat::U_TYPE:
                return Encoder::encodeU(data.opcode, arch.getRegister(line.tokens.at(1)), std::stoul(line.tokens.at(2)));
            case InstFormat::J_TYPE:
                return Encoder::encodeJ(data.opcode, arch.getRegister(line.tokens.at(1)), 
                                        (uint32_t)(symbolTable.at(line.tokens.at(2)) - programCounter));
            default:
                return 0;
        }
    }
};

// ============================================================================
// MAIN EXECUTION
// ============================================================================
int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_file.s> <output_file.bin>\n";
        return 1;
    }

    Assembler assembler;
    assembler.assemble(argv[1], argv[2]);
    
    return 0;
}