#include <stdio.h>
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
        default:
            if (DEBUG_MODE == 2 || DEBUG_MODE == 3) {
                printf("Unknown opcode: 0x%02X at PC=0x%04X\n", opcode, cpu->PC - 1);
            }
            break;
    }
}