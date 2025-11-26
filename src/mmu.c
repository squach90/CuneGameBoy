#include "../includes/mmu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t detect_mbc_type(const uint8_t *rom, size_t size) {
    if (!rom || size < 0x0148) return 0;
    uint8_t type = rom[0x0147];
    if (type == 0x00) return 0;
    if (type == 0x01 || type == 0x02 || type == 0x03) return 1; // MBC1-like
    return 0;
}

void mmu_init(MMU *mmu) {
    memset(mmu, 0, sizeof(MMU));
    mmu->rom = NULL;
    mmu->eram = NULL;
    mmu->rom_size = 0;
    mmu->eram_size = 0x2000;
    mmu->rom_bank_low = 1;
    mmu->ram_enabled = 0;
    mmu->mbc_type = 0;
    mmu->interrupt_enable = 0;
    mmu->bios_active = 1;
}

int mmu_load_bios_file(MMU *mmu, const char *filename) {
    if (!mmu || !filename) return -1;

    FILE *f = fopen(filename, "rb");
    if (!f) return -1;

    size_t n = fread(mmu->bios, 1, 0x100, f); // read 256 bytes
    fclose(f);

    if (n != 0x100) return -1; // incorrect size
    mmu->bios_active = 1;
    return 0;
}

int mmu_load_bios(MMU *mmu, const uint8_t *bios_data, size_t size) {
    if (size > sizeof(mmu->bios)) return -1;
    memcpy(mmu->bios, bios_data, size);
    mmu->bios_active = 1;
    return 0;
}


int mmu_load_rom(MMU *mmu, const uint8_t *data, size_t size) {
    if (!mmu || !data || size == 0) return -1;
    mmu->rom = malloc(size);
    if (!mmu->rom) return -1;
    memcpy(mmu->rom, data, size);
    mmu->rom_size = size;

    mmu->mbc_type = detect_mbc_type(mmu->rom, mmu->rom_size);

    // allocate external RAM if needed
    mmu->eram = malloc(mmu->eram_size);
    if (!mmu->eram) {
        free(mmu->rom);
        mmu->rom = NULL;
        mmu->rom_size = 0;
        return -1;
    }
    memset(mmu->eram, 0, mmu->eram_size);
    return 0;
}

void mmu_free_rom(MMU *mmu) {
    if (!mmu) return;
    if (mmu->rom) { free(mmu->rom); mmu->rom = NULL; }
    if (mmu->eram) { free(mmu->eram); mmu->eram = NULL; }
    mmu->rom_size = 0;
}

uint8_t mmu_read(MMU *mmu, uint16_t addr) {
    if (!mmu) return 0xFF;

    // BIOS actif
    if (mmu->bios_active && addr < 0x0100) {
        return mmu->bios[addr];
    }

    // ROM bank 0
    if (addr <= 0x3FFF) {
        if (addr < mmu->rom_size) return mmu->rom[addr];
        return 0xFF;
    }

    // ROM bankée
    if (addr <= 0x7FFF) {
        size_t bank = mmu->rom_bank_low;
        size_t offset = (bank * 0x4000) + (addr - 0x4000);
        if (offset < mmu->rom_size) return mmu->rom[offset];
        return 0xFF;
    }

    // VRAM
    if (addr >= 0x8000 && addr <= 0x9FFF)
        return mmu->vram[addr - 0x8000];

    // External RAM
    if (addr >= 0xA000 && addr <= 0xBFFF) {
        if (!mmu->ram_enabled) return 0xFF;
        size_t offset = addr - 0xA000;
        if (offset < mmu->eram_size) return mmu->eram[offset];
        return 0xFF;
    }

    // WRAM
    if (addr >= 0xC000 && addr <= 0xDFFF)
        return mmu->wram[addr - 0xC000];

    // Echo RAM
    if (addr >= 0xE000 && addr <= 0xFDFF)
        return mmu->wram[addr - 0xE000];

    // OAM
    if (addr >= 0xFE00 && addr <= 0xFE9F)
        return mmu->oam[addr - 0xFE00];

    // IO
    if (addr >= 0xFF00 && addr <= 0xFF7F)
        return mmu->io[addr - 0xFF00];

    // HRAM
    if (addr >= 0xFF80 && addr <= 0xFFFE)
        return mmu->hram[addr - 0xFF80];

    // Interrupt Enable
    if (addr == 0xFFFF)
        return mmu->interrupt_enable;

    return 0xFF;
}


void mmu_write(MMU *mmu, uint16_t addr, uint8_t val) {
    if (!mmu) return;

    if (addr <= 0x1FFF) {
        // RAM enable (cartridge)
        if (mmu->mbc_type) {
            mmu->ram_enabled = ((val & 0x0F) == 0x0A) ? 1 : 0;
        }
        // sans MBC on ignore
    } else if (addr >= 0x2000 && addr <= 0x3FFF) {
        // ROM bank number (MBC1-like low bits)
        if (mmu->mbc_type) {
            uint8_t bank = val & 0x1F;
            if (bank == 0) bank = 1;
            mmu->rom_bank_low = bank;
        }
    } else if (addr >= 0x4000 && addr <= 0x5FFF) {
        // pour MBC1: banque haute ou mode - non implémenté dans ce starter
        // TODO: gerer ici
    } else if (addr >= 0x6000 && addr <= 0x7FFF) {
        // mode select pour MBC1 - non implémenté
    } else if (addr >= 0x8000 && addr <= 0x9FFF) {
        mmu->vram[addr - 0x8000] = val;
    } else if (addr >= 0xA000 && addr <= 0xBFFF) {
        if (mmu->ram_enabled) {
            size_t offset = addr - 0xA000;
            if (offset < mmu->eram_size) mmu->eram[offset] = val;
        }
    } else if (addr >= 0xC000 && addr <= 0xDFFF) {
        mmu->wram[addr - 0xC000] = val;
    } else if (addr >= 0xE000 && addr <= 0xFDFF) {
        // Echo RAM mirror
        mmu->wram[addr - 0xE000] = val;
    } else if (addr >= 0xFE00 && addr <= 0xFE9F) {
        mmu->oam[addr - 0xFE00] = val;
    } else if (addr >= 0xFF00 && addr <= 0xFF7F) {
        mmu->io[addr - 0xFF00] = val;
    } else if (addr == 0xFF50) {
        mmu->bios_active = 0; // Disabling BIOS
    } else if (addr >= 0xFF80 && addr <= 0xFFFE) {
        mmu->hram[addr - 0xFF80] = val;
    } else if (addr == 0xFFFF) {
        mmu->interrupt_enable = val;
    }
}
