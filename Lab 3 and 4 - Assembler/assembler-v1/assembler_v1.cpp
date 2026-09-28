#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <cstdint>

using namespace std;

const unordered_map<string, uint8_t> RegisterMap = {
    {"x0", 0}, {"zero", 0}, {"x1", 1}, {"ra", 1}, {"x2", 2}, {"sp", 2},   
    {"x3", 3}, {"gp", 3}, {"x4", 4}, {"tp", 4}, {"x5", 5}, {"t0", 5},   
    {"x6", 6}, {"t1", 6}, {"x7", 7}, {"t2", 7}, {"x8", 8}, {"s0", 8},   
    {"fp", 8}, {"x9", 9}, {"s1", 9}, {"x10", 10}, {"a0", 10}, {"x11", 11}, 
    {"a1", 11}, {"x12", 12}, {"a2", 12}, {"x13", 13}, {"a3", 13}, {"x14", 14}, 
    {"a4", 14}, {"x15", 15}, {"a5", 15}, {"x16", 16}, {"a6", 16}, {"x17", 17}, 
    {"a7", 17}, {"x18", 18}, {"s2", 18}, {"x19", 19}, {"s3", 19}, {"x20", 20}, 
    {"s4", 20}, {"x21", 21}, {"s5", 21}, {"x22", 22}, {"s6", 22}, {"x23", 23}, 
    {"s7", 23}, {"x24", 24}, {"s8", 24}, {"x25", 25}, {"s9", 25}, {"x26", 26}, 
    {"s10", 26}, {"x27", 27}, {"s11", 27}, {"x28", 28}, {"t3", 28}, {"x29", 29}, 
    {"t4", 29}, {"x30", 30}, {"t5", 30}, {"x31", 31}, {"t6", 31}
};
enum class instFormat{
	rType,
	iType,
	sType,
	bType,
	jType,
	uType,
	unknown
};

struct instData{
	instFormat format;
	uint8_t opcode; // 6:0
	uint8_t funct3; // 12:14
	uint8_t funct7; // 31:25
};

unordered_map <string, instData> RV32IMap;

void initializeRV32IMap(){
	 // --- Register Type (R-Type) ---
    // Opcode = 51 (0x33)
    RV32IMap["add"] = {instFormat::rType, 0x33, 0x0, 0x00}; //
    RV32IMap["sub"] = {instFormat::rType, 0x33, 0x0, 0x20}; //
    RV32IMap["slt"] = {instFormat::rType, 0x33, 0x2, 0x00};//
    RV32IMap["sltu"] = {instFormat::rType, 0x33, 0x3, 0x00};//
    RV32IMap["and"] = {instFormat::rType, 0x33, 0x7, 0x00};//
    RV32IMap["sll"] = {instFormat::rType, 0x33, 0x1, 0x00};//
    RV32IMap["xor"] = {instFormat::rType, 0x33, 0x4, 0x00};//
    RV32IMap["srl"] = {instFormat::rType, 0x33, 0x5, 0x00};//
    RV32IMap["sra"] = {instFormat::rType, 0x33, 0x5, 0x20};//
    RV32IMap["or"] = {instFormat::rType, 0x33, 0x6, 0x00};//

    // --- Immediate Type (I-Type) Arithmetic & Logic ---
    // Opcode = 19 (0x13)
    RV32IMap["addi"] = {instFormat::iType, 0x13, 0x0, 0x00}; //
    RV32IMap["slti"] = {instFormat::iType, 0x13, 0x2, 0x00};//
    RV32IMap["sltiu"] = {instFormat::iType, 0x13, 0x3, 0x00};//
    RV32IMap["xori"] = {instFormat::iType, 0x13, 0x4, 0x00};//
    RV32IMap["ori"] = {instFormat::iType, 0x13, 0x6, 0x00};//
    RV32IMap["andi"] = {instFormat::iType, 0x13, 0x7, 0x00};//

    // --- Immediate Type (I-Type) Shifts ---
    // Opcode = 19 (0x13)
    RV32IMap["slli"] = {instFormat::iType, 0x13, 0x1, 0x00}; //
    RV32IMap["srli"] = {instFormat::iType, 0x13, 0x5, 0x00};//
    RV32IMap["srai"] = {instFormat::iType, 0x13, 0x5, 0x20}; //

    // --- Immediate Type (I-Type) Loads ---
    // Opcode = 3 (0x03)
    RV32IMap["lb"] = {instFormat::iType, 0x03, 0x0, 0x00}; //
    RV32IMap["lh"] = {instFormat::iType, 0x03, 0x1, 0x00};//
    RV32IMap["lw"] = {instFormat::iType, 0x03, 0x2, 0x00};//
    RV32IMap["lbu"] = {instFormat::iType, 0x03, 0x4, 0x00};//
    RV32IMap["lhu"] = {instFormat::iType, 0x03, 0x5, 0x00};//

    // --- Store Type (S-Type) ---
    // Opcode = 35 (0x23)
    RV32IMap["sb"] = {instFormat::sType, 0x23, 0x0, 0x00}; //
    RV32IMap["sh"] = {instFormat::sType, 0x23, 0x1, 0x00};//
    RV32IMap["sw"] = {instFormat::sType, 0x23, 0x2, 0x00};//

    // --- Branch Type (B-Type) ---
    // Opcode = 99 (0x63)
    RV32IMap["beq"] = {instFormat::bType, 0x63, 0x0, 0x00}; //
    RV32IMap["bne"] = {instFormat::bType, 0x63, 0x1, 0x00};//
    RV32IMap["blt"] = {instFormat::bType, 0x63, 0x4, 0x00};//
    RV32IMap["bge"] = {instFormat::bType, 0x63, 0x5, 0x00};//
    RV32IMap["bltu"] = {instFormat::bType, 0x63, 0x6, 0x00};//
    RV32IMap["bgeu"] = {instFormat::bType, 0x63, 0x7, 0x00};//

    // --- Jump Type (J-Type / I-Type) ---
    RV32IMap["jal"] = {instFormat::jType, 0x6F, 0x0, 0x00}; // JAL OPCODE = 111 (0x6F)
    RV32IMap["jalr"] = {instFormat::iType, 0x67, 0x0, 0x00}; // JALR OPCODE = 103 (0x67)

    // --- Upper Immediate Type (U-Type) ---
    RV32IMap["lui"] = {instFormat::uType, 0x37, 0x0, 0x00}; // OPCODE = 0x37
    RV32IMap["auipc"] = {instFormat::uType, 0x17, 0x0, 0x00}; // OPCODE = 0x17

    // --- System / Environment (I-Type) ---
    // OPCODE = 0x73
    RV32IMap["ecall"] = {instFormat::iType, 0x73, 0x0, 0x00};
    RV32IMap["ebreak"] = {instFormat::iType, 0x73, 0x0, 0x00};
}


struct ParsedLine {
    string label;               
    vector<string> tokens; 
    string originalText;        
};

string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t");
    if (first == string::npos) return ""; 
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

vector<ParsedLine> tokenize(const string& filename) {
    vector<ParsedLine> program;
    ifstream file(filename);
    string line;

    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << "\n";
        return program;
    }

    while (getline(file, line)) {
        ParsedLine parsed;
        parsed.originalText = line;

        size_t commentPos = line.find('#');
        if (commentPos != string::npos) {
            line = line.substr(0, commentPos);
        }

        line = trim(line);
        if (line.empty()) continue;

        // Extract Label
        size_t colonPos = line.find(':');
        if (colonPos != string::npos) {
            parsed.label = trim(line.substr(0, colonPos)); 
            line = line.substr(colonPos + 1); 
        }

        // Tokenize mnemonics and operands
        replace(line.begin(), line.end(), ',', ' ');
        replace(line.begin(), line.end(), '(', ' ');
        replace(line.begin(), line.end(), ')', ' ');

        stringstream ss(line);
        string token;
        while (ss >> token) {
            parsed.tokens.push_back(token);
        }

        if (!parsed.label.empty() || !parsed.tokens.empty()) {
            program.push_back(parsed);
        }
    }

    file.close();
    return program;
}


unordered_map<string, uint32_t> SymbolTable;

uint32_t encodeRType(uint8_t op, uint8_t rd, uint8_t f3, uint8_t rs1, uint8_t rs2, uint8_t f7) {
    return (op & 0x7F) | ((rd & 0x1F) << 7) | ((f3 & 0x7) << 12) | 
           ((rs1 & 0x1F) << 15) | ((rs2 & 0x1F) << 20) | ((f7 & 0x7F) << 25);
}

uint32_t encodeIType(uint8_t op, uint8_t rd, uint8_t f3, uint8_t rs1, uint16_t imm) {
    return (op & 0x7F) | ((rd & 0x1F) << 7) | ((f3 & 0x7) << 12) | 
           ((rs1 & 0x1F) << 15) | ((imm & 0xFFF) << 20);
}

uint32_t encodeSType(uint8_t op, uint8_t f3, uint8_t rs1, uint8_t rs2, uint16_t imm) {
    return (op & 0x7F) | ((imm & 0x1F) << 7) | ((f3 & 0x7) << 12) | 
           ((rs1 & 0x1F) << 15) | ((rs2 & 0x1F) << 20) | (((imm >> 5) & 0x7F) << 25);
}

uint32_t encodeBType(uint8_t op, uint8_t f3, uint8_t rs1, uint8_t rs2, uint16_t imm) {
    return (op & 0x7F) | (((imm >> 11) & 0x1) << 7) | (((imm >> 1) & 0xF) << 8) | 
           ((f3 & 0x7) << 12) | ((rs1 & 0x1F) << 15) | ((rs2 & 0x1F) << 20) | 
           (((imm >> 5) & 0x3F) << 25) | (((imm >> 12) & 0x1) << 31);
} 

uint32_t encodeUType(uint8_t op, uint8_t rd, uint32_t imm) {
    return (op & 0x7F) | ((rd & 0x1F) << 7) | (imm & 0xFFFFF000);
}

uint32_t encodeJType(uint8_t op, uint8_t rd, uint32_t imm) {
    return (op & 0x7F) | ((rd & 0x1F) << 7) | (((imm >> 12) & 0xFF) << 12) | 
           (((imm >> 11) & 0x1) << 20) | (((imm >> 1) & 0x3FF) << 21) | (((imm >> 20) & 0x1) << 31);
}



int main() {
    initializeRV32IMap();
    string inputFile = "test.s";
    string outputFile = "output.bin";
    
    vector<ParsedLine> program = tokenize(inputFile);
    if (program.empty()) return 1;

    // Pass 1: Symbol Table
    uint32_t PC = 0x00000000;
    for (const auto& line : program) {
        if (!line.label.empty()) SymbolTable[line.label] = PC;
        if (!line.tokens.empty()) PC += 4;
    }

    // Pass 2: Encoding
    PC = 0;
    ofstream out(outputFile, ios::binary);
    for (const auto& line : program) {
        if (line.tokens.empty()) continue;

        string mnemonic = line.tokens[0];
        instData data = RV32IMap[mnemonic];
        uint32_t machineInstruction = 0;

        try {
            if (data.format == instFormat::rType) {
                machineInstruction = encodeRType(data.opcode, RegisterMap.at(line.tokens[1]), 
                                                 data.funct3, RegisterMap.at(line.tokens[2]), 
                                                 RegisterMap.at(line.tokens[3]), data.funct7);
            } 
            else if (data.format == instFormat::iType) {
                uint16_t imm = stoi(line.tokens[3]);
                machineInstruction = encodeIType(data.opcode, RegisterMap.at(line.tokens[1]), 
                                                 data.funct3, RegisterMap.at(line.tokens[2]), imm);
            }
            else if (data.format == instFormat::sType) {
                uint16_t imm = stoi(line.tokens[2]);
                machineInstruction = encodeSType(data.opcode, data.funct3, RegisterMap.at(line.tokens[3]), 
                                                 RegisterMap.at(line.tokens[1]), imm);
            } 
            else if (data.format == instFormat::bType) {
                int32_t offset = SymbolTable[line.tokens[3]] - PC;
                machineInstruction = encodeBType(data.opcode, data.funct3, RegisterMap.at(line.tokens[1]), 
                                                 RegisterMap.at(line.tokens[2]), (uint16_t)offset);
            } 
            else if (data.format == instFormat::uType) {
                uint32_t imm = stoul(line.tokens[2]);
                machineInstruction = encodeUType(data.opcode, RegisterMap.at(line.tokens[1]), imm);
            } 
            else if (data.format == instFormat::jType) {
                int32_t offset = SymbolTable[line.tokens[2]] - PC;
                machineInstruction = encodeJType(data.opcode, RegisterMap.at(line.tokens[1]), (uint32_t)offset);
            }
        } 
        
        catch (...) {
            cerr << "Error parsing operands for: " << mnemonic << "\n";
        }

        out.write(reinterpret_cast<const char*>(&machineInstruction), sizeof(machineInstruction));
        PC += 4;
    }
    out.close();
    cout << "Successfully generated " << outputFile << "!\n";
    return 0;
}
