# 8-bit Virtual CPU Emulator and Assembler

An 8-bit virtual CPU emulator and custom assembler implemented in C. This project explores computer architecture, instruction sets, registers, memory, ALU operations, status flags, and the fetch-decode-execute cycle.

The project consists of two independent C programs: an assembler that converts human readable assembly instructions into machine code, and a CPU emulator that loads and executes the generated binary.

## Features

- Custom 8-bit CPU architecture.
- Eight general-purpose 8-bit registers (`R0`–`R7`).
- 256 bytes of unified memory for instructions and data.
- Custom instruction set containing 20 opcodes.
- Arithmetic Logic Unit (ALU) supporting arithmetic, bitwise operations, and shifts.
- Zero, Carry, Negative, and Overflow flags.
- Instruction decoding and fetch-decode-execute cycle.
- Conditional and unconditional branching.
- Separate assembler that generates a 256-byte binary file.
- Final CPU state output, including registers, flags, and program counter.

## Project Structure

```text
project/
├── assembler.c
├── instructions.txt
├── opcodeTable.txt
├── program.bin
├── program.txt
├── virProEmu1.c
└── virProVEmu1-NoComments.c
```

- `assembler.c` — assembler implementation.
- `virProEmu1.c` — CPU emulator source code with explanatory comments.
- `virProVEmu1-NoComments.c` — CPU emulator source code without comments.
- `instructions.txt` — instruction reference.
- `opcodeTable.txt` — opcode reference.
- `program.txt` — assembly program input for the assembler.
- `program.bin` — generated machine-code binary consumed by the CPU emulator.

## Architecture

The virtual CPU contains the following components:

| Component | Description |
|---|---|
| General-purpose registers | Eight 8-bit registers, `R0`–`R7` |
| Memory | 256 bytes of unified instruction and data memory |
| Program Counter (PC) | Tracks the address of the next instruction |
| Instruction Register (IR) | Holds the currently fetched opcode |
| Arithmetic Logic Unit (ALU) | Performs arithmetic, bitwise, and shift operations |
| Control Unit (CU) | Fetches, decodes, and executes instructions |
| Flags register | Stores Zero, Carry, Negative, and Overflow status flags |

### Memory Model

The CPU uses a unified memory model, meaning instructions and data share the same 256-byte memory array.

The Program Counter identifies where the CPU fetches its next instruction. Instructions can also read from or write to memory through the `LD` and `ST` operations.

## Instruction Set Architecture (ISA)

The CPU implements a custom instruction set containing 20 instructions, with opcode values ranging from `0x00` to `0x13`.

### Operand Conventions

| Operand | Meaning | Valid range |
|---|---|---|
| `R` | Register number | `0–7` |
| `Rd` | Destination register | `0–7` |
| `Rs` | Source register | `0–7` |
| `imm8` | 8-bit immediate value | `0–255` |
| `addr` | Memory address or jump target | `0–255` |

Registers are specified using numbers rather than names. For example, `LDI 0 10` loads the value `10` into register `R0`.

### Complete Instruction Reference

| Opcode | Instruction | Size | Operands | Description |
|---|---|---:|---|---|
| `0x00` | `NOP` | 1 byte | None | Performs no operation. |
| `0x01` | `HALT` | 1 byte | None | Stops CPU execution. |
| `0x02` | `LDI` | 3 bytes | `Rd, imm8` | Loads an immediate value into a register. |
| `0x03` | `MOV` | 3 bytes | `Rd, Rs` | Copies the value of a source register into a destination register. |
| `0x04` | `LD` | 3 bytes | `R, addr` | Loads a byte from memory into a register. |
| `0x05` | `ST` | 3 bytes | `R, addr` | Stores a register's value at a memory address. |
| `0x06` | `ADD` | 3 bytes | `Rd, Rs` | Adds `Rs` to `Rd` and stores the result in `Rd`. |
| `0x07` | `SUB` | 3 bytes | `Rd, Rs` | Subtracts `Rs` from `Rd` and stores the result in `Rd`. |
| `0x08` | `CMP` | 3 bytes | `R, Rs` | Compares two registers using subtraction and updates flags without storing the result. |
| `0x09` | `AND` | 3 bytes | `Rd, Rs` | Performs bitwise AND and stores the result in `Rd`. |
| `0x0A` | `OR` | 3 bytes | `Rd, Rs` | Performs bitwise OR and stores the result in `Rd`. |
| `0x0B` | `XOR` | 3 bytes | `Rd, Rs` | Performs bitwise XOR and stores the result in `Rd`. |
| `0x0C` | `NOT` | 2 bytes | `R` | Inverts all bits of a register. |
| `0x0D` | `SHL` | 3 bytes | `Rd, Rs` | Shifts the value in `Rd` left by the number of positions specified by the value in `Rs`. |
| `0x0E` | `SHR` | 3 bytes | `Rd, Rs` | Shifts the value in `Rd` right by the number of positions specified by the value in `Rs`. |
| `0x0F` | `JMP` | 2 bytes | `addr` | Unconditionally jumps to the specified address. |
| `0x10` | `JZ` | 2 bytes | `addr` | Jumps if the Zero flag is set. |
| `0x11` | `JNZ` | 2 bytes | `addr` | Jumps if the Zero flag is clear. |
| `0x12` | `JC` | 2 bytes | `addr` | Jumps if the Carry flag is set. |
| `0x13` | `JN` | 2 bytes | `addr` | Jumps if the Negative flag is set. |

**Instruction encoding:** Instructions occupy 1, 2, or 3 bytes depending on the opcode. Jump targets are absolute memory addresses, not instruction numbers or source-code line numbers.

### Flags

The CPU uses four status flags:

| Flag | Name | Meaning |
|---|---|---|
| `Z` | Zero | Set when the ALU result is zero. |
| `C` | Carry | Indicates carry according to the arithmetic operation's implementation. |
| `N` | Negative | Set when bit 7 of the result is `1`. |
| `O` | Overflow | Indicates signed arithmetic overflow. |

Conditional branching instructions use these flags to determine whether a jump should occur.

## Example Assembly Program

The following program loads two values, adds them, and halts:

```asm
LDI 0 10
LDI 1 5
ADD 0 1
HALT
```

### Expected Result

After execution:

- `R0 = 15`
- `R1 = 5`
- The CPU halts after executing `HALT`.

## How to Build and Run

The assembler and CPU emulator are independent C programs. Both can be compiled and run like normal C programs using your preferred IDE.

### 1. Run the Assembler

Keep `program.txt` in the folder from which the assembler runs.

The assembler reads `program.txt`, validates the instructions and operands, and generates `program.bin` after confirmation.

### 2. Run the CPU Emulator

Keep `program.bin` in the folder from which the CPU emulator runs.

The CPU emulator loads `program.bin` into its memory and executes the machine-code instructions. After execution, it prints the final register values, flags, and program counter.

**Important:**
- The assembler requires `program.txt` to be available in its working directory.
- The CPU emulator requires `program.bin` to be available in its working directory.
- `program.bin` is generated by the assembler and is the input used by the CPU emulator.

Both programs can be compiled and run separately, but the CPU emulator needs a valid binary generated by the assembler.

## Learning Objectives

This project explores:

- Computer architecture and CPU fundamentals.
- Instruction set design and opcode encoding.
- Registers and memory addressing.
- Arithmetic and bitwise operations.
- Status flags and comparisons.
- Conditional and unconditional branching.
- The fetch-decode-execute cycle.
- C programming, arrays, enums, and bit manipulation.
- Text parsing and binary file handling.
- The relationship between assembly language and machine code.

## Current Scope

This is an educational CPU emulator built around a fixed 20-instruction ISA and 256 bytes of memory. It demonstrates basic processor concepts and is not intended to emulate a commercial processor.

## Future Improvements

- Add instruction-by-instruction execution tracing.
- Improve handling of invalid machine code and memory boundaries.
- Expand tests for arithmetic, flags, and branching.
- Improve assembler input validation and error reporting.

## License

No license has been selected yet. A license can be added when the project is ready to specify how others may use, modify, and distribute the code.
