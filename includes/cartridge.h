#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <stdint.h>
#include <stddef.h>

// ===== Cartridge Types =====
typedef enum {
    MBC_NONE = 0,
    MBC1,
    MBC2,
    MBC3,
    MBC5,
    MBC_UNKNOWN
} MBCType;

// ===== Cartridge struct =====
typedef struct {

    // ===== ROM image (from file) =====
    uint8_t* rom;
    size_t romSize;       // in bytes

    // ===== External RAM (if cartridge has RAM) =====
    uint8_t* ram;
    size_t ramSize;

    // ===== MBC type (from header) =====
    MBCType mbcType;

    // ===== Banking =====
    uint8_t romBank;      // current ROM bank (1..n)
    uint8_t ramBank;      // current RAM bank
    uint8_t ramEnabled;   // 0/1

    // ===== MBC1 specific =====
    uint8_t mbc1Mode;     // 0 = 16Mbit ROM mode, 1 = RAM mode

    // ===== Real-Time Clock for MBC3 =====
    struct {
        uint8_t seconds;
        uint8_t minutes;
        uint8_t hours;
        uint8_t dayLow;
        uint8_t dayHigh;
        uint8_t latch; // latched state
    } rtc;

} Cartridge;

// === Functions ===

#endif
