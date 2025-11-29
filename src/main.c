#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../includes/cpu.h"
#include "../includes/mmu.h"
#include "../includes/ppu.h"
#include "../includes/main.h"
#include "../includes/bios.h"

int DEBUG_MODE = 0;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <rom_file> [--debug N]\n", argv[0]);
        return 1;
    }

    const char *rom_filename = argv[1];

    // Parse debug option
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--debug") == 0) {
            if (i + 1 < argc) {
                DEBUG_MODE = atoi(argv[i + 1]);
                i++;
            } else {
                DEBUG_MODE = 3;
            }
        }
    }

    CPU cpu;
    MMU mmu;
    PPU ppu;

    // Init CPU, MMU & PPU
    cpu_init(&cpu);
    mmu_init(&mmu);
    ppu_init(&ppu);

    // Load BIOS from array
    if (mmu.bios_active == 1) {
        if (mmu_load_bios(&mmu, biosArray, bios_size) != 0) {
            printf("Erreur: impossible de charger le BIOS depuis l'array\n");
            return 1;
        } else if (DEBUG_MODE >= 1) {
            printf("✅ BIOS chargé depuis l'array\n");
        }
    }

    // Load ROM file
    FILE *f = fopen(rom_filename, "rb");
    if (!f) {
        printf("Erreur: impossible d'ouvrir le fichier ROM '%s'\n", rom_filename);
        return 1;
    }

    fseek(f, 0, SEEK_END);
    size_t rom_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    uint8_t *rom_data = malloc(rom_size);
    if (!rom_data) {
        printf("Erreur: impossible d'allouer de la mémoire pour la ROM\n");
        fclose(f);
        return 1;
    }

    fread(rom_data, 1, rom_size, f);
    fclose(f);

    if (mmu_load_rom(&mmu, rom_data, rom_size) != 0) {
        printf("Erreur: impossible de charger la ROM\n");
        free(rom_data);
        return 1;
    }
    free(rom_data);

    if (DEBUG_MODE >= 1) {
        printf("ROM '%s' chargée (%zu bytes)\n", rom_filename, rom_size);
    }

    // Main CPU loop
    while (1) {
        uint16_t cycles = cpu_step(&cpu, &mmu);
    }

    return 0;
}
