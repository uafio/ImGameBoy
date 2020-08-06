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
        r->Flag.Z = r->Flag.N = r->Flag.H = 0;
        r->Flag.C = operand >> 7;
        return ( operand << 1 ) + r->Flag.C;
    }

    uint8_t rl( Registers* r, uint8_t operand )
    {
        r->Flag.Z = r->Flag.N = r->Flag.H = 0;
        r->Flag.C = operand >> 7;
        return operand << 1;
    }

    uint8_t rrc( Registers* r, uint8_t operand )
    {
        r->Flag.Z = r->Flag.N = r->Flag.H = 0;
        r->Flag.C = operand & 1;
        return ( operand >> 1 ) | ( r->Flag.C << 7 );
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



class InstructionLdABC : public Instruction
{
public:
    InstructionLdABC( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = m->rom[r->BC];
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "ld A, (BC)" );
    }
};


class InstructionDecBC : public Instruction
{
public:
    InstructionDecBC( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->BC--;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "dec BC" );
    }
};


class InstructionIncC : public Instruction
{
public:
    InstructionIncC( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->C = increment( r, r->C );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "inc C" );
    }
};


class InstructionDecC : public Instruction
{
public:
    InstructionDecC( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->C = decrement( r, r->C );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "dec C" );
    }
};


class InstructionLdC : public Instruction
{
public:
    InstructionLdC( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->C = m->rom[r->PC + 1];
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "ld C, %#02x", mem->rom[addr + 1] );
    }
};


class InstructionRRCA : public Instruction
{
public:
    InstructionRRCA( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = rrc( r, r->A );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "rrca" );
    }
};


class InstructionStop : public Instruction
{
public:
    InstructionStop( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        if ( m->rom[r->PC + 1] == 0 ) {
            // TODO: need a way to change dbg.sstate = StepState::STOP;
        }
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "stop" );
    }
};


class InstructionLdDE : public Instruction
{
public:
    InstructionLdDE( void )
        : Instruction::Instruction( 3 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->DE = *(uint16_t*)&m->rom[r->PC + 1];
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "ld DE, %#04x", *(uint16_t*)&mem->rom[ addr + 1 ] );
    }
};


class InstructionLdDEA : public Instruction
{
public:
    InstructionLdDEA( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        m->rom[r->DE] = r->A;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "ld (DE), A" );
    }
};

class InstructionIncDE : public Instruction
{
public:
    InstructionIncDE( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->DE++;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "inc DE" );
    }
};

class InstructionIncD : public Instruction
{
public:
    InstructionIncD( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->D = increment( r, r->D );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "inc D" );
    }
};

class InstructionDecD : public Instruction
{
public:
    InstructionDecD( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->D = decrement( r, r->D );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "dec D" );
    }
};

class InstructionLdD : public Instruction
{
public:
    InstructionLdD( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->D = m->rom[r->PC + 1];
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "ld D, %#02x", mem->rom[ addr + 1 ] );
    }
};

class InstructionRLA : public Instruction
{
public:
    InstructionRLA( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = rl( r, r->A );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "rla" );
    }
};

class InstructionJPe8 : public Instruction
{
public:
    InstructionJPe8( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->PC += m->rom[r->PC + 1];
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "jp %#x", addr + length + mem->rom[addr + 1] );
    }
};
