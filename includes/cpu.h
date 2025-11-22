#ifndef CPU_H
#define CPU_H

#include <stdint.h>

// ===== Flags masks (in F register) =====
#define FLAG_Z 0x80    // Zero
#define FLAG_N 0x40    // Subtract
#define FLAG_H 0x20    // Half carry
#define FLAG_C 0x10    // Carry

typedef struct {
    // === Registers ===
    union {
        struct {
            uint8_t F;      // Flags: Z -> Zero, N -> Substract, H -> Half-carry, C -> Carry
            uint8_t A;      // Accumulator
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

    uint16_t SP;   // Stack Pointer
    uint16_t PC;   // Program Counter / Pointer

    // === CPU State ===
    uint8_t ime;       // Interrupt Master Enable (0 or 1)
    uint8_t halted;    // HALT state
    uint8_t stopped;   // STOP state
} CPU;

// === Functions ===

#endif