#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../includes/cpu.h"
#include "../includes/mmu.h"
#include "../includes/main.h"

int DEBUG_MODE = 0;

int main(int argc, char *argv[]) {
    for(int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "--debug") == 0 && i+1 < argc) {
            DEBUG_MODE = atoi(argv[i+1]); // next argument
            i++;
        } else if(strcmp(argv[i], "--debug") == 0) {
            DEBUG_MODE = 3;
        }
    }

    CPU cpu;
    MMU mmu;

    mmu_init(&mmu);

    if (mmu_load_bios_file(&mmu, "roms/Gameboy-Bios.gb") != 0) {
        printf("Erreur: impossible de charger le BIOS\n");
        return 1;
    } else {
        if (DEBUG_MODE == 1 || DEBUG_MODE == 3) {
            printf("BIOS chargé avec succès\n");
        }
    }

    cpu_init(&cpu);

    while (1) {
        cpu_step(&cpu, &mmu);
    }

}