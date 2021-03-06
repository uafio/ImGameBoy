#pragma once
#include "registers.h"
#include "memory.h"
#include "debugger.h"
#include "instruction.h"
#include "shared.h"

uint8_t bootrom[] = {
    0x31,
    0xFE,
    0xFF,
    0xAF,
    0x21,
    0xFF,
    0x9F,
    0x32,
    0xCB,
    0x7C,
    0x20,
    0xFB,
    0x21,
    0x26,
    0xFF,
    0x0E,
    0x11,
    0x3E,
    0x80,
    0x32,
    0xE2,
    0x0C,
    0x3E,
    0xF3,
    0xE2,
    0x32,
    0x3E,
    0x77,
    0x77,
    0x3E,
    0xFC,
    0xE0,
    0x47,
    0x11,
    0x04,
    0x01,
    0x21,
    0x10,
    0x80,
    0x1A,
    0xCD,
    0x95,
    0x00,
    0xCD,
    0x96,
    0x00,
    0x13,
    0x7B,
    0xFE,
    0x34,
    0x20,
    0xF3,
    0x11,
    0xD8,
    0x00,
    0x06,
    0x08,
    0x1A,
    0x13,
    0x22,
    0x23,
    0x05,
    0x20,
    0xF9,
    0x3E,
    0x19,
    0xEA,
    0x10,
    0x99,
    0x21,
    0x2F,
    0x99,
    0x0E,
    0x0C,
    0x3D,
    0x28,
    0x08,
    0x32,
    0x0D,
    0x20,
    0xF9,
    0x2E,
    0x0F,
    0x18,
    0xF3,
    0x67,
    0x3E,
    0x64,
    0x57,
    0xE0,
    0x42,
    0x3E,
    0x91,
    0xE0,
    0x40,
    0x04,
    0x1E,
    0x02,
    0x0E,
    0x0C,
    0xF0,
    0x44,
    0xFE,
    0x90,
    0x20,
    0xFA,
    0x0D,
    0x20,
    0xF7,
    0x1D,
    0x20,
    0xF2,
    0x0E,
    0x13,
    0x24,
    0x7C,
    0x1E,
    0x83,
    0xFE,
    0x62,
    0x28,
    0x06,
    0x1E,
    0xC1,
    0xFE,
    0x64,
    0x20,
    0x06,
    0x7B,
    0xE2,
    0x0C,
    0x3E,
    0x87,
    0xE2,
    0xF0,
    0x42,
    0x90,
    0xE0,
    0x42,
    0x15,
    0x20,
    0xD2,
    0x05,
    0x20,
    0x4F,
    0x16,
    0x20,
    0x18,
    0xCB,
    0x4F,
    0x06,
    0x04,
    0xC5,
    0xCB,
    0x11,
    0x17,
    0xC1,
    0xCB,
    0x11,
    0x17,
    0x05,
    0x20,
    0xF5,
    0x22,
    0x23,
    0x22,
    0x23,
    0xC9,
    0xCE,
    0xED,
    0x66,
    0x66,
    0xCC,
    0x0D,
    0x00,
    0x0B,
    0x03,
    0x73,
    0x00,
    0x83,
    0x00,
    0x0C,
    0x00,
    0x0D,
    0x00,
    0x08,
    0x11,
    0x1F,
    0x88,
    0x89,
    0x00,
    0x0E,
    0xDC,
    0xCC,
    0x6E,
    0xE6,
    0xDD,
    0xDD,
    0xD9,
    0x99,
    0xBB,
    0xBB,
    0x67,
    0x63,
    0x6E,
    0x0E,
    0xEC,
    0xCC,
    0xDD,
    0xDC,
    0x99,
    0x9F,
    0xBB,
    0xB9,
    0x33,
    0x3E,
    0x3C,
    0x42,
    0xB9,
    0xA5,
    0xB9,
    0xA5,
    0x42,
    0x3C,
    0x21,
    0x04,
    0x01,
    0x11,
    0xA8,
    0x00,
    0x1A,
    0x13,
    0xBE,
    0x20,
    0xFE,
    0x23,
    0x7D,
    0xFE,
    0x34,
    0x20,
    0xF5,
    0x06,
    0x19,
    0x78,
    0x86,
    0x23,
    0x05,
    0x20,
    0xFB,
    0x86,
    0x20,
    0xFE,
    0x3E,
    0x01,
    0xE0,
    0x50
};

class Emulator
{
private:
    Registers r;
    Memory mem;
    Instruction* opcode[0x100];
    Debugger dbg;

public:
    Emulator( void )
    {
        memcpy( mem.map, bootrom, sizeof( bootrom ) );

        opcode[0x00] = new InstructionNop();
        opcode[0x01] = new InstructionLdBCu16();
        opcode[0x02] = new InstructionLdBCA();
        opcode[0x03] = new InstructionIncBC();
        opcode[0x04] = new InstructionIncB();
        opcode[0x05] = new InstructionDecB();
        opcode[0x06] = new InstructionLdB();
        opcode[0x07] = new InstructionRLCA();
        opcode[0x08] = new InstructionLdn16SP();
        opcode[0x09] = new InstructionAddHLBC();
        opcode[0x0A] = new InstructionLdABC();
        opcode[0x0B] = new InstructionDecBC();
        opcode[0x0C] = new InstructionIncC();
        opcode[0x0D] = new InstructionDecC();
        opcode[0x0E] = new InstructionLdC();
        opcode[0x0F] = new InstructionRRCA();

        opcode[0x10] = new InstructionStop( dbg.sstate );
        opcode[0x11] = new InstructionLdDEu16();
        opcode[0x12] = new InstructionLdDEA();
        opcode[0x13] = new InstructionIncDE();
        opcode[0x14] = new InstructionIncD();
        opcode[0x15] = new InstructionDecD();
        opcode[0x16] = new InstructionLdD();
        opcode[0x17] = new InstructionRLA();
        opcode[0x18] = new InstructionJPe8();
        opcode[0x19] = new InstructionAddHLDE();
        opcode[0x1A] = new InstructionLdADE();
        opcode[0x1B] = new InstructionDecDE();
        opcode[0x1C] = new InstructionIncE();
        opcode[0x1D] = new InstructionDecE();
        opcode[0x1E] = new InstructionLdE();
        opcode[0x1F] = new InstructionRRA();

        opcode[0x20] = new InstructionJRNZ();
        opcode[0x21] = new InstructionLdHLu16();
        opcode[0x22] = new InstructionLdHLIA();
        opcode[0x23] = new InstructionIncHL();
        opcode[0x24] = new InstructionIncH();
        opcode[0x25] = new InstructionDecH();
        opcode[0x26] = new InstructionLdH();
        opcode[0x27] = new InstructionDAA();
        opcode[0x28] = new InstructionJRZ();
        opcode[0x29] = new InstructionAddHLHL();
        opcode[0x2A] = new InstructionLdAHLI();
        opcode[0x2B] = new InstructionDecHL();
        opcode[0x2C] = new InstructionIncL();
        opcode[0x2D] = new InstructionDecL();
        opcode[0x2E] = new InstructionLdL();
        opcode[0x2F] = new InstructionCPL();

        opcode[0x30] = new InstructionJRNC();
        opcode[0x31] = new InstructionLdSP();
        opcode[0x32] = new InstructionLdHLDecA();
        opcode[0x33] = new InstructionIncSP();
        opcode[0x34] = new InstructionIncHLderef();
        opcode[0x35] = new InstructionDecHLderef();
        opcode[0x36] = new InstructionLdHLderef();
        opcode[0x37] = new InstructionSCF();
        opcode[0x38] = new InstructionJRC();
        opcode[0x39] = new InstructionAddHLSP();
        opcode[0x3A] = new InstructionLdAHLD();
        opcode[0x3B] = new InstructionDecSP();
        opcode[0x3C] = new InstructionIncA();
        opcode[0x3D] = new InstructionDecA();
        opcode[0x3E] = new InstructionLdA();
        opcode[0x3F] = new InstructionCCF();

        opcode[0x40] = new InstructionLdBB();
        opcode[0x41] = new InstructionLdBC();
        opcode[0x42] = new InstructionLdBD();
        opcode[0x43] = new InstructionLdBE();
        opcode[0x44] = new InstructionLdBH();
        opcode[0x45] = new InstructionLdBL();
        opcode[0x46] = new InstructionLdBHL();
        opcode[0x47] = new InstructionLdBA();
        opcode[0x48] = new InstructionLdCB();
        opcode[0x49] = new InstructionLdCC();
        opcode[0x4A] = new InstructionLdCD();
        opcode[0x4B] = new InstructionLdCE();
        opcode[0x4C] = new InstructionLdCH();
        opcode[0x4D] = new InstructionLdCL();
        opcode[0x4E] = new InstructionLdCHL();
        opcode[0x4F] = new InstructionLdCA();

        opcode[0x50] = new InstructionLdDB();
        opcode[0x51] = new InstructionLdDC();
        opcode[0x52] = new InstructionLdDD();
        opcode[0x53] = new InstructionLdDE();
        opcode[0x54] = new InstructionLdDH();
        opcode[0x55] = new InstructionLdDL();
        opcode[0x56] = new InstructionLdDHL();
        opcode[0x57] = new InstructionLdDA();
        opcode[0x58] = new InstructionLdEB();
        opcode[0x59] = new InstructionLdEC();
        opcode[0x5A] = new InstructionLdED();
        opcode[0x5B] = new InstructionLdEE();
        opcode[0x5C] = new InstructionLdEH();
        opcode[0x5D] = new InstructionLdEL();
        opcode[0x5E] = new InstructionLdEHL();
        opcode[0x5F] = new InstructionLdEA();

        opcode[0x60] = new InstructionLdHB();
        opcode[0x61] = new InstructionLdHC();
        opcode[0x62] = new InstructionLdHD();
        opcode[0x63] = new InstructionLdHE();
        opcode[0x64] = new InstructionLdHH();
        opcode[0x65] = new InstructionLdHL();
        opcode[0x66] = new InstructionLdHHL();
        opcode[0x67] = new InstructionLdHA();
        opcode[0x68] = new InstructionLdLB();
        opcode[0x69] = new InstructionLdLC();
        opcode[0x6A] = new InstructionLdLD();
        opcode[0x6B] = new InstructionLdLE();
        opcode[0x6C] = new InstructionLdLH();
        opcode[0x6D] = new InstructionLdLL();
        opcode[0x6E] = new InstructionLdHL();
        opcode[0x6F] = new InstructionLdLA();

        opcode[0x70] = new InstructionLdHLB();
        opcode[0x71] = new InstructionLdHLC();
        opcode[0x72] = new InstructionLdHLD();
        opcode[0x73] = new InstructionLdHLE();
        opcode[0x74] = new InstructionLdHLH();
        opcode[0x75] = new InstructionLdHLL();
        opcode[0x76] = new InstructionHALT( dbg.sstate );
        opcode[0x77] = new InstructionLdHLA();
        opcode[0x78] = new InstructionLdAB();
        opcode[0x79] = new InstructionLdAC();
        opcode[0x7A] = new InstructionLdAD();
        opcode[0x7B] = new InstructionLdAE();
        opcode[0x7C] = new InstructionLdAH();
        opcode[0x7D] = new InstructionLdAL();
        opcode[0x7E] = new InstructionLdAHL();
        opcode[0x7F] = new InstructionLdAA();

        opcode[0x80] = new InstructionAddAB();
        opcode[0x81] = new InstructionAddAC();
        opcode[0x82] = new InstructionAddAD();
        opcode[0x83] = new InstructionAddAE();
        opcode[0x84] = new InstructionAddAH();
        opcode[0x85] = new InstructionAddAL();
        opcode[0x86] = new InstructionAddAHL();
        opcode[0x87] = new InstructionAddAA();
        opcode[0x88] = new InstructionAdcAB();
        opcode[0x89] = new InstructionAdcAC();
        opcode[0x8A] = new InstructionAdcAD();
        opcode[0x8B] = new InstructionAdcAE();
        opcode[0x8C] = new InstructionAdcAH();
        opcode[0x8D] = new InstructionAdcAL();
        opcode[0x8E] = new InstructionAdcAHL();
        opcode[0x8F] = new InstructionAdcAA();

        opcode[0x90] = new InstructionSubAB();
        opcode[0x91] = new InstructionSubAC();
        opcode[0x92] = new InstructionSubAD();
        opcode[0x93] = new InstructionSubAE();
        opcode[0x94] = new InstructionSubAH();
        opcode[0x95] = new InstructionSubAL();
        opcode[0x96] = new InstructionSubAHL();
        opcode[0x97] = new InstructionSubAA();
        opcode[0x98] = new InstructionSbcAB();
        opcode[0x99] = new InstructionSbcAC();
        opcode[0x9A] = new InstructionSbcAD();
        opcode[0x9B] = new InstructionSbcAE();
        opcode[0x9C] = new InstructionSbcAH();
        opcode[0x9D] = new InstructionSbcAL();
        opcode[0x9E] = new InstructionSbcAHL();
        opcode[0x9F] = new InstructionSbcAA();

        opcode[0xA0] = new InstructionAndAB();
        opcode[0xA1] = new InstructionAndAC();
        opcode[0xA2] = new InstructionAndAD();
        opcode[0xA3] = new InstructionAndAE();
        opcode[0xA4] = new InstructionAndAH();
        opcode[0xA5] = new InstructionAndAL();
        opcode[0xA6] = new InstructionAndAHL();
        opcode[0xA7] = new InstructionAndAA();
        opcode[0xA8] = new InstructionXorAB();
        opcode[0xA9] = new InstructionXorAC();
        opcode[0xAA] = new InstructionXorAD();
        opcode[0xAB] = new InstructionXorAE();
        opcode[0xAC] = new InstructionXorAH();
        opcode[0xAD] = new InstructionXorAL();
        opcode[0xAE] = new InstructionXorAHL();
        opcode[0xAF] = new InstructionXorAA();

        opcode[0xB0] = new InstructionOrAB();
        opcode[0xB1] = new InstructionOrAC();
        opcode[0xB2] = new InstructionOrAD();
        opcode[0xB3] = new InstructionOrAE();
        opcode[0xB4] = new InstructionOrAH();
        opcode[0xB5] = new InstructionOrAL();
        opcode[0xB6] = new InstructionOrAHL();
        opcode[0xB7] = new InstructionOrAA();
        opcode[0xB8] = new InstructionCpAB();
        opcode[0xB9] = new InstructionCpAC();
        opcode[0xBA] = new InstructionCpAD();
        opcode[0xBB] = new InstructionCpAE();
        opcode[0xBC] = new InstructionCpAH();
        opcode[0xBD] = new InstructionCpAL();
        opcode[0xBE] = new InstructionCpAHL();
        opcode[0xBF] = new InstructionCpAA();

        opcode[0xC0] = new InstructionRetNZ();
        opcode[0xC1] = new InstructionPopBC();
        opcode[0xC2] = new InstructionJpNZu16();
        opcode[0xC3] = new InstructionJPu16();
        opcode[0xC4] = new InstructionCallNZ();
        opcode[0xC5] = new InstructionPushBC();
        opcode[0xC6] = new InstructionAddA();
        opcode[0xC7] = new InstructionRST00();
        opcode[0xC8] = new InstructionRetZ();
        opcode[0xC9] = new InstructionRet();
        opcode[0xCA] = new InstructionJpZu16();
        opcode[0xCB] = new InstructionEx();
        opcode[0xCC] = new InstructionCallZ();
        opcode[0xCD] = new InstructionCall();
        opcode[0xCE] = new InstructionAdcA();
        opcode[0xCF] = new InstructionRST08();

        opcode[0xD0] = new InstructionRetNC();
        opcode[0xD1] = new InstructionPopDE();
        opcode[0xD2] = new InstructionJpNCu16();
        opcode[0xD3] = new Instruction();
        opcode[0xD4] = new InstructionCallNC();
        opcode[0xD5] = new InstructionPushDE();
        opcode[0xD6] = new InstructionSubAu8();
        opcode[0xD7] = new InstructionRST10();
        opcode[0xD8] = new InstructionRetC();
        opcode[0xD9] = new InstructionRetI();
        opcode[0xDA] = new InstructionJpCu16();
        opcode[0xDB] = new Instruction();
        opcode[0xDC] = new InstructionCallC();
        opcode[0xDD] = new Instruction();
        opcode[0xDE] = new InstructionSbcA();
        opcode[0xDF] = new InstructionRST18();

        opcode[0xE0] = new InstructionLdFFA();
        opcode[0xE1] = new InstructionPopHL();
        opcode[0xE2] = new InstructionLdFFCA();
        opcode[0xE5] = new InstructionPushHL();
        opcode[0xE6] = new InstructionAddAu8();
        opcode[0xE7] = new InstructionRST20();
        opcode[0xE8] = new InstructionAddSPi8();
        opcode[0xE9] = new InstructionJpHL();
        opcode[0xEA] = new InstructionLdu16A();
        opcode[0xEE] = new InstructionXorA();
        opcode[0xEF] = new InstructionRST28();

        opcode[0xF0] = new InstructionLdAFF();
        opcode[0xF1] = new InstructionPopAF();
        opcode[0xF2] = new InstructionLdAFFC();
        opcode[0xF3] = new InstructionDI();
        opcode[0xF4] = new InstructionPushAF();
        opcode[0xF6] = new InstructionOrA();
        opcode[0xF7] = new InstructionRST30();
        opcode[0xF8] = new InstructionLdHLSPi8();
        opcode[0xF9] = new InstructionLdSPHL();
        opcode[0xFA] = new InstructionLdAu16();
        opcode[0xFB] = new InstructionEI();
        opcode[0xFE] = new InstructionCpA();
        opcode[0xFF] = new InstructionRST38();
    }

    ~Emulator( void )
    {
    }

    void step( void )
    {
        if ( dbg.sstate != StepState::STOP ) {
            opcode[mem.map[r.PC]]->execute( &mem, &r );
            if ( dbg.sstate == StepState::STEP ) {
                dbg.sstate = StepState::STOP;
            }
        }

        if ( std::find( dbg.bps.begin(), dbg.bps.end(), r.PC ) != dbg.bps.end() ) {
            dbg.sstate = StepState::STOP;
        }

    }

    void draw( void )
    {
        // tile is 16 bytes array
        uint8_t* tile = mem.vram.tile( 0 );

        // tile row is 2 bytes each
        for ( int row = 0; row < 8; row++ ) {

            uint8_t* trow = &tile[row * 2];

            for ( int col = 7; col >= 0; col-- ) {

                uint8_t pixel = 0;
                pixel = trow[0] & ( 1 << col );
                pixel <<= 1;
                pixel |= trow[1] & ( 1 << col );
                ImGui::Text( "%d", pixel );
                if ( col ) {
                    ImGui::SameLine();
                }
            }
        }
    }

    void debugger( void )
    {
        dbg.show_memory( &mem );
        dbg.show_registers( &r );
        dbg.show_disassembly( &mem, &r, opcode );
    }
};