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

void cpu_execute_cb(CPU *cpu, uint8_t op) {
    switch(op) {

        case 0x11: { // RL C
            uint8_t carry_in = (cpu->F & FLAG_C) ? 1 : 0;
            uint8_t new_carry = (cpu->C & 0x80) ? 1 : 0;

            cpu->C = (cpu->C << 1) | carry_in;

            // Update flags
            cpu->F &= 0x1F; // Clear Z N H C
            if (cpu->C == 0) {
                cpu->F |= FLAG_Z; // Set Z flag
            }
            if (new_carry) {
                cpu->F |= FLAG_C; // Set C flag
            }

            if (DEBUG_MODE >= 2) {
                printf("CB RL C -> C=0x%02X, F=0x%02X\n", cpu->C, cpu->F);
            }
            break;
        }

        case 0x7C: { // BIT 7, H
            uint8_t bit = (cpu->H >> 7) & 1;

            cpu->F &= ~(FLAG_Z | FLAG_N);
            cpu->F |= FLAG_H; // H flag always set

            if (bit == 0) cpu->F |= FLAG_Z;
            if (DEBUG_MODE >= 2) {
                printf("CB BIT 7, H -> H=0x%02X, F=0x%02X\n", cpu->H, cpu->F);
            }
            break;
        }

        default:
            printf("Unknown CB opcode: 0x%02X\n", op);
            exit(1);
            break;
    }
}

void cpu_step(CPU *cpu, MMU *mmu) {
    uint8_t opcode = mmu_read(mmu, cpu->PC++); // - Fetch opcode -
    switch (opcode) {
        case 0x00: // NOP
            break;

        case 0x04: { // INC B
            uint8_t old = cpu->B;

            cpu->B += 1;

            // Clear Z, N, H but keep C flag
            cpu->F &= 0x10;

            // Set Z flag
            if (cpu->B == 0)
                cpu->F |= FLAG_Z;

            // H flag is set if lower nibble overflowed
            if ((old & 0x0F) == 0x0F)
                cpu->F |= FLAG_H;

            if (DEBUG_MODE >= 2) {
                printf("INC B -> B=0x%02X, F=0x%02X\n", cpu->B, cpu->F);
            }
            break;
        }

        case 0x05: { // DEC B
            uint8_t old = cpu->B;
            cpu->B--;

            cpu->F &= FLAG_C;
            cpu->F |= FLAG_N;
            if(cpu->B == 0)
                cpu->F |= FLAG_Z;
            if((old & 0x0F) == 0x00)
                cpu->F |= FLAG_H;
            if (DEBUG_MODE >= 2) {
                printf("DEC B -> B=0x%02X, F=0x%02X\n", cpu->B, cpu->F);
            }
            break;
        }

        case 0x06: { // LD B, d8
            cpu->B = mmu_read(mmu, cpu->PC++);

            if (DEBUG_MODE >= 2) {
                printf("LD B, 0x%02X\n", cpu->B);
            }
            break;
        }

        case 0x08: { // LD (a16), SP
            uint8_t low  = mmu_read(mmu, cpu->PC++);
            uint8_t high = mmu_read(mmu, cpu->PC++);
            uint16_t addr = (high << 8) | low;

            mmu_write(mmu, addr + 1, (cpu->SP >> 8) & 0xFF); // High byte
            mmu_write(mmu, addr, cpu->SP & 0xFF);        // Low byte

            if (DEBUG_MODE >= 2) {
                printf("LD (0x%04X), SP -> [0x%04X]=0x%04X\n", addr, addr, cpu->SP);
            }
            break;
        }

        case 0x0C: { // INC C
            uint8_t old = cpu->C;

            cpu->C += 1;

            // Clear Z, N, H but keep C flag
            cpu->F &= 0x10;

            // Set Z flag
            if (cpu->C == 0)
                cpu->F |= FLAG_Z;

            // H flag is set if lower nibble overflowed
            if ((old & 0x0F) == 0x0F)
                cpu->F |= FLAG_H;

            if (DEBUG_MODE >= 2) {
                printf("INC C -> C=0x%02X, F=0x%02X\n", cpu->C, cpu->F);
            }
            break;
        }

        case 0x0D: { // DEC C
            uint8_t old = cpu->C;
            cpu->C--;

            cpu->F &= FLAG_C;
            cpu->F |= FLAG_N;
            if(cpu->C == 0)
                cpu->F |= FLAG_Z;
            if((old & 0x0F) == 0x00)
                cpu->F |= FLAG_H;
            if (DEBUG_MODE >= 2) {
                printf("DEC C -> C=0x%02X, F=0x%02X\n", cpu->C, cpu->F);
            }
            break;
        }

        case 0x0E: { // LD C, d8
            cpu->C = mmu_read(mmu, cpu->PC++);

            if (DEBUG_MODE >= 2) {
                printf("LD C, 0x%02X\n", cpu->C);
            }
            break;
        }

        case 0x11: { // LD DE, d16
            uint8_t low  = mmu_read(mmu, cpu->PC++);
            uint8_t high = mmu_read(mmu, cpu->PC++);
            cpu->DE = (high << 8) | low;

            if (DEBUG_MODE >= 2) {
                printf("LD DE, 0x%04X\n", cpu->DE);
            }
            break;
        }

        case 0x13: { // INC DE
            cpu->DE++;
            if (DEBUG_MODE >= 2) {
                printf("INC DE -> DE=0x%04X\n", cpu->DE);
            }
            break;
        }

        case 0x17: { // RLA
            uint8_t carry_in = (cpu->F & FLAG_C) ? 1 : 0;
            uint8_t new_carry = (cpu->A & 0x80) ? 1 : 0;

            cpu->A = (cpu->A << 1) | carry_in;

            // Update flags
            cpu->F &= 0x1F; // Clear Z N H C
            if (new_carry) {
                cpu->F |= FLAG_C; // Set C flag
            }

            if (DEBUG_MODE >= 2) {
                printf("RLA -> A=0x%02X, F=0x%02X\n", cpu->A, cpu->F);
            }
            break;
        }

        case 0x18: { // JR r8
            int8_t offset = (int8_t) mmu_read(mmu, cpu->PC++);
            cpu->PC += offset;

            if (DEBUG_MODE >= 2) {
                printf("JR %d -> PC=0x%04X\n", offset, cpu->PC);
            }
            break;
        }

        case 0x1A: {
            // LD A, (DE)
            cpu->A = mmu_read(mmu, cpu->DE);

            if (DEBUG_MODE >= 2) {
                printf("LD A, (DE) -> A=0x%02X from [0x%04X]\n", cpu->A, cpu->DE);
            }
            break;
        }

        case 0x1E: { // LD E, d8
            cpu->E = mmu_read(mmu, cpu->PC++);

            if (DEBUG_MODE >= 2) {
                printf("LD E, 0x%02X\n", cpu->E);
            }
            break;
        }

        case 0x20: { // JR NZ, r8
            int8_t offset = (int8_t) mmu_read(mmu, cpu->PC++);
            if (!(cpu->F & FLAG_Z)) { // if Z == 0
                cpu->PC += offset;
            }

            if (DEBUG_MODE >= 2) {
                printf("JR NZ, %d -> PC=0x%04X\n", offset, cpu->PC);
            }
            break;
        }

        case 0x21: { // LD HL, d16
            uint8_t low  = mmu_read(mmu, cpu->PC++);
            uint8_t high = mmu_read(mmu, cpu->PC++);
            cpu->HL = (high << 8) | low;

            if(DEBUG_MODE >= 2) {
                printf("LD HL, 0x%04X\n", cpu->HL);
            }
            break;
        }

        case 0x22: { // LD (HL+), A
            mmu_write(mmu, cpu->HL, cpu->A);
            cpu->HL++;

            if(DEBUG_MODE >= 2) {
                printf("LD (HL+), A -> [0x%04X]=0x%02X, HL=0x%04X\n", cpu->HL + 1, cpu->A, cpu->HL);
            }
            break;
        }

        case 0x23: {
            // INC HL
            cpu->HL++;

            if (DEBUG_MODE >= 2) {
                printf("INC HL -> HL=0x%04X\n", cpu->HL);
            }
            break;
        }

        case 0x28: { // JR Z, r8
            int8_t offset = (int8_t) mmu_read(mmu, cpu->PC++);
            if (cpu->F & FLAG_Z) { // if Z == 1
                cpu->PC += offset;
            }

            if (DEBUG_MODE >= 2) {
                printf("JR Z, %d -> PC=0x%04X\n", offset, cpu->PC);
            }
            break;
        }

        case 0x2E: { // LD L, d8
            cpu->L = mmu_read(mmu, cpu->PC++);

            if (DEBUG_MODE >= 2) {
                printf("LD L, 0x%02X\n", cpu->L);
            }
            break;
        }

        case 0x31: { // LD SP, d16
            uint8_t low  = mmu_read(mmu, cpu->PC++);
            uint8_t high = mmu_read(mmu, cpu->PC++);
            cpu->SP = (high << 8) | low;

            if(DEBUG_MODE >= 2) {
                printf("LD SP, 0x%04X\n", cpu->SP);
            }
            break;
        }

        case 0x32: { // LD (HL-), A
            mmu_write(mmu, cpu->HL, cpu->A);
            cpu->HL--;

            if(DEBUG_MODE >= 2) {
                printf("LD (HL-), A -> [0x%04X]=0x%02X, HL=0x%04X\n", cpu->HL + 1, cpu->A, cpu->HL);
            }
            break;
        }

        case 0x3D: { // DEC A
            uint8_t old = cpu->A;
            cpu->A--;

            cpu->F &= FLAG_C;
            cpu->F |= FLAG_N;
            if(cpu->A == 0)
                cpu->F |= FLAG_Z;
            if((old & 0x0F) == 0x00)
                cpu->F |= FLAG_H;
            if (DEBUG_MODE >= 2) {
                printf("DEC A -> A=0x%02X, F=0x%02X\n", cpu->A, cpu->F);
            }
            break;
        }

        case 0x3E: { // LD A, d8
            cpu->A = mmu_read(mmu, cpu->PC++);

            if (DEBUG_MODE >= 2) {
                printf("LD A, 0x%02X\n", cpu->A);
            }
            break;
        }

        case 0x4F: { // LD C, A
            cpu->C = cpu->A;
            if (DEBUG_MODE >= 2) {
                printf("LD C, A -> C=0x%02X\n", cpu->C);
            }
            break;
        }

        case 0x57: { // LD D, A
            cpu->D = cpu->A;

            if(DEBUG_MODE >= 2) {
                printf("LD D, A -> D=0x%02X\n", cpu->D);
            }
            break;
        }

        case 0x67: { // LD H, A
            cpu->H = cpu->A;

            if(DEBUG_MODE >= 2) {
                printf("LD H, A -> H=0x%02X\n", cpu->H);
            }
            break;
        }

        case 0x77: { // LD (HL), A
            mmu_write(mmu, cpu->HL, cpu->A);

            if(DEBUG_MODE >= 2) {
                printf("LD (HL), A -> [0x%04X]=0x%02X\n", cpu->HL, cpu->A);
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

        case 0xC1: { // POP BC
            uint8_t low  = mmu_read(mmu, cpu->SP++);
            uint8_t high = mmu_read(mmu, cpu->SP++);
            cpu->BC = (high << 8) | low;

            if (DEBUG_MODE >= 2) {
                printf("POP BC -> BC=0x%04X, SP=0x%04X\n", cpu->BC, cpu->SP);
            }
            break;
        }

        case 0xC5: { // PUSH BC
            cpu->SP -= 2;
            mmu_write(mmu, cpu->SP, (cpu->BC >> 8) & 0xFF); // High byte
            mmu_write(mmu, cpu->SP + 1, cpu->BC & 0xFF);    // Low byte

            if (DEBUG_MODE >= 2) {
                printf("PUSH BC -> BC=0x%04X, SP=0x%04X\n", cpu->BC, cpu->SP);
            }
            break;
        }

        case 0xC9: { // RET
            uint8_t low  = mmu_read(mmu, cpu->SP++);
            uint8_t high = mmu_read(mmu, cpu->SP++);
            cpu->PC = (high << 8) | low;

            if (DEBUG_MODE >= 2) {
                printf("RET -> PC=0x%04X, SP=0x%04X\n", cpu->PC, cpu->SP);
            }
            break;
        }

        case 0xCB: { // Prefix CB
            if (DEBUG_MODE >= 2){
                printf("Switched to CB\n");
            }
            cpu_execute_cb(cpu, mmu_read(mmu, cpu->PC++));
            break;
        }

        case 0xCD: { // CALL a16
            uint8_t low  = mmu_read(mmu, cpu->PC++);
            uint8_t high = mmu_read(mmu, cpu->PC++);
            uint16_t addr = (high << 8) | low;

            // Push return address (PC) on stack
            cpu->SP -= 2;
            mmu_write(mmu, cpu->SP, (cpu->PC >> 8) & 0xFF); // High byte
            mmu_write(mmu, cpu->SP + 1, cpu->PC & 0xFF);    // Low byte

            cpu->PC = addr;

            if (DEBUG_MODE >= 2) {
                printf("CALL 0x%04X -> PC=0x%04X, SP=0x%04X\n",
                    addr, cpu->PC, cpu->SP);
            }
            break;
        }


        case 0xE0: { // LDH (a8), A
            uint8_t addr = mmu_read(mmu, cpu->PC++);
            uint16_t full_addr = 0xFF00 + addr;
            mmu_write(mmu, full_addr, cpu->A);

            if (DEBUG_MODE >= 2) {
                printf("LDH (0x%02X), A -> [0x%04X]=0x%02X\n", addr, full_addr, cpu->A);
            }
            break;
        }

        case 0xE2: { // LD (0xFF00 + C), A
            uint16_t addr = 0xFF00 + cpu->C;
            mmu_write(mmu, addr, cpu->A);

            if (DEBUG_MODE >= 2) {
                printf("LD (0xFF00 + C), A -> [0x%04X]=0x%02X\n", addr, cpu->A);
            }
            break;
        }

        case 0xEA: { // LD (a16), A
            uint8_t low  = mmu_read(mmu, cpu->PC++);
            uint8_t high = mmu_read(mmu, cpu->PC++);
            uint16_t addr = (high << 8) | low;

            mmu_write(mmu, addr, cpu->A);

            if (DEBUG_MODE >= 2) {
                printf("LD (0x%04X), A -> [0x%04X]=0x%02X\n", addr, addr, cpu->A);
            }
            break;
        }

        case 0xF0: { // LDH A, (a8)
            uint8_t addr = mmu_read(mmu, cpu->PC++);
            uint16_t full_addr = 0xFF00 + addr;
            cpu->A = mmu_read(mmu, full_addr);

            if (DEBUG_MODE >= 2) {
                printf("LDH A, (0x%02X) -> A=0x%02X from [0x%04X]\n", addr, cpu->A, full_addr);
            }
            break;
        }

        case 0xFE: { // CP d8
            uint8_t value = mmu_read(mmu, cpu->PC++);
            uint8_t result = cpu->A - value;

            // Set flags
            cpu->F &= 0x10; // Clear Z H C but keep N
            cpu->F |= FLAG_N; // Set N flag

            if (result == 0) {
                cpu->F |= FLAG_Z; // Set Z flag
            }
            if (cpu->A < value) {
                cpu->F |= FLAG_C; // Set C flag
            }
            if ((cpu->A & 0x0F) < (value & 0x0F)) {
                cpu->F |= FLAG_H; // Set H flag
            }

            if (DEBUG_MODE >= 2) {
                printf("CP 0x%02X -> A=0x%02X, F=0x%02X\n", value, cpu->A, cpu->F);
            }
            break;
        }

        case 0xFF: { // RST 38H
            cpu->SP -= 2;
            mmu_write(mmu, cpu->SP, cpu->PC & 0xFF);            // Low byte
            mmu_write(mmu, cpu->SP + 1, (cpu->PC >> 8) & 0xFF); // High byte

            cpu->PC = 0x0038;

            if (DEBUG_MODE >= 2) {
                printf("RST 38H -> PC=0x%04X, SP=0x%04X\n", cpu->PC, cpu->SP);
            }
            break;
        }

        default:
            if (DEBUG_MODE >= 2) {
                printf("Unknown opcode: 0x%02X at PC=0x%04X\n", opcode, cpu->PC - 1);
            }
            exit(1);
            break;
    }
}