#include <string.h>
#include <stdio.h>
#include "../includes/ppu.h"
#include "../includes/mmu.h"

void ppu_init(PPU *ppu) {
    memset(ppu, 0, sizeof(PPU));

    ppu->LCDC = 0x91; // Typical initial value
    ppu->STAT = 0x00;
    ppu->SCY = 0x00;
    ppu->SCX = 0x00;
    ppu->LY = 0x00;
    ppu->LYC = 0x00;
    ppu->BGP = 0xFC;  // Typical initial value
    ppu->OBP0 = 0xFF; // Typical initial value
    ppu->OBP1 = 0xFF; // Typical initial value
    ppu->WY = 0x00;
    ppu->WX = 0x00;

    ppu->modeClock = 0;
    ppu->mode = 2; // Starting in OAM search mode
    ppu->spriteHeight = 8; // Default sprite height
    ppu->frameComplete = 0;

    // Clear framebuffer
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            ppu->framebuffer[y][x] = 0;
        }
    }
}