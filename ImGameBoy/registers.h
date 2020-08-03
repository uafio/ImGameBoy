#pragma once
#include <stdint.h>

struct Registers {
    union {
        struct {
            union {
                struct {
                    uint8_t : 4, C : 1, H : 1, N : 1, Z : 1;
                } Flag;
                uint8_t F;
            };
            uint8_t A;
        };
        uint16_t AF;
    };

    union {
        struct {
            uint8_t C;
            uint8_t B;
        };
        uint16_t BC;
    };

    union {
        struct {
            uint8_t E;
            uint8_t D;
        };
        uint16_t DE;
    };

    union {
        struct {
            uint8_t L;
            uint8_t H;
        };
        uint16_t HL;
    };

    uint16_t SP;
    uint16_t PC;

    Registers( void )
        : AF( 0 ), BC( 0 ), DE( 0 ), HL( 0 ), SP( 0 ), PC( 0 )
    {
    }
};