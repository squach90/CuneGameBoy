#ifndef PPU_H
#define PPU_H

#include <stdint.h>
#include "mmu.h"

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144

typedef struct {

    // ===== Registers (hardware mapped) =====
    uint8_t LCDC;    // FF40 - LCD Control
    uint8_t STAT;    // FF41 - LCD Status
    uint8_t SCY;     // FF42 - Scroll Y
    uint8_t SCX;     // FF43 - Scroll X
    uint8_t LY;      // FF44 - Current scanline
    uint8_t LYC;     // FF45 - LY Compare
    uint8_t BGP;     // FF47 - BG Palette
    uint8_t OBP0;    // FF48 - Sprite Palette 0
    uint8_t OBP1;    // FF49 - Sprite Palette 1
    uint8_t WY;      // FF4A - Window Y
    uint8_t WX;      // FF4B - Window X

    // ===== Internal state =====
    uint16_t modeClock;   // mode timing counter
    uint8_t mode;         // current PPU mode (0-3)
    uint8_t spriteHeight; // 8 or 16
    uint8_t frameComplete;

    // ===== Framebuffer =====
    // Each pixel = uint8_t (0..3 after palette mapping)
    uint8_t framebuffer[SCREEN_HEIGHT][SCREEN_WIDTH];

} PPU;

// === Functions ===
void ppu_init(PPU *ppu);

#endif