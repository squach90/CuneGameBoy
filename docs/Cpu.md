# CPU

This page aims to repertory all info on the CPU you need to create an ✨ GameBoy Emulator because why not ¯\_(ツ)_/¯

The CPU of the gameboy (the 8-bit 8080-like Sharp CPU, [Pandocs](https://gbdev.io/pandocs/Specifications.html)) can be split in 4 parts: **Registers, Fetch/Decode/Execute, Instructions, Interruptions**

## Registers

There is **8**, 8-bit registers: `A, F, B, C, D, E, H, L` and **4**, 16-bit registers: `AF, BC, DE, HL` (just a **combination** of all 8-bit registers).

The `A` register is the **Accumulator**.
The `B, C, D, E, H & L` register are just **basics** registers
The `F` register is the **Flag** register who contain flags like:

- Z -> Zero flag
- N -> Substraction flag
- H -> Half Carry flag
- C -> Carry flag

```h
// === Registers ===
union {
    struct {
        uint8_t F;
        uint8_t A;
    };
    uint16_t AF;
};

union {
    struct {
        uint8_t C;
        uint8_t B;
    };
    uint16_t BC;
};

union {
    struct {
        uint8_t E;
        uint8_t D;
    };
    uint16_t DE;
};

union {
    struct {
        uint8_t L;
        uint8_t H;
    };
    uint16_t HL;
};
```

In this code we use `union{}` to create 16 bits registers like the GameBoy, and with this we don't need to do some complicated operation like just to get HL (by example) :

```c
H = (value >> 8) & 0xFF;
L = value & 0xFF;
```

### The [Zero](https://gbdev.io/pandocs/CPU_Registers_and_Flags.html#the-zero-flag-z) Flag `0x80`

This bit is set if and only if the result of an operation is zero.

### The [Carry](https://gbdev.io/pandocs/CPU_Registers_and_Flags.html#the-carry-flag-c-or-cy) Flag `0x10`

Is set in this case :

- When the result of an 8-bit addition is higher than $FF.
- When the result of a 16-bit addition is higher than $FFFF.
- When the result of a subtraction or comparison is lower than zero.
- When a rotate/shift operation shifts out a "1" bit.

### [The N & H Flags](https://gbdev.io/pandocs/CPU_Registers_and_Flags.html#the-bcd-flags-n-h) `0x40` `0x20`

TODO: Explain

### SP & PC

#### **PC - Program Counter**

No, PC is not for *Personal Computer* (˶ᵔ ᵕ ᵔ˶) but for **Program Counter**. It points to the **next instruction** to execute.

The CPU increments it automatically every time it fetches an opcode, like :

```asm
LD A, $42   ; PC = 0x0000 → 0x0002
LD B, $10   ; PC = 0x0002 → 0x0004
NOP         ; PC = 0x0004 → 0x0005
```

After each instruction, the PC moves forward by the size of the instruction (1, 2 or 3 bytes)

Some instruction can modify the PC directly, like :

```asm
JP $2000   ; PC is now 0x2000
```

#### **SP - Stack Pointer**

The **SP (Stack Pointer)** is a 16-bit register that points to the **top of the stack**.

On the Game Boy the SP grown **downwards**.

- **PUSH X** → SP decreases by 2, then the value is stored on the stack.
- **POP X** → The value is read from the stack, then SP increases by 2.

```md
Top adress (ex: $C1FF)
    ↓
    +-------------------+
    |                   |  ← SP
    +-------------------+
    |    Element 3      |  ← Last In (LI)
    +-------------------+
    |    Element 2      |
    +-------------------+
    |    Element 1      |  ← First In (FI)
    +-------------------+
    |                   |
    |   Espace libre    |
    |                   |
    +-------------------+
    ↑
Low adress (ex: $C100)
```

The SP in the GameBoy is **LIFO** *Last In First Out*, the in the example the last is Element 3 so it will be the first if we do a `POP`.

## Fetch/Decode/Execute

The GameBoy CPU has **256 normal opcodes** plus **256 CB-prefixed opcodes**: [GameBoy Opcodes Table](https://meganesu.github.io/generate-gb-opcodes/)
The CPU will fetch the opcode who is at the PC adress, and execute this one.
