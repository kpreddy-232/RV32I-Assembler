# Datapath & Control — Quick Reference

## Instruction Encoding (this lab's subset only)

```
R-type: funct7[31:25] rs2[24:20] rs1[19:15] funct3[14:12] rd[11:7]  opcode[6:0]
I-type: imm[31:20]              rs1[19:15] funct3[14:12] rd[11:7]  opcode[6:0]
S-type: imm[31:25]      rs2[24:20] rs1[19:15] funct3[14:12] imm[11:7] opcode[6:0]
B-type: imm[12|10:5]    rs2[24:20] rs1[19:15] funct3[14:12] imm[4:1|11] opcode[6:0]
```

| Instruction | opcode  | funct3 | funct7 |
|---|---|---|---|
| add | 0110011 | 000 | 0000000 |
| sub | 0110011 | 000 | 0100000 |
| and | 0110011 | 111 | 0000000 |
| or  | 0110011 | 110 | 0000000 |
| lw  | 0000011 | 010 | — |
| sw  | 0100011 | 010 | — |
| beq | 1100011 | 000 | — |

## Control Signal Table

| Instr | RegWrite | ALUSrc | MemRead | MemWrite | MemtoReg | Branch | ALU op |
|---|---|---|---|---|---|---|---|
| add | 1 | 0 | 0 | 0 | 0 | 0 | ADD |
| sub | 1 | 0 | 0 | 0 | 0 | 0 | SUB |
| and | 1 | 0 | 0 | 0 | 0 | 0 | AND |
| or  | 1 | 0 | 0 | 0 | 0 | 0 | OR  |
| lw  | 1 | 1 | 1 | 0 | 1 | 0 | ADD |
| sw  | 0 | 1 | 0 | 1 | X | 0 | ADD |
| beq | 0 | 0 | 0 | 0 | X | 1 | SUB |

- `ALUSrc = 0` → second ALU operand is a register (rs2); `= 1` → it's the immediate.
- `MemtoReg = 0` → write-back value comes from ALU result; `= 1` → from memory.
- `Branch = 1` alone doesn't mean "taken" — the actual branch decision is
  `Branch AND ALU.zero`.
- `X` = don't care (signal is irrelevant because RegWrite/MemWrite is off anyway).

## Immediate Sign-Extension Cheat

```c
// I-type (lw): bits [31:20], arithmetic shift auto sign-extends
int32_t imm = (int32_t)instruction >> 20;

// S-type (sw): bits [31:25] and [11:7], must sign-extend manually
uint32_t raw = ((instruction >> 25) << 5) | ((instruction >> 7) & 0x1F);
if (raw & 0x800) raw |= 0xFFFFF000;

// B-type (beq): bits [31],[7],[30:25],[11:8], shifted left 1, sign-extend from bit 12
uint32_t raw = (bit12 << 12) | (bit11 << 11) | (bits10_5 << 5) | (bits4_1 << 1);
if (raw & 0x1000) raw |= 0xFFFFE000;
```

## Debugging Workflow

```bash
# 1. Compile with warnings
g++ -Wall -Wextra -std=c++17 -o cpu datapath.cpp control.cpp cpu.cpp

# 2. Run and inspect per-instruction trace
./cpu program.hex

# 3. Diff against known-good output
./cpu program.hex > my_output.txt
diff my_output.txt expected_output/reference_run.txt

# 4. Cross-check against the visualizer
# https://risc-v-cpu-visualizer.vercel.app/single-stage
```

## Common GDB Commands (from Lab 01) Useful Here

```bash
gdb ./cpu
(gdb) break generate_control      # inspect control signals per instruction
(gdb) run program.hex
(gdb) print ctrl                  # view the ControlSignals struct
(gdb) print f.opcode              # view decoded instruction fields
(gdb) continue
```
