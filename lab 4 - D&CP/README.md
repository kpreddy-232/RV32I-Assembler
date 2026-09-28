# Lab — Single-Cycle RISC-V Datapath & Control Unit

## Overview

This lab teaches you to build a working **single-cycle RISC-V CPU simulator**
in C/C++, one layer at a time. By the end of this lab you will have designed
and implemented the datapath components (register file, ALU, memory,
immediate generator), a separate control unit that decodes instructions and
drives those components, and a fetch-decode-execute loop that can run a real
program — the same machine code produced by your earlier RISC-V assembler
lab.

This lab builds directly on:
- **COA-LAB-KIT Lab 01 (GDB)** and **Lab 02 (Godbolt/PIN)** — debugging and
  inspection skills you'll need while your simulator misbehaves.
- **RISC-V Assembler Lab** — your simulator's input is the machine code your
  own assembler produces.

---

## Supported Instruction Subset

To keep the design tractable, this lab only requires support for:

| Instruction | Type | Meaning |
|---|---|---|
| `add rd, rs1, rs2` | R-type | rd = rs1 + rs2 |
| `sub rd, rs1, rs2` | R-type | rd = rs1 - rs2 |
| `and rd, rs1, rs2` | R-type | rd = rs1 & rs2 |
| `or rd, rs1, rs2`  | R-type | rd = rs1 \| rs2 |
| `lw rd, offset(rs1)` | I-type | rd = Mem[rs1 + offset] |
| `sw rs2, offset(rs1)` | S-type | Mem[rs1 + offset] = rs2 |
| `beq rs1, rs2, offset` | B-type | if (rs1 == rs2) PC = PC + offset |

No shifts, multiply/divide, or floating point. See `Datapath_Control_Assignment.docx`
for full task descriptions, and `Cheat-Sheet.md` for instruction encoding and
control-signal quick reference.

---

## Session Index

| # | Focus | What you build |
|---|---|---|
| 1 | Datapath components | Register file, ALU, data memory, immediate generator, PC — each as an independent tested module |
| 2 | Control unit | Decodes opcode/funct3/funct7 into control signals; wires into Session 1's datapath *without modifying it* |
| 3 | Full execution | Fetch-decode-execute loop that runs a real assembled program and verifies output |

Pipelining is a **separate, later lab** (after the cache lab) and is *not*
part of this assignment — don't implement pipeline registers or hazard
handling here.

---

## Why "modular"?

The datapath (Session 1) and control unit (Session 2) must be independent
pieces of code with a clean interface between them — e.g. `datapath.h` /
`datapath.cpp` and `control.h` / `control.cpp` as separate files. This isn't
just a style preference:

- You can unit-test datapath components before any control logic exists.
- Bugs are easier to isolate — "is this wrong because of my ALU, or because
  of my control unit?" is only answerable if the two are separable.
- It mirrors how real CPU design and, later in the course, the pipelined
  design is built by extending exactly this same split.

---

## Verification: the RISC-V Visualizer

<https://risc-v-cpu-visualizer.vercel.app/single-stage> is a working
single-cycle RISC-V simulator.
**You do not submit anything from this website** — it exists purely so you
can check your own simulator's correctness. Load the same program into both,
run it, and compare final register/memory state. If they disagree, your
simulator has a bug — find it before submitting.

---

## Folder Structure

```
Lab-DatapathControl/
├── README.md                       
├── Cheat-Sheet.md                   ← Encoding + control signal quick reference
├── Datapath_Control_Assignment.docx ← Full assignment (Sessions 1-3)
├── src/                              ← Starter file skeletons for students
│   ├── datapath.h
│   ├── control.h
├── expected_output/
│   └── reference_run.txt            ← Sample program's correct final state
```

---

## General Workflow (each session)

```bash
# 1. Compile with warnings enabled
g++ -Wall -Wextra -std=c++17 -o cpu datapath.cpp control.cpp cpu.cpp

# 2. Run against a test program
./cpu program.hex

# 3. Compare your register/memory dump against expected_output/
#    and/or the RISC-V Visualizer
```

---

## Common Errors and Fixes

**Wrong immediate value from `lw`/`sw`/`beq`:** Double-check sign extension
— arithmetic right-shift (`(int32_t)instr >> 20`) sign-extends automatically
for I-type; S-type and B-type immediates must be manually sign-extended
since their bits are non-contiguous.

**Branch always/never taken:** Check that `Branch` AND the ALU's `zero` flag
are both required — `Branch` alone doesn't mean "taken," it means "this is a
branch instruction, check the zero flag."

**x0 not staying zero:** Register file writes to x0 must be silently
discarded, and reads from x0 must always return 0, regardless of RegWrite.

**Datapath compiles but gives wrong answers after adding control unit:**
This usually means a component's interface was modified when it shouldn't
have been. Re-run your Session 1 standalone component tests to isolate
whether the bug is in the datapath or the control wiring.
