#pragma once
#include <stdint.h>
#include "memory.h"
#include "registers.h"

class Instruction
{
private:

public:
    int length;

    Instruction( void )
        : length( 1 )
    {
    }

    Instruction( const int l )
    {
        length = l;
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "Undef instruction" );
    }

    uint8_t increment( Registers* r, uint8_t operand )
    {
        r->Flag.N = 0;
        r->Flag.Z = operand == 0xff;
        r->Flag.H = ( operand & 0xf ) == 0xf;
        return ++operand;
    }

    uint8_t decrement( Registers* r, uint8_t operand )
    {
        r->Flag.N = 1;
        r->Flag.Z = operand == 1;
        r->Flag.H = ( operand & 0xf ) == 0;
        return --operand;
    }

    uint8_t rlc( Registers* r, uint8_t operand )
    {
        r->Flag.Z = 0;
        r->Flag.N = 0;
        r->Flag.H = 0;
        r->Flag.C = operand >> 7;
        return ( operand << 1 ) + r->Flag.C;
    }

    void add_hl_r16( Registers* r, uint16_t r16 )
    {
        r->Flag.N = 0;
        r->Flag.H = r16 && ( ( r->HL & 0x0fff ) == 0xfff );
        r->Flag.C = r16 && ( r->HL == 0xffff );
        r->HL += r16;
    }
};


class InstructionNop : public Instruction
{
public:
    InstructionNop( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "NOP" );
    }
};


class InstructionLdBC : public Instruction
{
public:
    InstructionLdBC( void )
        : Instruction::Instruction( 3 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->BC = *(uint16_t*)&m->map[r->PC + 1];
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "ld BC, %#04x", *(uint16_t*)&mem->rom[ addr + 1 ] );
    }
};



class InstructionLdBCA : public Instruction
{
public:
    InstructionLdBCA( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        m->rom[r->BC] = r->A;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "ld (BC), A" );
    }
};


class InstructionIncBC : public Instruction
{
public:
    InstructionIncBC( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->BC++;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "inc BC" );
    }
};



class InstructionIncB : public Instruction
{
public:
    InstructionIncB( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->B = increment( r, r->B );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "inc B" );
    }
};


class InstructionDecB : public Instruction
{
public:
    InstructionDecB( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->B = decrement( r, r->B );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "dec B" );
    }
};



class InstructionLdB : public Instruction
{
public:
    InstructionLdB( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->B = m->rom[r->PC + 1];
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "ld B, %#02x", mem->rom[addr + 1] );
    }
};


class InstructionRLCA : public Instruction
{
public:
    InstructionRLCA( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = rlc( r, r->A );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "rlca" );
    }
};


class InstructionLdn16SP : public Instruction
{
public:
    InstructionLdn16SP( void )
        : Instruction::Instruction( 3 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        uint16_t addr = *(uint16_t*)&m->rom[r->PC + 1];
        *(uint16_t*)&m->rom[addr] = r->SP;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "ld %#04x, SP", *(uint16_t*)&mem->rom[addr + 1] );
    }
};


class InstructionAddHLBC : public Instruction
{
public:
    InstructionAddHLBC( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        add_hl_r16( r, r->BC );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "add HL, BC" );
    }
};
