#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include "mmu.h"

// ===== Flags masks (in F register) =====
#define FLAG_Z 0x80    // Zero
#define FLAG_N 0x40    // Subtract
#define FLAG_H 0x20    // Half carry
#define FLAG_C 0x10    // Carry

typedef struct {
    // === Registers ===
    union {
        struct {
            uint8_t A;      // Accumulator
            uint8_t F;      // Flags: Z -> Zero, N -> Substract, H -> Half-carry, C -> Carry
        };
        uint16_t AF;
    };

    union {
        struct {
            uint8_t B;
            uint8_t C;
        };
        uint16_t BC;
    };

    union {
        struct {
            uint8_t D;
            uint8_t E;
        };
        uint16_t DE;
    };

    union {
        struct {
            uint8_t H;
            uint8_t L;
        };
        uint16_t HL;
    };

    uint16_t SP;   // Stack Pointer
    uint16_t PC;   // Program Counter / Pointer

    // === CPU State ===
    uint8_t ime;       // Interrupt Master Enable (0 or 1)
    uint8_t halted;    // HALT state
    uint8_t stopped;   // STOP state
} CPU;

// === Functions ===
void cpu_init(CPU *cpu);
uint16_t cpu_step(CPU *cpu, MMU *mmu);

#endif