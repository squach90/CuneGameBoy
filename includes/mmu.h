#ifndef MMU_H
#define MMU_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint8_t *rom;
    size_t rom_size;

    uint8_t rom_bank_low;   // bank number (1..)
    uint8_t ram_enabled;

    // External RAM (cartridge) - max 32KB depend on which cartridge ; we alloc 0x8000 per security
    uint8_t *eram;
    size_t eram_size;

    // Internal memory areas
    uint8_t vram[0x2000];   // 0x8000-0x9FFF
    uint8_t wram[0x2000];   // 0xC000-0xDFFF
    uint8_t oam[0xA0];      // 0xFE00-0xFE9F
    uint8_t io[0x80];       // 0xFF00-0xFF7F
    uint8_t hram[0x7F];     // 0xFF80-0xFFFE
    uint8_t interrupt_enable; // 0xFFFF

    uint8_t bios_active;   // 1 = BIOS enabled, 0 = disabled
    uint8_t bios[0x100];   // 256-byte boot ROM


    // simple MBC type detection (0 = no MBC, 1 = MBC1-like)
    uint8_t mbc_type;
} MMU;

// == Function ==
void mmu_init(MMU *mmu);
int mmu_load_bios_file(MMU *mmu, const char *filename);
int mmu_load_rom(MMU *mmu, const uint8_t *data, size_t size);
void mmu_free_rom(MMU *mmu);
uint8_t mmu_read(MMU *mmu, uint16_t addr);
void mmu_write(MMU *mmu, uint16_t addr, uint8_t val);

#endif