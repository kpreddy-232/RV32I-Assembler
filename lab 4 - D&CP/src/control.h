#ifndef CONTROL_H
#define CONTROL_H

// ============================================================
// STARTER SKELETON — Session 2
// This module must NOT modify datapath.h/datapath.cpp — it only
// produces signals that plug into the existing datapath interface.
// ============================================================

#include <cstdint>
#include "datapath.h"

struct ControlSignals {
    bool RegWrite = false;
    bool ALUSrc = false;
    bool MemRead = false;
    bool MemWrite = false;
    bool MemtoReg = false;
    bool Branch = false;
    ALUOp aluOp;
};

// TODO: implement — see Cheat-Sheet.md for the control signal table
ControlSignals generate_control(uint32_t opcode, uint32_t funct3, uint32_t funct7);

#endif