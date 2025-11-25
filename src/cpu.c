#include <stdio.h>
#include <stdlib.h>
#include "../includes/cpu.h"
#include "../includes/mmu.h"
#include "../includes/main.h"

void cpu_init(CPU *cpu) {
    // == Flags ==
    cpu->A = 0;
    cpu->F = 0;
    cpu->B = 0;
    cpu->C = 0;
    cpu->D = 0;
    cpu->E = 0;
    cpu->H = 0;
    cpu->L = 0;

    // == Pointers ==
    cpu->PC = 0x0000;
    cpu->SP = 0x0000;

    // == CPU State ==
    cpu->ime = 0;       // Interrupt Master Enable (0 or 1)
    cpu->halted = 0;    // HALT state
    cpu->stopped = 0;   // STOP state
}

void cpu_step(CPU *cpu, MMU *mmu) {
    uint8_t opcode = mmu_read(mmu, cpu->PC++); // - Fetch opcode -
    switch (opcode) {
        case 0x00: // NOP
            break;

        case 0x31: { // LD SP, d16
            uint8_t low  = mmu_read(mmu, cpu->PC++);
            uint8_t high = mmu_read(mmu, cpu->PC++);
            cpu->SP = (high << 8) | low;

            if(DEBUG_MODE >= 2) {
                printf("LD SP, 0x%04X\n", cpu->SP);
            }
            break;
        }

        case 0xAF: // XOR A
            cpu->A ^= cpu->A;
            // Set flags
            cpu->F = FLAG_Z; // Set Zero flag
            if(DEBUG_MODE >= 2) {
                printf("XOR A -> A=0x%02X, F=0x%02X\n", cpu->A, cpu->F);
            }
            break;

        default:
            if (DEBUG_MODE >= 2) {
                printf("Unknown opcode: 0x%02X at PC=0x%04X\n", opcode, cpu->PC - 1);
            }
            exit(1);
            break;
    }
}