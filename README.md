# Simple Ben Eater 8-bit Computer Simulator

This is a small computer simulator written in C++. The program runs one
instruction at a time, just like a real computer.

The simulator uses:

- a **program counter** called `pc`
- a **carry flag**
- a **zero flag**
- registers (`r1`, `r2`, `r3`, ...) and memory (`m1`, `m2`, `m3`, ...)

`storage.h` is not changed by this version.

## Bit size

At the top of `main-simpler.cpp`:

```cpp
int bit_max = 4;
```

This means the computer can keep only 4 bits. Its numbers go from `0` to
`15`.

If a calculation is too large, the computer keeps the low bits and turns on
the carry flag.

For example, with 4 bits:

```text
14 + 12 = 26
26 becomes 10
carry becomes true
```

This is what a real fixed-width computer does. To simulate an 8-bit computer,
change the value to:

```cpp
int bit_max = 8;
```

Then numbers go from `0` to `255`.

## Writing a program

Programs are written in the `program.asm` file:

```asm
LDI r1 14     ; put 14 in register r1
ADD r1 m1 r1  ; r1 = r1 + m1
LDI m2 10     ; put 10 in memory m2
HLT
```

The first line is line `0`. The computer normally runs downward, one line at
a time.

Lines starting with `;` are comments. Blank lines are also allowed.

## Ben Eater instructions

These are the instructions used by the Ben Eater 8-bit computer.

| Instruction | What it does | Example |
|---|---|---|
| `NOP` | Does nothing and moves to the next line. | `NOP` |
| `LDA memory` | Copies memory into the internal accumulator. | `LDA m1` |
| `ADD memory` | Adds memory to the internal accumulator. | `ADD m1` |
| `SUB memory` | Subtracts memory from the internal accumulator. | `SUB m1` |
| `STA memory` | Copies the internal accumulator into memory. | `STA m1` |
| `LDI place number` | Puts a number into a register or memory cell. | `LDI r1 7` or `LDI m1 7` |
| `JMP line` | Always jumps to a line. | `JMP 0` |
| `JC line` | Jumps only when carry is true. | `JC 0` |
| `JNC line` | Jumps only when carry is false. | `JNC 0` |
| `JZ line` | Jumps only when the accumulator is zero. | `JZ 0` |
| `JNZ line` | Jumps only when the accumulator is not zero. | `JNZ 0` |
| `OUT` | Prints the internal accumulator. | `OUT` |
| `HLT` | Stops the computer. | `HLT` |

### Example: count down and stop

The internal accumulator starts at zero, so load a value from memory first.
This example sets `m2` to `1`, because the default value in `storage.h` is
`12`.

```asm
LDI m2 1    ; m2 = 1
LDA m1      ; accumulator = 12
SUB m2      ; accumulator = accumulator - 1
STA m3      ; save the current answer in m3
OUT         ; print the accumulator
JNZ 2       ; repeat while the accumulator is not zero
HLT         ; stop
```

This example demonstrates the Ben Eater-style instructions. For register and
memory arithmetic, use the three-operand form instead:

```asm
LDI r1 10     ; r1 = 10
LDI r2 1      ; r2 = 1
SUB r1 r2 r1  ; r1 = r1 - r2
JNZ 2         ; repeat while r1 is not zero
HLT
```

`JNC 2` jumps to line 2 only when the last calculation did not overflow.
`JNZ 2` jumps to line 2 only when the last result was not zero.

## Extra instructions

The original simulator instructions are still supported:

```text
ADD r1 r2 r3   -> r3 = r1 + r2
SUB r1 r2 r3   -> r3 = r1 - r2
AND r1 r2 r3   -> r3 = r1 & r2
XOR r1 r2 r3   -> r3 = r1 ^ r2
OR  r1 r2 r3   -> r3 = r1 | r2
RSH r1 r2 r3   -> r3 = r1 >> r2
ADI r3 r2      -> r3 = r3 + r2
LDI r3 7       -> r3 = 7
LDI m1 7       -> m1 = 7
```

These use the names stored in `storage.h`, such as `r1`, `r2`, `r3`, and
`m1`.

## Building

Compile the simulator with C++17. Keep `program.asm` in the same folder as the
executable:

```text
g++ -std=c++17 main-simpler.cpp -o pc_sim
```

Then run it:

```text
pc_sim
```

The simulator looks for `program.asm` beside the executable, so it can also
be started from an IDE or from another folder.

### CMake

```text
cmake -S . -B build
cmake --build build
```
The generated executable is `build/pc_sim` on Linux/macOS or
`build/Debug/pc_sim.exe` with a Visual Studio generator on Windows.
### Usage of ai

Ai was used for most of the comments and for some code,
but most of it i done myself.


