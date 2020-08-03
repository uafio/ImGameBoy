#pragma once
#include <Windows.h>
#include <stdint.h>

struct Memory
{
    union {
        uint8_t map[0x10000];               // full memory map

        struct {
        
            union {
                uint8_t rom[0x8000];        // ROM from cartridge
                uint8_t bank[2][0x4000];    // Bank 00-01
            };

            uint8_t vram[0x2000];           // Video RAM
            uint8_t eram[0x2000];           // External RAM

            union {
                uint8_t wram[0x2000];       // Work RAM
                uint8_t wbank[2][0x1000];   // Work RAM Bank 0-1
            };

            uint8_t echo[0x1e00];           // Mirror of C000~DDFF (ECHO RAM)	Typically not used
            uint8_t sat[0xa0];              // Sprite Attribute Table
            uint8_t NA[0x60];               // Not Usable
            uint8_t io[0x80];               // I/O Registers
            uint8_t hram[0x7f];             // High RAM
            uint8_t ie;                     // Interrupt Enable Register
        };

    };

    Memory( void )
        : map{ 0 }
    {
    }
};

static_assert( sizeof( Memory ) == 0x10000, "Memory not 32kB" );
static_assert( offsetof( Memory, map ) == 0, "Invalid memory map start" );
static_assert( offsetof( Memory, rom ) == 0, "ROM Bank 00" );
static_assert( offsetof( Memory, bank[1] ) == 0x4000, "ROM Bank 01" );
static_assert( offsetof( Memory, vram ) == 0x8000, "VRAM" );
static_assert( offsetof( Memory, eram ) == 0xA000, "External RAM" );
static_assert( offsetof( Memory, wram ) == 0xC000, "Work RAM bank 0" );
static_assert( offsetof( Memory, wbank[1] ) == 0xD000, "Work RAM bank 1" );
static_assert( offsetof( Memory, echo ) == 0xE000, "ECHO RAM" );
static_assert( offsetof( Memory, sat ) == 0xFE00, "Sprite Attribute Table" );
static_assert( offsetof( Memory, NA ) == 0xFEA0, "Not Usable" );
static_assert( offsetof( Memory, io ) == 0xFF00, "I/O Registers" );
static_assert( offsetof( Memory, hram ) == 0xFF80, "High RAM" );
static_assert( offsetof( Memory, ie ) == 0xFFFF, "Interrupt Enable Register" );


