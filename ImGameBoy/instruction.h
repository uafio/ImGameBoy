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

    uint8_t rlca( Registers* r, uint8_t operand )
    {
        r->Flag.Z = r->Flag.N = r->Flag.H = 0;
        r->Flag.C = operand >> 7;
        return ( operand << 1 ) + r->Flag.C;
    }

    uint8_t rla( Registers* r, uint8_t operand )
    {
        r->Flag.Z = r->Flag.N = r->Flag.H = 0;
        r->Flag.C = operand >> 7;
        return operand << 1;
    }

    uint8_t rrca( Registers* r, uint8_t operand )
    {
        r->Flag.Z = r->Flag.N = r->Flag.H = 0;
        r->Flag.C = operand & 1;
        return ( operand >> 1 ) | ( r->Flag.C << 7 );
    }

    uint8_t rr( Registers* r, uint8_t operand )
    {
        r->Flag.Z = r->Flag.N = r->Flag.H = 0;
        r->Flag.C = operand & 1;
        return operand >> 1;
    }

    void add_hl_r16( Registers* r, uint16_t r16 )
    {
        r->Flag.N = 0;
        r->Flag.H = r16 > ( 0xfff - ( r->HL & 0xfff ) );
        r->Flag.C = r16 > ( 0xffff - r->HL );
        r->HL += r16;
    }

    template< class T>
    T add( Registers* r, T rd, T rs )
    {
        T result = rd + rs;

        r->Flag.N = 0;
        r->Flag.Z = result == 0;
        r->Flag.H = rs > ( 0xf - ( rd & 0xf ) );
        r->Flag.C = rs > ( 0xff - rd );

        return result;
    }

    uint8_t adc( Registers* r, uint8_t rd, uint8_t rs )
    {
        uint8_t result = rd + rs + r->Flag.C;

        r->Flag.N = 0;
        r->Flag.Z = result == 0;
        r->Flag.H = ( rs + r->Flag.C ) > ( 0xf - ( rd & 0xf ) );
        r->Flag.C = ( rs + r->Flag.C ) > ( 0xff - rd );

        return result;
    }

    uint8_t sub( Registers* r, uint8_t rd, uint8_t rs )
    {
        uint8_t result = rd - rs;

        r->Flag.Z = result == 0;
        r->Flag.N = 1;
        r->Flag.H = rs && ( rd & 0xf ) == 0;
        r->Flag.C = rs > rd;

        return result;
    }

    uint8_t sbc( Registers* r, uint8_t rd, uint8_t rs )
    {
        uint8_t result = rd - rs - r->Flag.C;

        r->Flag.Z = result == 0;
        r->Flag.N = 1;
        r->Flag.H = ( rs || r->Flag.C ) && ( rd & 0xf ) == 0;
        r->Flag.C = ( r->Flag.C + rs ) > rd;

        return result;
    }

    uint8_t _and( Registers* r, uint8_t rd, uint8_t rs )
    {
        uint8_t result = rd & rs;

        r->Flag.Z = result == 0;
        r->Flag.N = r->Flag.C = 0;
        r->Flag.H = 1;

        return result;
    }

    uint8_t _xor( Registers* r, uint8_t rd, uint8_t rs )
    {
        uint8_t result = rd ^ rs;

        r->Flag.Z = result == 0;
        r->Flag.N = r->Flag.H = r->Flag.C = 0;

        return result;
    }

    uint8_t _or( Registers* r, uint8_t rd, uint8_t rs )
    {
        uint8_t result = rd | rs;

        r->Flag.Z = result == 0;
        r->Flag.N = r->Flag.H = r->Flag.C = 0;

        return result;
    }

    void push( Memory* mem, Registers* r, uint16_t addr )
    {
        r->SP -= 2;
        *(uint16_t*)&mem->rom[r->SP] = addr;
    }

    uint16_t pop( Memory* mem, Registers* r )
    {
        uint16_t result = *(uint16_t*)&mem->rom[r->SP];
        r->SP += 2;
        return result;
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

class InstructionLdBCu16 : public Instruction
{
public:
    InstructionLdBCu16( void )
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
        snprintf( dst, size, "LD BC, %#04x", *(uint16_t*)&mem->rom[addr + 1] );
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
        snprintf( dst, size, "LD (BC), A" );
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
        snprintf( dst, size, "INC BC" );
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
        snprintf( dst, size, "INC B" );
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
        snprintf( dst, size, "DEC B" );
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
        snprintf( dst, size, "LD B, %#02x", mem->rom[addr + 1] );
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
        r->A = rlca( r, r->A );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RLCA" );
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
        snprintf( dst, size, "LD %#04x, SP", *(uint16_t*)&mem->rom[addr + 1] );
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
        snprintf( dst, size, "ADD HL, BC" );
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
        snprintf( dst, size, "LD A, (BC)" );
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
        snprintf( dst, size, "DEC BC" );
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
        snprintf( dst, size, "INC C" );
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
        snprintf( dst, size, "DEC C" );
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
        snprintf( dst, size, "LD C, %#02x", mem->rom[addr + 1] );
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
        r->A = rrca( r, r->A );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RRCA" );
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
        snprintf( dst, size, "STOP" );
    }
};

class InstructionLdDEu16 : public Instruction
{
public:
    InstructionLdDEu16( void )
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
        snprintf( dst, size, "LD DE, %#04x", *(uint16_t*)&mem->rom[addr + 1] );
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
        snprintf( dst, size, "LD (DE), A" );
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
        snprintf( dst, size, "INC DE" );
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
        snprintf( dst, size, "INC D" );
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
        snprintf( dst, size, "DEC D" );
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
        snprintf( dst, size, "LD D, %#02x", mem->rom[addr + 1] );
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
        r->A = rla( r, r->A );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RLA" );
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
        snprintf( dst, size, "JP %#x", addr + length + mem->rom[addr + 1] );
    }
};

class InstructionAddHLDE : public Instruction
{
public:
    InstructionAddHLDE( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        add_hl_r16( r, r->DE );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "ADD HL, DE" );
    }
};

class InstructionLdADE : public Instruction
{
public:
    InstructionLdADE( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = m->rom[r->DE];
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD A, (DE)" );
    }
};

class InstructionDecDE : public Instruction
{
public:
    InstructionDecDE( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->DE--;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "DEC DE" );
    }
};

class InstructionIncE : public Instruction
{
public:
    InstructionIncE( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->E = increment( r, r->E );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "INC E" );
    }
};

class InstructionDecE : public Instruction
{
public:
    InstructionDecE( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->E = decrement( r, r->E );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "DEC E" );
    }
};

class InstructionLdE : public Instruction
{
public:
    InstructionLdE( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->E = m->rom[r->PC + 1];
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD E, %#02x", mem->rom[addr + 1] );
    }
};

class InstructionRRA : public Instruction
{
public:
    InstructionRRA( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = rr( r, r->A );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RRA" );
    }
};

class InstructionJRNZ : public Instruction
{
public:
    InstructionJRNZ( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        if ( r->Flag.Z == 0 ) {
            r->PC += m->rom[r->PC + 1];
        }
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "JR NZ, %#04x", addr + length + mem->rom[addr + 1] );
    }
};

class InstructionLdHLu16 : public Instruction
{
public:
    InstructionLdHLu16( void )
        : Instruction::Instruction( 3 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->HL = *(uint16_t*)&m->rom[r->PC + 1];
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD HL, %#04x", *(uint16_t*)&mem->rom[addr + 1] );
    }
};

class InstructionLdHLIA : public Instruction
{
public:
    InstructionLdHLIA( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        m->rom[r->HL++] = r->A;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD (HL+), A" );
    }
};

class InstructionIncHL : public Instruction
{
public:
    InstructionIncHL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->HL++;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "INC HL" );
    }
};

class InstructionIncH : public Instruction
{
public:
    InstructionIncH( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->H = increment( r, r->H );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "INC H" );
    }
};

class InstructionDecH : public Instruction
{
public:
    InstructionDecH( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->H = decrement( r, r->H );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "DEC H" );
    }
};

class InstructionLdH : public Instruction
{
public:
    InstructionLdH( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->H = m->rom[r->PC + 1];
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "Ld H, %#02x", mem->rom[addr + 1] );
    }
};


class InstructionDAA : public Instruction
{
public:
    InstructionDAA( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        // TODO
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "DAA" );
    }
};


class InstructionJRZ : public Instruction
{
public:
    InstructionJRZ( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        if ( r->Flag.Z ) {
            r->PC += m->rom[r->PC + 1];
        }
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "JR Z, %#04x", addr + length + mem->rom[addr + 1] );
    }
};

class InstructionAddHLHL : public Instruction
{
public:
    InstructionAddHLHL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        add_hl_r16( r, r->HL );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "ADD HL, HL" );
    }
};

class InstructionLdAHLI : public Instruction
{
public:
    InstructionLdAHLI( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = m->rom[r->HL++];
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD A, (HL+)" );
    }
};

class InstructionDecHL : public Instruction
{
public:
    InstructionDecHL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->HL--;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "DEC HL" );
    }
};

class InstructionIncL : public Instruction
{
public:
    InstructionIncL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->L = increment( r, r->L );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "INC L" );
    }
};

class InstructionDecL : public Instruction
{
public:
    InstructionDecL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->L = decrement( r, r->L );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "DEC L" );
    }
};

class InstructionLdL : public Instruction
{
public:
    InstructionLdL( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->L = m->rom[r->PC + 1];
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD L, %#02x", mem->rom[addr + 1] );
    }
};

class InstructionCPL : public Instruction
{
public:
    InstructionCPL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->Flag.N = r->Flag.H = 1;
        r->A = ~r->A;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "CPL" );
    }
};

class InstructionJRNC : public Instruction
{
public:
    InstructionJRNC( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        if ( r->Flag.C == 0 ) {
            r->PC += m->rom[r->PC + 1];
        }
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "JR NC, %#04x", addr + length + mem->rom[addr + 1] );
    }
};

class InstructionLdSP : public Instruction
{
public:
    InstructionLdSP( void )
        : Instruction::Instruction( 3 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->SP = *(uint16_t*)&m->rom[r->PC + 1];
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD SP, %#04x", *(uint16_t*)&mem->rom[addr + 1] );
    }
};

class InstructionLdHLDecA : public Instruction
{
public:
    InstructionLdHLDecA( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = m->rom[r->HL--];
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD (HL-), A" );
    }
};

class InstructionIncSP : public Instruction
{
public:
    InstructionIncSP( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->SP++;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "INC SP" );
    }
};

class InstructionIncHLderef : public Instruction
{
public:
    InstructionIncHLderef( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        m->rom[r->HL] = increment( r, m->rom[r->HL] );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "INC (HL)" );
    }
};

class InstructionDecHLderef : public Instruction
{
public:
    InstructionDecHLderef( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        m->rom[r->HL] = decrement( r, m->rom[r->HL] );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "DEC (HL)" );
    }
};

class InstructionLdHLderef : public Instruction
{
public:
    InstructionLdHLderef( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        m->rom[r->HL] = m->rom[r->PC + 1];
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD (HL), %#02x", mem->rom[addr + 1] );
    }
};

class InstructionSCF : public Instruction
{
public:
    InstructionSCF( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->Flag.N = r->Flag.H = 0;
        r->Flag.C = 1;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SCF" );
    }
};

class InstructionJRC : public Instruction
{
public:
    InstructionJRC( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        if ( r->Flag.C ) {
            r->PC += m->rom[r->PC + 1];
        }
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "JR C, %#04x", addr + length + mem->rom[addr + 1] );
    }
};

class InstructionAddHLSP : public Instruction
{
public:
    InstructionAddHLSP( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        add_hl_r16( r, r->SP );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "ADD HL, SP" );
    }
};

class InstructionLdAHLD : public Instruction
{
public:
    InstructionLdAHLD( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = m->rom[r->HL--];
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD A, (HL-)" );
    }
};

class InstructionDecSP : public Instruction
{
public:
    InstructionDecSP( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->SP--;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "DEC SP" );
    }
};

class InstructionIncA : public Instruction
{
public:
    InstructionIncA( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = increment( r, r->A );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "INC A" );
    }
};

class InstructionDecA : public Instruction
{
public:
    InstructionDecA( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = decrement( r, r->A );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "DEC A" );
    }
};

class InstructionLdA : public Instruction
{
public:
    InstructionLdA( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = m->rom[r->PC + 1];
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD A, %#02x", mem->rom[addr + 1] );
    }
};

class InstructionCCF : public Instruction
{
public:
    InstructionCCF( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->Flag.N = r->Flag.H = 0;
        r->Flag.C ^= 1;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "CCF" );
    }
};

class InstructionLdBB : public Instruction
{
public:
    InstructionLdBB( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD B, B" );
    }
};

class InstructionLdBC : public Instruction
{
public:
    InstructionLdBC( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->B = r->C;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD B, C" );
    }
};

class InstructionLdBD : public Instruction
{
public:
    InstructionLdBD( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->B = r->D;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD B, D" );
    }
};

class InstructionLdBE : public Instruction
{
public:
    InstructionLdBE( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->B = r->E;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD B, E" );
    }
};

class InstructionLdBH : public Instruction
{
public:
    InstructionLdBH( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->B = r->H;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD B, H" );
    }
};

class InstructionLdBL : public Instruction
{
public:
    InstructionLdBL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->B = r->L;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD B, L" );
    }
};

class InstructionLdBHL : public Instruction
{
public:
    InstructionLdBHL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->B = m->rom[r->HL];
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD B, (HL)" );
    }
};

class InstructionLdBA : public Instruction
{
public:
    InstructionLdBA( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->B = r->A;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD B, A" );
    }
};

class InstructionLdCB : public Instruction
{
public:
    InstructionLdCB( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->C = r->B;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD C, B" );
    }
};

class InstructionLdCC : public Instruction
{
public:
    InstructionLdCC( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD C, C" );
    }
};

class InstructionLdCD : public Instruction
{
public:
    InstructionLdCD( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->C = r->D;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD C, D" );
    }
};

class InstructionLdCE : public Instruction
{
public:
    InstructionLdCE( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->C = r->E;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD C, E" );
    }
};

class InstructionLdCH : public Instruction
{
public:
    InstructionLdCH( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->C = r->H;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD C, H" );
    }
};

class InstructionLdCL : public Instruction
{
public:
    InstructionLdCL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->C = r->L;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD C, L" );
    }
};

class InstructionLdCHL : public Instruction
{
public:
    InstructionLdCHL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->C = m->rom[r->HL];
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD C, (HL)" );
    }
};

class InstructionLdCA : public Instruction
{
public:
    InstructionLdCA( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->C = r->A;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD C, A" );
    }
};

class InstructionLdDB : public Instruction
{
public:
    InstructionLdDB( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->D = r->B;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD D, B" );
    }
};

class InstructionLdDC : public Instruction
{
public:
    InstructionLdDC( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->D = r->C;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD D, C" );
    }
};

class InstructionLdDD : public Instruction
{
public:
    InstructionLdDD( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD D, D" );
    }
};

class InstructionLdDE : public Instruction
{
public:
    InstructionLdDE( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->D = r->E;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD D, E" );
    }
};

class InstructionLdDH : public Instruction
{
public:
    InstructionLdDH( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->D = r->H;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD D, H" );
    }
};

class InstructionLdDL : public Instruction
{
public:
    InstructionLdDL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->D = r->L;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD D, L" );
    }
};

class InstructionLdDHL : public Instruction
{
public:
    InstructionLdDHL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->D = m->rom[r->HL];
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD D, (HL)" );
    }
};

class InstructionLdDA : public Instruction
{
public:
    InstructionLdDA( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->D = r->A;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD D, A" );
    }
};

class InstructionLdEB : public Instruction
{
public:
    InstructionLdEB( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->E = r->B;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD E, B" );
    }
};

class InstructionLdEC : public Instruction
{
public:
    InstructionLdEC( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->E = r->C;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD E, C" );
    }
};

class InstructionLdED : public Instruction
{
public:
    InstructionLdED( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->E = r->D;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD E, D" );
    }
};

class InstructionLdEE : public Instruction
{
public:
    InstructionLdEE( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD E, E" );
    }
};

class InstructionLdEH : public Instruction
{
public:
    InstructionLdEH( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->E = r->H;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD E, H" );
    }
};

class InstructionLdEL : public Instruction
{
public:
    InstructionLdEL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->E = r->L;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD E, L" );
    }
};

class InstructionLdEHL : public Instruction
{
public:
    InstructionLdEHL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->E = m->rom[r->HL];
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD E, (HL)" );
    }
};

class InstructionLdEA : public Instruction
{
public:
    InstructionLdEA( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->E = r->A;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD E, A" );
    }
};

class InstructionLdHB : public Instruction
{
public:
    InstructionLdHB( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->H = r->B;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD H, B" );
    }
};

class InstructionLdHC : public Instruction
{
public:
    InstructionLdHC( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->H = r->C;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD H, C" );
    }
};

class InstructionLdHD : public Instruction
{
public:
    InstructionLdHD( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->H = r->D;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD H, D" );
    }
};

class InstructionLdHE : public Instruction
{
public:
    InstructionLdHE( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->H = r->E;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD H, E" );
    }
};

class InstructionLdHH : public Instruction
{
public:
    InstructionLdHH( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD H, H" );
    }
};

class InstructionLdHL : public Instruction
{
public:
    InstructionLdHL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->H = r->L;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD H, L" );
    }
};

class InstructionLdHHL : public Instruction
{
public:
    InstructionLdHHL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->H = m->rom[r->HL];
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD H, (HL)" );
    }
};

class InstructionLdHA : public Instruction
{
public:
    InstructionLdHA( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->H = r->A;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD H, A" );
    }
};

class InstructionLdLB : public Instruction
{
public:
    InstructionLdLB( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->L = r->B;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD L, B" );
    }
};

class InstructionLdLC : public Instruction
{
public:
    InstructionLdLC( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->L = r->C;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD L, C" );
    }
};

class InstructionLdLD : public Instruction
{
public:
    InstructionLdLD( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->L = r->D;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD L, D" );
    }
};

class InstructionLdLE : public Instruction
{
public:
    InstructionLdLE( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->L = r->E;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD L, E" );
    }
};

class InstructionLdLH : public Instruction
{
public:
    InstructionLdLH( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->L = r->H;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD L, H" );
    }
};

class InstructionLdLL : public Instruction
{
public:
    InstructionLdLL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD L, L" );
    }
};

class InstructionLdLHL : public Instruction
{
public:
    InstructionLdLHL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->L = m->rom[r->HL];
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD L, (HL)" );
    }
};

class InstructionLdLA : public Instruction
{
public:
    InstructionLdLA( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->L = r->A;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD L, A" );
    }
};

class InstructionLdHLB : public Instruction
{
public:
    InstructionLdHLB( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        m->rom[r->HL] = r->B;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD (HL), B" );
    }
};

class InstructionLdHLC : public Instruction
{
public:
    InstructionLdHLC( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        m->rom[r->HL] = r->C;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD (HL), C" );
    }
};

class InstructionLdHLD : public Instruction
{
public:
    InstructionLdHLD( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        m->rom[r->HL] = r->D;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD (HL), D" );
    }
};

class InstructionLdHLE : public Instruction
{
public:
    InstructionLdHLE( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        m->rom[r->HL] = r->E;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD (HL), E" );
    }
};

class InstructionLdHLH : public Instruction
{
public:
    InstructionLdHLH( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        m->rom[r->HL] = r->H;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD (HL), H" );
    }
};

class InstructionLdHLL : public Instruction
{
public:
    InstructionLdHLL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        m->rom[r->HL] = r->L;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD (HL), L" );
    }
};

class InstructionHALT : public Instruction
{
public:
    InstructionHALT( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        // TODO: Implement HALT
        // probably change StepState
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "HALT" );
    }
};

class InstructionLdHLA : public Instruction
{
public:
    InstructionLdHLA( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        m->rom[r->HL] = r->A;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD (HL), A" );
    }
};

class InstructionLdAB : public Instruction
{
public:
    InstructionLdAB( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = r->B;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD A, B" );
    }
};

class InstructionLdAC : public Instruction
{
public:
    InstructionLdAC( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = r->C;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD A, C" );
    }
};

class InstructionLdAD : public Instruction
{
public:
    InstructionLdAD( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = r->D;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD A, D" );
    }
};

class InstructionLdAE : public Instruction
{
public:
    InstructionLdAE( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = r->E;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD A, E" );
    }
};

class InstructionLdAH : public Instruction
{
public:
    InstructionLdAH( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = r->H;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD A, H" );
    }
};

class InstructionLdAL : public Instruction
{
public:
    InstructionLdAL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = r->L;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD A, L" );
    }
};

class InstructionLdAHL : public Instruction
{
public:
    InstructionLdAHL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = m->rom[r->HL];
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD A, (HL)" );
    }
};

class InstructionLdAA : public Instruction
{
public:
    InstructionLdAA( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD A, A" );
    }
};

class InstructionAddAB : public Instruction
{
public:
    InstructionAddAB( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = add( r, r->A, r->B );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "ADD A, B" );
    }
};

class InstructionAddAC : public Instruction
{
public:
    InstructionAddAC( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = add( r, r->A, r->C );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "ADD A, C" );
    }
};

class InstructionAddAD : public Instruction
{
public:
    InstructionAddAD( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = add( r, r->A, r->D );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "ADD A, D" );
    }
};

class InstructionAddAE : public Instruction
{
public:
    InstructionAddAE( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = add( r, r->A, r->E );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "ADD A, E" );
    }
};

class InstructionAddAH : public Instruction
{
public:
    InstructionAddAH( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = add( r, r->A, r->H );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "ADD A, H" );
    }
};

class InstructionAddAL : public Instruction
{
public:
    InstructionAddAL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = add( r, r->A, r->L );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "ADD A, L" );
    }
};

class InstructionAddAHL : public Instruction
{
public:
    InstructionAddAHL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = add( r, r->A, m->rom[r->HL] );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "ADD A, (HL)" );
    }
};

class InstructionAddAA : public Instruction
{
public:
    InstructionAddAA( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = add( r, r->A, r->A );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "ADD A, A" );
    }
};

class InstructionAdcAB : public Instruction
{
public:
    InstructionAdcAB( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = adc( r, r->A, r->B );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "ADC A, B" );
    }
};

class InstructionAdcAC : public Instruction
{
public:
    InstructionAdcAC( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = adc( r, r->A, r->C );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "ADC A, C" );
    }
};

class InstructionAdcAD : public Instruction
{
public:
    InstructionAdcAD( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = adc( r, r->A, r->D );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "ADC A, D" );
    }
};

class InstructionAdcAE : public Instruction
{
public:
    InstructionAdcAE( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = adc( r, r->A, r->E );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "ADC A, E" );
    }
};

class InstructionAdcAH : public Instruction
{
public:
    InstructionAdcAH( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = adc( r, r->A, r->H );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "ADC A, H" );
    }
};

class InstructionAdcAL : public Instruction
{
public:
    InstructionAdcAL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = adc( r, r->A, r->L );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "ADC A, L" );
    }
};

class InstructionAdcAHL : public Instruction
{
public:
    InstructionAdcAHL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = adc( r, r->A, m->rom[r->HL] );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "ADC A, (HL)" );
    }
};

class InstructionAdcAA : public Instruction
{
public:
    InstructionAdcAA( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = adc( r, r->A, r->A );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "ADC A, A" );
    }
};

class InstructionSubAB : public Instruction
{
public:
    InstructionSubAB( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = sub( r, r->A, r->B );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SUB A, B" );
    }
};

class InstructionSubAC : public Instruction
{
public:
    InstructionSubAC( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = sub( r, r->A, r->C );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SUB A, C" );
    }
};

class InstructionSubAD : public Instruction
{
public:
    InstructionSubAD( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = sub( r, r->A, r->D );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SUB A, D" );
    }
};

class InstructionSubAE : public Instruction
{
public:
    InstructionSubAE( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = sub( r, r->A, r->E );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SUB A, E" );
    }
};

class InstructionSubAH : public Instruction
{
public:
    InstructionSubAH( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = sub( r, r->A, r->H );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SUB A, H" );
    }
};

class InstructionSubAL : public Instruction
{
public:
    InstructionSubAL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = sub( r, r->A, r->L );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SUB A, L" );
    }
};

class InstructionSubAHL : public Instruction
{
public:
    InstructionSubAHL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = sub( r, r->A, m->rom[r->HL] );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SUB A, (HL)" );
    }
};

class InstructionSubAA : public Instruction
{
public:
    InstructionSubAA( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = sub( r, r->A, r->A );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SUB A, A" );
    }
};

class InstructionSbcAB : public Instruction
{
public:
    InstructionSbcAB( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = sbc( r, r->A, r->B );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SBC A, B" );
    }
};

class InstructionSbcAC : public Instruction
{
public:
    InstructionSbcAC( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = sbc( r, r->A, r->C );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SBC A, B" );
    }
};

class InstructionSbcAD : public Instruction
{
public:
    InstructionSbcAD( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = sbc( r, r->A, r->D );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SBC A, D" );
    }
};

class InstructionSbcAE : public Instruction
{
public:
    InstructionSbcAE( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = sbc( r, r->A, r->E );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SBC A, E" );
    }
};

class InstructionSbcAH : public Instruction
{
public:
    InstructionSbcAH( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = sbc( r, r->A, r->H );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SBC A, H" );
    }
};

class InstructionSbcAL : public Instruction
{
public:
    InstructionSbcAL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = sbc( r, r->A, r->L );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SBC A, L" );
    }
};

class InstructionSbcAHL : public Instruction
{
public:
    InstructionSbcAHL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = sbc( r, r->A, m->rom[r->HL] );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SBC A, (HL)" );
    }
};

class InstructionSbcAA : public Instruction
{
public:
    InstructionSbcAA( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = sbc( r, r->A, r->A );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SBC A, A" );
    }
};

class InstructionAndAB : public Instruction
{
public:
    InstructionAndAB( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = _and( r, r->A, r->B );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "AND A, B" );
    }
};

class InstructionAndAC : public Instruction
{
public:
    InstructionAndAC( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = _and( r, r->A, r->C );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "AND A, C" );
    }
};

class InstructionAndAD : public Instruction
{
public:
    InstructionAndAD( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = _and( r, r->A, r->D );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "AND A, D" );
    }
};

class InstructionAndAE : public Instruction
{
public:
    InstructionAndAE( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = _and( r, r->A, r->E );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "AND A, E" );
    }
};

class InstructionAndAH : public Instruction
{
public:
    InstructionAndAH( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = _and( r, r->A, r->H );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "AND A, H" );
    }
};

class InstructionAndAL : public Instruction
{
public:
    InstructionAndAL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = _and( r, r->A, r->L );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "AND A, L" );
    }
};

class InstructionAndAHL : public Instruction
{
public:
    InstructionAndAHL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = _and( r, r->A, m->rom[r->HL] );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "AND A, (HL)" );
    }
};

class InstructionAndAA : public Instruction
{
public:
    InstructionAndAA( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = _and( r, r->A, r->A );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "AND A, A" );
    }
};

class InstructionXorAB : public Instruction
{
public:
    InstructionXorAB( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = _xor( r, r->A, r->B );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "XOR A, B" );
    }
};

class InstructionXorAC : public Instruction
{
public:
    InstructionXorAC( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = _xor( r, r->A, r->C );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "XOR A, C" );
    }
};

class InstructionXorAD : public Instruction
{
public:
    InstructionXorAD( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = _xor( r, r->A, r->D );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "XOR A, D" );
    }
};

class InstructionXorAE : public Instruction
{
public:
    InstructionXorAE( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = _xor( r, r->A, r->E );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "XOR A, E" );
    }
};

class InstructionXorAH : public Instruction
{
public:
    InstructionXorAH( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = _xor( r, r->A, r->H );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "XOR A, H" );
    }
};

class InstructionXorAL : public Instruction
{
public:
    InstructionXorAL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = _xor( r, r->A, r->L );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "XOR A, L" );
    }
};

class InstructionXorAHL : public Instruction
{
public:
    InstructionXorAHL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = _xor( r, r->A, m->rom[r->HL] );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "XOR A, (HL)" );
    }
};

class InstructionXorAA : public Instruction
{
public:
    InstructionXorAA( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = _xor( r, r->A, r->A );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "XOR A, A" );
    }
};

class InstructionOrAB : public Instruction
{
public:
    InstructionOrAB( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = _or( r, r->A, r->B );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "OR A, B" );
    }
};

class InstructionOrAC : public Instruction
{
public:
    InstructionOrAC( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = _or( r, r->A, r->C );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "OR A, C" );
    }
};

class InstructionOrAD : public Instruction
{
public:
    InstructionOrAD( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = _or( r, r->A, r->D );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "OR A, D" );
    }
};

class InstructionOrAE : public Instruction
{
public:
    InstructionOrAE( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = _or( r, r->A, r->E );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "OR A, E" );
    }
};

class InstructionOrAH : public Instruction
{
public:
    InstructionOrAH( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = _or( r, r->A, r->H );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "OR A, H" );
    }
};

class InstructionOrAL : public Instruction
{
public:
    InstructionOrAL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = _or( r, r->A, r->L );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "OR A, L" );
    }
};

class InstructionOrAHL : public Instruction
{
public:
    InstructionOrAHL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = _or( r, r->A, m->rom[r->HL] );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "OR A, (HL)" );
    }
};

class InstructionOrAA : public Instruction
{
public:
    InstructionOrAA( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = _or( r, r->A, r->A );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "OR A, A" );
    }
};

class InstructionCpAB : public Instruction
{
public:
    InstructionCpAB( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        sub( r, r->A, r->B );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "CP A, B" );
    }
};

class InstructionCpAC : public Instruction
{
public:
    InstructionCpAC( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        sub( r, r->A, r->C );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "CP A, C" );
    }
};

class InstructionCpAD : public Instruction
{
public:
    InstructionCpAD( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        sub( r, r->A, r->D );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "CP A, D" );
    }
};

class InstructionCpAE : public Instruction
{
public:
    InstructionCpAE( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        sub( r, r->A, r->E );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "CP A, E" );
    }
};

class InstructionCpAH : public Instruction
{
public:
    InstructionCpAH( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        sub( r, r->A, r->H );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "CP A, H" );
    }
};

class InstructionCpAL : public Instruction
{
public:
    InstructionCpAL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        sub( r, r->A, r->L );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "CP A, L" );
    }
};

class InstructionCpAHL : public Instruction
{
public:
    InstructionCpAHL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        sub( r, r->A, m->rom[r->HL] );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "CP A, (HL)" );
    }
};

class InstructionCpAA : public Instruction
{
public:
    InstructionCpAA( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        sub( r, r->A, r->A );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "CP A, A" );
    }
};

class InstructionRetNZ : public Instruction
{
public:
    InstructionRetNZ( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        if ( !r->Flag.Z ) {
            r->PC = pop( m, r );
            r->SP += 2;
            return;
        }
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RET NZ" );
    }
};

class InstructionPopBC : public Instruction
{
public:
    InstructionPopBC( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->C = m->rom[r->SP++];
        r->B = m->rom[r->SP++];
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "POP BC" );
    }
};

class InstructionJpNZu16 : public Instruction
{
public:
    InstructionJpNZu16( void )
        : Instruction::Instruction( 3 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        if ( !r->Flag.Z ) {
            r->PC = *(uint16_t*)&m->rom[r->PC + 1];
            return;
        }
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "JP NZ, %#04x", *(uint16_t*)&mem->rom[addr + 1] );
    }
};

class InstructionJPu16 : public Instruction
{
public:
    InstructionJPu16( void )
        : Instruction::Instruction( 3 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->PC = *(uint16_t*)&m->rom[r->PC + 1];
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "JP %#04x", *(uint16_t*)&mem->rom[addr + 1] );
    }
};

class InstructionCallNZ : public Instruction
{
public:
    InstructionCallNZ( void )
        : Instruction::Instruction( 3 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        if ( !r->Flag.Z ) {
            push( m, r, r->PC + length );
            r->PC = *(uint16_t*)&m->rom[r->PC + 1];
            return;
        }
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "CALL NZ, %#04x", *(uint16_t*)&mem->rom[addr + 1] );
    }
};

class InstructionPushBC : public Instruction
{
public:
    InstructionPushBC( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        push( m, r, r->BC );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "PUSH BC" );
    }
};

class InstructionAddA : public Instruction
{
public:
    InstructionAddA( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = add( r, r->A, m->rom[r->PC + 1] );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "ADD A, %#02x", mem->rom[addr + 1] );
    }
};

class InstructionRST00 : public Instruction
{
public:
    InstructionRST00( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        push( m, r, r->PC + length );
        r->PC = 0x00;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RST 0x00" );
    }
};

class InstructionRetZ : public Instruction
{
public:
    InstructionRetZ( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        if ( r->Flag.Z ) {
            r->PC = pop( m, r );
            return;
        }
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RET Z" );
    }
};

class InstructionRet : public Instruction
{
public:
    InstructionRet( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->PC = pop( m, r );
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RET" );
    }
};

class InstructionJpZu16 : public Instruction
{
public:
    InstructionJpZu16( void )
        : Instruction::Instruction( 3 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        if ( r->Flag.Z ) {
            r->PC = *(uint16_t*)&m->rom[r->PC + 1];
            return;
        }
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "JP Z, %#04x", *(uint16_t*)&mem->rom[addr + 1] );
    }
};


class InstructionCallZ : public Instruction
{
public:
    InstructionCallZ( void )
        : Instruction::Instruction( 3 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        if ( r->Flag.Z ) {
            push( m, r, r->PC + length );
            r->PC = *(uint16_t*)&m->rom[r->PC + 1];
            return;
        }
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "CALL Z, %#04x", *(uint16_t*)&mem->rom[addr + 1] );
    }
};

class InstructionCall : public Instruction
{
public:
    InstructionCall( void )
        : Instruction::Instruction( 3 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        uint16_t addr = *(uint16_t*)&m->rom[r->PC + 1];
        push( m, r, addr);
        r->PC = addr;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "CALL %#04x", *(uint16_t*)&mem->rom[addr + 1] );
    }
};

class InstructionAdcA : public Instruction
{
public:
    InstructionAdcA( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = adc( r, r->A, m->rom[r->PC + 1] );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "ADC A, %#02x", mem->rom[addr + 1] );
    }
};

class InstructionRST08 : public Instruction
{
public:
    InstructionRST08( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        // TODO
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RST 0x08" );
    }
};

class InstructionRetNC : public Instruction
{
public:
    InstructionRetNC( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        if ( !r->Flag.C ) {
            r->PC = pop( m, r );
            return;
        }
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RET NC" );
    }
};

class InstructionPopDE : public Instruction
{
public:
    InstructionPopDE( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->DE = pop( m, r );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "POP DE" );
    }
};

class InstructionJpNCu16 : public Instruction
{
public:
    InstructionJpNCu16( void )
        : Instruction::Instruction( 3 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        if ( !r->Flag.C ) {
            r->PC = *(uint16_t*)&m->rom[r->PC + 1];
            return;
        }
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "JP NC, %#04x", *(uint16_t*)&mem->rom[addr + 1] );
    }
};

class InstructionCallNC : public Instruction
{
public:
    InstructionCallNC( void )
        : Instruction::Instruction( 3 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        if ( !r->Flag.C ) {
            push( m, r, r->PC + length );
            r->PC = *(uint16_t*)&m->rom[r->PC + 1];
            return;
        }
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "CALL NC, %#04x", *(uint16_t*)&mem->rom[addr + 1] );
    }
};

class InstructionPushDE : public Instruction
{
public:
    InstructionPushDE( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        push( m, r, r->DE );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "PUSH DE" );
    }
};

class InstructionSubAu8 : public Instruction
{
public:
    InstructionSubAu8( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = sub( r, r->A, m->rom[r->PC + 1] );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SUB A, %#02x", mem->rom[addr + 1] );
    }
};

class InstructionRST10 : public Instruction
{
public:
    InstructionRST10( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        push( m, r, r->PC + length );
        r->PC = 0x10;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RST 0x10" );
    }
};

class InstructionRetC : public Instruction
{
public:
    InstructionRetC( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        if ( r->Flag.C ) {
            r->PC = pop( m, r );
            return;
        }
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RET C" );
    }
};

class InstructionRetI : public Instruction
{
public:
    InstructionRetI( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->PC = pop( m, r );
        m->ie = 1;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RETI" );
    }
};

class InstructionJpCu16 : public Instruction
{
public:
    InstructionJpCu16( void )
        : Instruction::Instruction( 3 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        if ( r->Flag.C ) {
            r->PC = *(uint16_t*)&m->rom[r->PC + 1];
            return;
        }
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "JP C, %#04x", *(uint16_t*)&mem->rom[addr + 1] );
    }
};

class InstructionCallC : public Instruction
{
public:
    InstructionCallC( void )
        : Instruction::Instruction( 3 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        if ( r->Flag.C ) {
            uint16_t addr = *(uint16_t*)&m->rom[r->PC + 1];
            push( m, r, addr );
            r->PC = addr;
            return;
        }
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "CALL C, %#04x", *(uint16_t*)&mem->rom[addr + 1] );
    }
};

class InstructionSbcA : public Instruction
{
public:
    InstructionSbcA( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = sbc( r, r->A, m->rom[r->PC + 1] );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SBC A, %#02x", mem->rom[addr + 1] );
    }
};

class InstructionRST18 : public Instruction
{
public:
    InstructionRST18( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        push( m, r, r->PC + length );
        r->PC = 0x18;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RST 0x18" );
    }
};

class InstructionLdFFA : public Instruction
{
public:
    InstructionLdFFA( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        m->rom[0xFF00 | m->rom[r->PC + 1]] = r->A;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD (%#04x), A", 0xFF00 | mem->rom[addr + 1] );
    }
};

class InstructionPopHL : public Instruction
{
public:
    InstructionPopHL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->HL = pop( m, r );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "POP HL" );
    }
};

class InstructionLdFFCA : public Instruction
{
public:
    InstructionLdFFCA( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        m->rom[0xFF00 | r->C] = r->A;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD (0xFF00 + C), A" );
    }
};

class InstructionPushHL : public Instruction
{
public:
    InstructionPushHL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        push( m, r, r->HL );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "PUSH HL" );
    }
};

class InstructionAddAu8 : public Instruction
{
public:
    InstructionAddAu8( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = add( r, r->A, m->rom[r->PC + 1] );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "ADD A, %#02x", mem->rom[addr + 1] );
    }
};

class InstructionRST20 : public Instruction
{
public:
    InstructionRST20( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        push( m, r, r->PC + length );
        r->PC = 0x20;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RST 0x20" );
    }
};

class InstructionAddSPi8 : public Instruction
{
public:
    InstructionAddSPi8( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->SP = add<uint16_t>( r, r->SP, (int8_t)m->rom[r->PC + 1] );
        r->Flag.Z = 0;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "ADD SP, %#04x", (uint16_t)(int8_t)mem->rom[addr + 1] );
    }
};

class InstructionJpHL : public Instruction
{
public:
    InstructionJpHL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->PC = r->HL;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "JP HL" );
    }
};

class InstructionLdu16A : public Instruction
{
public:
    InstructionLdu16A( void )
        : Instruction::Instruction( 3 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        m->rom[*(uint16_t*)&m->rom[r->PC + 1]] = r->A;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD (%#04x), A", *(uint16_t*)&mem->rom[addr + 1] );
    }
};

class InstructionXorA : public Instruction
{
public:
    InstructionXorA( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = _xor( r, r->A, m->rom[r->PC + 1] );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "XOR A, %#02x", mem->rom[addr + 1] );
    }
};

class InstructionRST28 : public Instruction
{
public:
    InstructionRST28( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        // TODO
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RST 0x28" );
    }
};

class InstructionLdAFF : public Instruction
{
public:
    InstructionLdAFF( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = m->rom[0xFF00 | m->rom[r->PC + 1]];
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD A, (%#04x)", 0xFF00 | mem->rom[addr + 1] );
    }
};

class InstructionPopAF : public Instruction
{
public:
    InstructionPopAF( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->AF = pop( m, r );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "POP AF" );
    }
};

class InstructionLdAFFC : public Instruction
{
public:
    InstructionLdAFFC( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = m->rom[0xFF00 | r->C];
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD A, (0xFF00 + C)" );
    }
};

class InstructionDI : public Instruction
{
public:
    InstructionDI( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        m->ie = false;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "DI" );
    }
};


class InstructionPushAF : public Instruction
{
public:
    InstructionPushAF( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        push( m, r, r->AF );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "PUSH AF" );
    }
};

class InstructionOrA : public Instruction
{
public:
    InstructionOrA( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = _or( r, r->A, m->rom[r->PC + 1] );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "OR A, %#02x", mem->rom[addr + 1] );
    }
};

class InstructionRST30 : public Instruction
{
public:
    InstructionRST30( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        // TODO
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RST 0x30" );
    }
};

class InstructionLdHLSPi8 : public Instruction
{
public:
    InstructionLdHLSPi8( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->HL = add< uint16_t >( r, r->HL, (int8_t)m->rom[r->PC + 1] );
        r->Flag.Z = 0;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD HL, SP + %#04x", (uint16_t)(int8_t)mem->rom[addr + 1] );
    }
};

class InstructionLdSPHL : public Instruction
{
public:
    InstructionLdSPHL( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->SP = r->HL;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD SP, HL" );
    }
};

class InstructionLdAu16 : public Instruction
{
public:
    InstructionLdAu16( void )
        : Instruction::Instruction( 3 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = m->rom[*(uint16_t*)&m->rom[r->PC + 1]];
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "LD A, (%#04x)", *(uint16_t*)&mem->rom[addr + 1] );
    }
};

class InstructionEI : public Instruction
{
public:
    InstructionEI( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        m->ie = true;
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "EI" );
    }
};

class InstructionCpA : public Instruction
{
public:
    InstructionCpA( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        sub( r, r->A, m->rom[r->PC + 1] );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "CP A, %#02x", mem->rom[addr + 1] );
    }
};

class InstructionRST38 : public Instruction
{
public:
    InstructionRST38( void )
        : Instruction::Instruction( 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        // TODO
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RST 0x38" );
    }
};


/* ===================================================================

                  Extended Instructions (0xCB)

   ================================================================ */ 



class InstructionEx : public Instruction
{
private:
    Instruction* opcode[0x100];

public:
    InstructionEx( void );

    virtual void execute( Memory* m, Registers* r )
    {
        opcode[m->rom[r->PC + 1]]->execute( m, r );
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        opcode[mem->rom[addr + 1]]->dis( dst, size, mem, addr );        
    }

    static uint8_t rlc( Registers* r, uint8_t operand )
    {
        r->Flag.N = r->Flag.H = 0;
        r->Flag.C = operand >> 7;

        uint8_t result = ( operand << 1 ) | r->Flag.C;

        r->Flag.Z = result == 0;

        return result;
    }

    static uint8_t rrc( Registers* r, uint8_t operand )
    {
        r->Flag.N = r->Flag.H = 0;
        r->Flag.C = operand & 1;

        uint8_t result = ( r->Flag.C << 7 ) | ( operand >> 1 );
        r->Flag.Z = result == 0;

        return result;
    }

    static uint8_t rl( Registers* r, uint8_t operand )
    {
        uint8_t result = operand << 1;

        r->Flag.Z = result == 0;
        r->Flag.N = r->Flag.H = 0;
        r->Flag.C = operand >> 7;

        return result;
    }

    static uint8_t rr( Registers* r, uint8_t operand )
    {
        uint8_t result = operand >> 1;

        r->Flag.Z = result == 0;
        r->Flag.N = r->Flag.H = 0;
        r->Flag.C = operand & 1;

        return result;
    }

    static uint8_t sla( Registers* r, uint8_t operand )
    {
        uint8_t result = operand << 1;

        r->Flag.Z = result == 0;
        r->Flag.N = r->Flag.H = 0;
        r->Flag.C = operand >> 7;

        return result;
    }

    static uint8_t sra( Registers* r, uint8_t operand )
    {
        uint8_t result = operand >> 1;

        r->Flag.Z = result == 0;
        r->Flag.N = r->Flag.H = 0;
        r->Flag.C = operand & 1;

        return ( result | (operand & 0x80 ) );
    }

    static uint8_t swap( Registers* r, uint8_t operand )
    {
        r->Flag.Z = operand == 0;
        r->Flag.C = r->Flag.N = r->Flag.H = 0;

        return ( ( operand << 4 ) | ( operand >> 4 ) );
    }

    static uint8_t srl( Registers* r, uint8_t operand )
    {
        uint8_t result = operand >> 1;

        r->Flag.Z = result == 0;
        r->Flag.N = r->Flag.H = 0;
        r->Flag.C = operand & 1;

        return result;
    }

    static bool bit( Registers* r, uint8_t reg, uint8_t bit )
    {
        r->Flag.N = 0;
        r->Flag.H = 1;
        r->Flag.Z = reg & ( 1 << bit ) ? 0 : 1;
        return r->Flag.Z == 0;
    }

    static void res( uint8_t& r, uint8_t bit )
    {
        r |= ( 1 << bit );
    }
};

class InstructionExRLCB : public Instruction
{
public:
    InstructionExRLCB( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->B = InstructionEx::rlc( r, r->B );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RLC B" );
    }
};

class InstructionExRLCC : public Instruction
{
public:
    InstructionExRLCC( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->C = InstructionEx::rlc( r, r->C );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RLC C" );
    }
};



class InstructionExRLCD : public Instruction
{
public:
    InstructionExRLCD( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->D = InstructionEx::rlc( r, r->D );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RLC D" );
    }
};



class InstructionExRLCE : public Instruction
{
public:
    InstructionExRLCE( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->E = InstructionEx::rlc( r, r->E );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RLC E" );
    }
};


class InstructionExRLCH : public Instruction
{
public:
    InstructionExRLCH( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->H = InstructionEx::rlc( r, r->H );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RLC H" );
    }
};



class InstructionExRLCL : public Instruction
{
public:
    InstructionExRLCL( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->L = InstructionEx::rlc( r, r->L );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RLC H" );
    }
};


class InstructionExRLCHL : public Instruction
{
public:
    InstructionExRLCHL( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        m->rom[r->HL] = InstructionEx::rlc( r, m->rom[r->HL] );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RLC (HL)" );
    }
};



class InstructionExRLCA : public Instruction
{
public:
    InstructionExRLCA( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = InstructionEx::rlc( r, r->A );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RLC A" );
    }
};


class InstructionExRRCB : public Instruction
{
public:
    InstructionExRRCB( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->B = InstructionEx::rrc( r, r->B );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RRC B" );
    }
};


class InstructionExRRCC : public Instruction
{
public:
    InstructionExRRCC( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->C = InstructionEx::rrc( r, r->C );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RRC C" );
    }
};

class InstructionExRRCD : public Instruction
{
public:
    InstructionExRRCD( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->D = InstructionEx::rrc( r, r->D );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RRC D" );
    }
};

class InstructionExRRCE : public Instruction
{
public:
    InstructionExRRCE( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->E = InstructionEx::rrc( r, r->E );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RRC E" );
    }
};

class InstructionExRRCH : public Instruction
{
public:
    InstructionExRRCH( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->H = InstructionEx::rrc( r, r->H );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RRC H" );
    }
};


class InstructionExRRCL : public Instruction
{
public:
    InstructionExRRCL( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->L = InstructionEx::rrc( r, r->L );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RRC L" );
    }
};


class InstructionExRRCHL : public Instruction
{
public:
    InstructionExRRCHL( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        m->rom[r->HL] = InstructionEx::rrc( r, m->rom[r->HL] );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RRC (HL)" );
    }
};

class InstructionExRRCA : public Instruction
{
public:
    InstructionExRRCA( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = InstructionEx::rrc( r, r->A );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RRC A" );
    }
};

class InstructionExRLB : public Instruction
{
public:
    InstructionExRLB( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->B = InstructionEx::rl( r, r->B );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RL B" );
    }
};

class InstructionExRLC : public Instruction
{
public:
    InstructionExRLC( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->C = InstructionEx::rl( r, r->C );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RL C" );
    }
};

class InstructionExRLD : public Instruction
{
public:
    InstructionExRLD( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->D = InstructionEx::rl( r, r->D );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RL D" );
    }
};

class InstructionExRLE : public Instruction
{
public:
    InstructionExRLE( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->E = InstructionEx::rl( r, r->E );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RL E" );
    }
};

class InstructionExRLH : public Instruction
{
public:
    InstructionExRLH( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->H = InstructionEx::rl( r, r->H );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RL H" );
    }
};

class InstructionExRLL : public Instruction
{
public:
    InstructionExRLL( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->L = InstructionEx::rl( r, r->L );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RL L" );
    }
};

class InstructionExRLHL : public Instruction
{
public:
    InstructionExRLHL( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        m->rom[r->HL] = InstructionEx::rl( r, m->rom[r->HL] );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RL (HL)" );
    }
};

class InstructionExRLA : public Instruction
{
public:
    InstructionExRLA( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = InstructionEx::rl( r, r->A );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RL A" );
    }
};

class InstructionExRRB : public Instruction
{
public:
    InstructionExRRB( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->B = InstructionEx::rr( r, r->B );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RR B" );
    }
};

class InstructionExRRC : public Instruction
{
public:
    InstructionExRRC( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->C = InstructionEx::rr( r, r->C );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RR C" );
    }
};

class InstructionExRRD : public Instruction
{
public:
    InstructionExRRD( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->D = InstructionEx::rr( r, r->D );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RR D" );
    }
};

class InstructionExRRE : public Instruction
{
public:
    InstructionExRRE( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->E = InstructionEx::rr( r, r->E );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RR E" );
    }
};

class InstructionExRRH : public Instruction
{
public:
    InstructionExRRH( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->H = InstructionEx::rr( r, r->H );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RR H" );
    }
};

class InstructionExRRL : public Instruction
{
public:
    InstructionExRRL( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->L = InstructionEx::rr( r, r->L );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RR L" );
    }
};

class InstructionExRRHL : public Instruction
{
public:
    InstructionExRRHL( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        m->rom[r->HL] = InstructionEx::rr( r, m->rom[r->HL] );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RR (HL)" );
    }
};

class InstructionExRRA : public Instruction
{
public:
    InstructionExRRA( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = InstructionEx::rr( r, r->A );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RR A" );
    }
};

class InstructionExSLAB : public Instruction
{
public:
    InstructionExSLAB( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->B = InstructionEx::sla( r, r->B );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SLA B" );
    }
};

class InstructionExSLAC : public Instruction
{
public:
    InstructionExSLAC( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->C = InstructionEx::sla( r, r->C );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SLA C" );
    }
};

class InstructionExSLAD : public Instruction
{
public:
    InstructionExSLAD( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->D = InstructionEx::sla( r, r->D );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SLA D" );
    }
};

class InstructionExSLAE : public Instruction
{
public:
    InstructionExSLAE( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->E = InstructionEx::sla( r, r->E );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SLA E" );
    }
};

class InstructionExSLAH : public Instruction
{
public:
    InstructionExSLAH( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->H = InstructionEx::sla( r, r->H );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SLA H" );
    }
};

class InstructionExSLAL : public Instruction
{
public:
    InstructionExSLAL( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->L = InstructionEx::sla( r, r->L );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SLA L" );
    }
};

class InstructionExSLAHL : public Instruction
{
public:
    InstructionExSLAHL( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        m->rom[r->HL] = InstructionEx::sla( r, m->rom[r->HL] );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SLA (HL)" );
    }
};

class InstructionExSLAA : public Instruction
{
public:
    InstructionExSLAA( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = InstructionEx::sla( r, r->A );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SLA A" );
    }
};

class InstructionExSRAB : public Instruction
{
public:
    InstructionExSRAB( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->B = InstructionEx::sra( r, r->B );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SRA B" );
    }
};

class InstructionExSRAC : public Instruction
{
public:
    InstructionExSRAC( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->C = InstructionEx::sra( r, r->C );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SRA C" );
    }
};

class InstructionExSRAD : public Instruction
{
public:
    InstructionExSRAD( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->D = InstructionEx::sra( r, r->D );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SRA D" );
    }
};

class InstructionExSRAE : public Instruction
{
public:
    InstructionExSRAE( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->E = InstructionEx::sra( r, r->E );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SRA E" );
    }
};

class InstructionExSRAH : public Instruction
{
public:
    InstructionExSRAH( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->H = InstructionEx::sra( r, r->H );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SRA H" );
    }
};

class InstructionExSRAL : public Instruction
{
public:
    InstructionExSRAL( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->L = InstructionEx::sra( r, r->L );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SRA L" );
    }
};

class InstructionExSRAHL : public Instruction
{
public:
    InstructionExSRAHL( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        m->rom[r->HL] = InstructionEx::sra( r, m->rom[r->HL] );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SRA (HL)" );
    }
};

class InstructionExSRAA : public Instruction
{
public:
    InstructionExSRAA( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = InstructionEx::sra( r, r->A );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SRA A" );
    }
};

class InstructionExSWAPB : public Instruction
{
public:
    InstructionExSWAPB( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->B = InstructionEx::swap( r, r->B );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SWAP B" );
    }
};

class InstructionExSWAPC : public Instruction
{
public:
    InstructionExSWAPC( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->C = InstructionEx::swap( r, r->C );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SWAP C" );
    }
};

class InstructionExSWAPD : public Instruction
{
public:
    InstructionExSWAPD( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->D = InstructionEx::swap( r, r->D );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SWAP D" );
    }
};

class InstructionExSWAPE : public Instruction
{
public:
    InstructionExSWAPE( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->E = InstructionEx::swap( r, r->E );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SWAP E" );
    }
};

class InstructionExSWAPH : public Instruction
{
public:
    InstructionExSWAPH( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->H = InstructionEx::swap( r, r->H );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SWAP H" );
    }
};

class InstructionExSWAPL : public Instruction
{
public:
    InstructionExSWAPL( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->L = InstructionEx::swap( r, r->L );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SWAP L" );
    }
};

class InstructionExSWAPHL : public Instruction
{
public:
    InstructionExSWAPHL( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        m->rom[r->HL] = InstructionEx::swap( r, m->rom[r->HL] );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SWAP (HL)" );
    }
};

class InstructionExSWAPA : public Instruction
{
public:
    InstructionExSWAPA( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = InstructionEx::swap( r, r->A );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SWAP A" );
    }
};

class InstructionExSRLB : public Instruction
{
public:
    InstructionExSRLB( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->B = InstructionEx::srl( r, r->B );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SRL B" );
    }
};

class InstructionExSRLC : public Instruction
{
public:
    InstructionExSRLC( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->C = InstructionEx::srl( r, r->C );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SRL C" );
    }
};

class InstructionExSRLD : public Instruction
{
public:
    InstructionExSRLD( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->D = InstructionEx::srl( r, r->D );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SRL D" );
    }
};


class InstructionExSRLE : public Instruction
{
public:
    InstructionExSRLE( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->E = InstructionEx::srl( r, r->E );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SRL E" );
    }
};

class InstructionExSRLH : public Instruction
{
public:
    InstructionExSRLH( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->H = InstructionEx::srl( r, r->H );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SRL H" );
    }
};

class InstructionExSRLL : public Instruction
{
public:
    InstructionExSRLL( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->L = InstructionEx::srl( r, r->L );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SRL L" );
    }
};

class InstructionExSRLHL : public Instruction
{
public:
    InstructionExSRLHL( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        m->rom[r->HL] = InstructionEx::srl( r, m->rom[r->HL] );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SRL (HL)" );
    }
};

class InstructionExSRLA : public Instruction
{
public:
    InstructionExSRLA( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->A = InstructionEx::srl( r, r->A );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "SRL A" );
    }
};


class InstructionExBit0B : public Instruction
{
public:
    InstructionExBit0B( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->B, 0 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 0, B" );
    }
};

class InstructionExBit0C : public Instruction
{
public:
    InstructionExBit0C( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->C, 0 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 0, C" );
    }
};

class InstructionExBit0D : public Instruction
{
public:
    InstructionExBit0D( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->D, 0 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 0, D" );
    }
};


class InstructionExBit0E : public Instruction
{
public:
    InstructionExBit0E( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->E, 0 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 0, E" );
    }
};


class InstructionExBit0H : public Instruction
{
public:
    InstructionExBit0H( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->H, 0 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 0, H" );
    }
};


class InstructionExBit0L : public Instruction
{
public:
    InstructionExBit0L( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->L, 0 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 0, L" );
    }
};

class InstructionExBit0HL : public Instruction
{
public:
    InstructionExBit0HL( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, m->rom[r->HL], 0 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 0, (HL)" );
    }
};

class InstructionExBit0A : public Instruction
{
public:
    InstructionExBit0A( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->A, 0 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 0, A" );
    }
};

class InstructionExBit1B : public Instruction
{
public:
    InstructionExBit1B( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->B, 1 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 1, B" );
    }
};

class InstructionExBit1C : public Instruction
{
public:
    InstructionExBit1C( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->C, 1 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 1, C" );
    }
};

class InstructionExBit1D : public Instruction
{
public:
    InstructionExBit1D( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->D, 1 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 1, D" );
    }
};

class InstructionExBit1E : public Instruction
{
public:
    InstructionExBit1E( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->E, 1 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 1, E" );
    }
};

class InstructionExBit1H : public Instruction
{
public:
    InstructionExBit1H( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->H, 1 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 1, H" );
    }
};

class InstructionExBit1L : public Instruction
{
public:
    InstructionExBit1L( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->L, 1 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 1, L" );
    }
};

class InstructionExBit1HL : public Instruction
{
public:
    InstructionExBit1HL( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, m->rom[r->HL], 1 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 1, (HL)" );
    }
};

class InstructionExBit1A : public Instruction
{
public:
    InstructionExBit1A( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->A, 1 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 1, A" );
    }
};

class InstructionExBit2B : public Instruction
{
public:
    InstructionExBit2B( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->B, 2 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 2, B" );
    }
};

class InstructionExBit2C : public Instruction
{
public:
    InstructionExBit2C( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->C, 2 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 2, C" );
    }
};

class InstructionExBit2D : public Instruction
{
public:
    InstructionExBit2D( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->D, 2 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 2, D" );
    }
};

class InstructionExBit2E : public Instruction
{
public:
    InstructionExBit2E( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->E, 2 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 2, E" );
    }
};

class InstructionExBit2H : public Instruction
{
public:
    InstructionExBit2H( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->H, 2 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 2, H" );
    }
};

class InstructionExBit2L : public Instruction
{
public:
    InstructionExBit2L( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->L, 2 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 2, L" );
    }
};

class InstructionExBit2HL : public Instruction
{
public:
    InstructionExBit2HL( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, m->rom[r->HL], 2 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 2, (HL)" );
    }
};

class InstructionExBit2A : public Instruction
{
public:
    InstructionExBit2A( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->A, 2 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 2, A" );
    }
};

class InstructionExBit3B : public Instruction
{
public:
    InstructionExBit3B( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->B, 3 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 3, B" );
    }
};

class InstructionExBit3C : public Instruction
{
public:
    InstructionExBit3C( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->C, 3 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 3, C" );
    }
};

class InstructionExBit3D : public Instruction
{
public:
    InstructionExBit3D( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->D, 3 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 3, D" );
    }
};

class InstructionExBit3E : public Instruction
{
public:
    InstructionExBit3E( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->E, 3 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 3, E" );
    }
};

class InstructionExBit3H : public Instruction
{
public:
    InstructionExBit3H( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->H, 3 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 3, H" );
    }
};

class InstructionExBit3L : public Instruction
{
public:
    InstructionExBit3L( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->L, 3 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 3, L" );
    }
};

class InstructionExBit3HL : public Instruction
{
public:
    InstructionExBit3HL( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, m->rom[r->HL], 3 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 3, (HL)" );
    }
};

class InstructionExBit3A : public Instruction
{
public:
    InstructionExBit3A( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->A, 3 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 3, A" );
    }
};

class InstructionExBit4B : public Instruction
{
public:
    InstructionExBit4B( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->B, 4 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 4, B" );
    }
};

class InstructionExBit4C : public Instruction
{
public:
    InstructionExBit4C( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->C, 4 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 4, C" );
    }
};

class InstructionExBit4D : public Instruction
{
public:
    InstructionExBit4D( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->D, 4 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 4, D" );
    }
};

class InstructionExBit4E : public Instruction
{
public:
    InstructionExBit4E( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->E, 4 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 4, E" );
    }
};

class InstructionExBit4H : public Instruction
{
public:
    InstructionExBit4H( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->H, 4 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 4, H" );
    }
};

class InstructionExBit4L : public Instruction
{
public:
    InstructionExBit4L( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->L, 4 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 4, L" );
    }
};

class InstructionExBit4HL : public Instruction
{
public:
    InstructionExBit4HL( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, m->rom[r->HL], 4 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 4, (HL)" );
    }
};

class InstructionExBit4A : public Instruction
{
public:
    InstructionExBit4A( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->A, 4 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 4, A" );
    }
};

class InstructionExBit5B : public Instruction
{
public:
    InstructionExBit5B( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->B, 5 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 5, B" );
    }
};

class InstructionExBit5C : public Instruction
{
public:
    InstructionExBit5C( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->C, 5 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 5, C" );
    }
};

class InstructionExBit5D : public Instruction
{
public:
    InstructionExBit5D( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->D, 5 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 5, D" );
    }
};

class InstructionExBit5E : public Instruction
{
public:
    InstructionExBit5E( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->E, 5 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 5, E" );
    }
};

class InstructionExBit5H : public Instruction
{
public:
    InstructionExBit5H( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->H, 5 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 5, H" );
    }
};

class InstructionExBit5L : public Instruction
{
public:
    InstructionExBit5L( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->L, 5 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 5, L" );
    }
};

class InstructionExBit5HL : public Instruction
{
public:
    InstructionExBit5HL( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, m->rom[r->HL], 5 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 5, (HL)" );
    }
};

class InstructionExBit5A : public Instruction
{
public:
    InstructionExBit5A( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->A, 5 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 5, A" );
    }
};

class InstructionExBit6B : public Instruction
{
public:
    InstructionExBit6B( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->B, 6 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 6, B" );
    }
};

class InstructionExBit6C : public Instruction
{
public:
    InstructionExBit6C( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->C, 6 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 6, C" );
    }
};

class InstructionExBit6D : public Instruction
{
public:
    InstructionExBit6D( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->D, 6 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 6, D" );
    }
};

class InstructionExBit6E : public Instruction
{
public:
    InstructionExBit6E( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->E, 6 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 6, E" );
    }
};

class InstructionExBit6H : public Instruction
{
public:
    InstructionExBit6H( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->H, 6 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 6, H" );
    }
};

class InstructionExBit6L : public Instruction
{
public:
    InstructionExBit6L( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->L, 6 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 6, L" );
    }
};

class InstructionExBit6HL : public Instruction
{
public:
    InstructionExBit6HL( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, m->rom[r->HL], 6 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 6, (HL)" );
    }
};

class InstructionExBit6A : public Instruction
{
public:
    InstructionExBit6A( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->A, 6 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 6, A" );
    }
};

class InstructionExBit7B : public Instruction
{
public:
    InstructionExBit7B( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->B, 7 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 7, B" );
    }
};

class InstructionExBit7C : public Instruction
{
public:
    InstructionExBit7C( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->C, 7 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 7, C" );
    }
};

class InstructionExBit7D : public Instruction
{
public:
    InstructionExBit7D( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->D, 7 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 7, D" );
    }
};

class InstructionExBit7E : public Instruction
{
public:
    InstructionExBit7E( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->E, 7 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 7, E" );
    }
};

class InstructionExBit7H : public Instruction
{
public:
    InstructionExBit7H( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->H, 7 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 7, H" );
    }
};

class InstructionExBit7L : public Instruction
{
public:
    InstructionExBit7L( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->L, 7 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 7, L" );
    }
};

class InstructionExBit7HL : public Instruction
{
public:
    InstructionExBit7HL( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, m->rom[r->HL], 7 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 7, (HL)" );
    }
};

class InstructionExBit7A : public Instruction
{
public:
    InstructionExBit7A( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::bit( r, r->A, 7 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "BIT 7, A" );
    }
};

class InstructionExRes0B : public Instruction
{
public:
    InstructionExRes0B( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::res( r->B, 0 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RES 0, B" );
    }
};


class InstructionExRes0C : public Instruction
{
public:
    InstructionExRes0C( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::res( r->C, 0 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RES 0, C" );
    }
};

class InstructionExRes0D : public Instruction
{
public:
    InstructionExRes0D( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::res( r->D, 0 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RES 0, D" );
    }
};

class InstructionExRes0E : public Instruction
{
public:
    InstructionExRes0E( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::res( r->E, 0 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RES 0, E" );
    }
};

class InstructionExRes0H : public Instruction
{
public:
    InstructionExRes0H( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::res( r->H, 0 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RES 0, H" );
    }
};

class InstructionExRes0L : public Instruction
{
public:
    InstructionExRes0L( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::res( r->L, 0 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RES 0, L" );
    }
};

class InstructionExRes0HL : public Instruction
{
public:
    InstructionExRes0HL( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::res( m->rom[r->HL], 0 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RES 0, (HL)" );
    }
};

class InstructionExRes0A : public Instruction
{
public:
    InstructionExRes0A( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::res( r->A, 0 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RES 0, A" );
    }
};

class InstructionExRes1B : public Instruction
{
public:
    InstructionExRes1B( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::res( r->B, 1 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RES 1, B" );
    }
};

class InstructionExRes1C : public Instruction
{
public:
    InstructionExRes1C( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::res( r->C, 1 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RES 1, C" );
    }
};

class InstructionExRes1D : public Instruction
{
public:
    InstructionExRes1D( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::res( r->D, 1 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RES 1, D" );
    }
};

class InstructionExRes1E : public Instruction
{
public:
    InstructionExRes1E( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::res( r->E, 1 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RES 1, E" );
    }
};

class InstructionExRes1H : public Instruction
{
public:
    InstructionExRes1H( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::res( r->H, 1 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RES 1, H" );
    }
};

class InstructionExRes1L : public Instruction
{
public:
    InstructionExRes1L( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::res( r->L, 1 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RES 1, L" );
    }
};

class InstructionExRes1HL : public Instruction
{
public:
    InstructionExRes1HL( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::res( m->rom[r->HL], 1 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RES 1, (HL)" );
    }
};

class InstructionExRes1A : public Instruction
{
public:
    InstructionExRes1A( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::res( r->A, 1 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RES 1, A" );
    }
};

class InstructionExRes2B : public Instruction
{
public:
    InstructionExRes2B( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::res( r->B, 2 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RES 2, B" );
    }
};

class InstructionExRes2C : public Instruction
{
public:
    InstructionExRes2C( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::res( r->C, 2 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RES 2, C" );
    }
};

class InstructionExRes2D : public Instruction
{
public:
    InstructionExRes2D( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::res( r->D, 2 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RES 2, D" );
    }
};

class InstructionExRes2E : public Instruction
{
public:
    InstructionExRes2E( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::res( r->E, 2 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RES 2, E" );
    }
};

class InstructionExRes2H : public Instruction
{
public:
    InstructionExRes2H( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::res( r->H, 2 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RES 2, H" );
    }
};

class InstructionExRes2L : public Instruction
{
public:
    InstructionExRes2L( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::res( r->L, 2 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RES 2, L" );
    }
};

class InstructionExRes2HL : public Instruction
{
public:
    InstructionExRes2HL( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::res( m->rom[r->HL], 2 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RES 2, (HL)" );
    }
};

class InstructionExRes2A : public Instruction
{
public:
    InstructionExRes2A( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::res( r->A, 2 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RES 2, A" );
    }
};

class InstructionExRes3B : public Instruction
{
public:
    InstructionExRes3B( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::res( r->B, 3 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RES 3, B" );
    }
};

class InstructionExRes3C : public Instruction
{
public:
    InstructionExRes3C( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::res( r->C, 3 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RES 3, C" );
    }
};

class InstructionExRes3D : public Instruction
{
public:
    InstructionExRes3D( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::res( r->D, 3 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RES 3, D" );
    }
};

class InstructionExRes3E : public Instruction
{
public:
    InstructionExRes3E( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::res( r->E, 3 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RES 3, E" );
    }
};

class InstructionExRes3H : public Instruction
{
public:
    InstructionExRes3H( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::res( r->H, 3 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RES 3, H" );
    }
};

class InstructionExRes3L : public Instruction
{
public:
    InstructionExRes3L( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::res( r->L, 3 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RES 3, L" );
    }
};

class InstructionExRes3HL : public Instruction
{
public:
    InstructionExRes3HL( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::res( m->rom[r->HL], 3 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RES 3, (HL)" );
    }
};

class InstructionExRes3A : public Instruction
{
public:
    InstructionExRes3A( void )
        : Instruction::Instruction( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        InstructionEx::res( r->A, 3 );
        r->PC += length;
    }

    virtual void dis( char* dst, size_t size, Memory* mem, uint16_t addr )
    {
        snprintf( dst, size, "RES 3, A" );
    }
};







InstructionEx::InstructionEx( void )
    : Instruction::Instruction( 2 )
{
    memset( opcode, 0, sizeof( opcode ) );

    opcode[0x00] = new InstructionExRLCB();
    opcode[0x01] = new InstructionExRLCC();
    opcode[0x02] = new InstructionExRLCD();
    opcode[0x03] = new InstructionExRLCE();
    opcode[0x04] = new InstructionExRLCH();
    opcode[0x05] = new InstructionExRLCL();
    opcode[0x06] = new InstructionExRLCHL();
    opcode[0x07] = new InstructionExRLCA();
    opcode[0x08] = new InstructionExRRCB();
    opcode[0x09] = new InstructionExRRCC();
    opcode[0x0A] = new InstructionExRRCD();
    opcode[0x0B] = new InstructionExRRCE();
    opcode[0x0C] = new InstructionExRRCH();
    opcode[0x0D] = new InstructionExRRCL();
    opcode[0x0E] = new InstructionExRLCHL();
    opcode[0x0F] = new InstructionExRRCA();

    opcode[0x10] = new InstructionExRLB();
    opcode[0x11] = new InstructionExRLC();
    opcode[0x12] = new InstructionExRLD();
    opcode[0x13] = new InstructionExRLE();
    opcode[0x14] = new InstructionExRLH();
    opcode[0x15] = new InstructionExRLL();
    opcode[0x16] = new InstructionExRLHL();
    opcode[0x17] = new InstructionExRLA();
    opcode[0x18] = new InstructionExRRB();
    opcode[0x19] = new InstructionExRRC();
    opcode[0x1A] = new InstructionExRRD();
    opcode[0x1B] = new InstructionExRRE();
    opcode[0x1C] = new InstructionExRRH();
    opcode[0x1D] = new InstructionExRRL();
    opcode[0x1E] = new InstructionExRRHL();
    opcode[0x1F] = new InstructionExRRA();

    opcode[0x20] = new InstructionExSLAB();
    opcode[0x21] = new InstructionExSLAC();
    opcode[0x22] = new InstructionExSLAD();
    opcode[0x23] = new InstructionExSLAE();
    opcode[0x24] = new InstructionExSLAH();
    opcode[0x25] = new InstructionExSLAL();
    opcode[0x26] = new InstructionExSLAHL();
    opcode[0x27] = new InstructionExSLAA();
    opcode[0x28] = new InstructionExSRAB();
    opcode[0x29] = new InstructionExSRAC();
    opcode[0x2A] = new InstructionExSRAD();
    opcode[0x2B] = new InstructionExSRAE();
    opcode[0x2C] = new InstructionExSRAH();
    opcode[0x2D] = new InstructionExSRAL();
    opcode[0x2E] = new InstructionExSRAHL();
    opcode[0x2F] = new InstructionExSRAA();

    opcode[0x30] = new InstructionExSWAPB();
    opcode[0x31] = new InstructionExSWAPC();
    opcode[0x32] = new InstructionExSWAPD();
    opcode[0x33] = new InstructionExSWAPE();
    opcode[0x34] = new InstructionExSWAPL();
    opcode[0x35] = new InstructionExSWAPH();
    opcode[0x36] = new InstructionExSWAPHL();
    opcode[0x37] = new InstructionExSWAPA();
    opcode[0x38] = new InstructionExSRLB();
    opcode[0x39] = new InstructionExSRLC();
    opcode[0x3A] = new InstructionExSRLD();
    opcode[0x3B] = new InstructionExSRLE();
    opcode[0x3C] = new InstructionExSRLH();
    opcode[0x3D] = new InstructionExSRLL();
    opcode[0x3E] = new InstructionExSRLHL();
    opcode[0x3F] = new InstructionExSRLA();

    opcode[0x40] = new InstructionExBit0B();
    opcode[0x41] = new InstructionExBit0C();
    opcode[0x42] = new InstructionExBit0D();
    opcode[0x43] = new InstructionExBit0E();
    opcode[0x44] = new InstructionExBit0H();
    opcode[0x45] = new InstructionExBit0L();
    opcode[0x46] = new InstructionExBit0HL();
    opcode[0x47] = new InstructionExBit0A();
    opcode[0x48] = new InstructionExBit1B();
    opcode[0x49] = new InstructionExBit1C();
    opcode[0x4A] = new InstructionExBit1D();
    opcode[0x4B] = new InstructionExBit1E();
    opcode[0x4C] = new InstructionExBit1H();
    opcode[0x4D] = new InstructionExBit1L();
    opcode[0x4E] = new InstructionExBit1HL();
    opcode[0x4F] = new InstructionExBit1A();

    opcode[0x50] = new InstructionExBit2B();
    opcode[0x51] = new InstructionExBit2C();
    opcode[0x52] = new InstructionExBit2D();
    opcode[0x53] = new InstructionExBit2E();
    opcode[0x54] = new InstructionExBit2H();
    opcode[0x55] = new InstructionExBit2L();
    opcode[0x56] = new InstructionExBit2HL();
    opcode[0x57] = new InstructionExBit2A();
    opcode[0x58] = new InstructionExBit3B();
    opcode[0x59] = new InstructionExBit3C();
    opcode[0x5A] = new InstructionExBit3D();
    opcode[0x5B] = new InstructionExBit3E();
    opcode[0x5C] = new InstructionExBit3H();
    opcode[0x5D] = new InstructionExBit3L();
    opcode[0x5E] = new InstructionExBit3HL();
    opcode[0x5F] = new InstructionExBit3A();

    opcode[0x60] = new InstructionExBit4B();
    opcode[0x61] = new InstructionExBit4C();
    opcode[0x62] = new InstructionExBit4D();
    opcode[0x63] = new InstructionExBit4E();
    opcode[0x64] = new InstructionExBit4H();
    opcode[0x65] = new InstructionExBit4L();
    opcode[0x66] = new InstructionExBit4HL();
    opcode[0x67] = new InstructionExBit4A();
    opcode[0x68] = new InstructionExBit5B();
    opcode[0x69] = new InstructionExBit5C();
    opcode[0x6A] = new InstructionExBit5D();
    opcode[0x6B] = new InstructionExBit5E();
    opcode[0x6C] = new InstructionExBit5H();
    opcode[0x6D] = new InstructionExBit5L();
    opcode[0x6E] = new InstructionExBit5HL();
    opcode[0x6F] = new InstructionExBit5A();

    opcode[0x70] = new InstructionExBit6B();
    opcode[0x71] = new InstructionExBit6C();
    opcode[0x72] = new InstructionExBit6D();
    opcode[0x73] = new InstructionExBit6E();
    opcode[0x74] = new InstructionExBit6H();
    opcode[0x75] = new InstructionExBit6L();
    opcode[0x76] = new InstructionExBit6HL();
    opcode[0x77] = new InstructionExBit6A();
    opcode[0x78] = new InstructionExBit7B();
    opcode[0x79] = new InstructionExBit7C();
    opcode[0x7A] = new InstructionExBit7D();
    opcode[0x7B] = new InstructionExBit7E();
    opcode[0x7C] = new InstructionExBit7H();
    opcode[0x7D] = new InstructionExBit7L();
    opcode[0x7E] = new InstructionExBit7HL();
    opcode[0x7F] = new InstructionExBit7A();

    opcode[0x80] = new InstructionExRes0B();
    opcode[0x81] = new InstructionExRes0C();
    opcode[0x82] = new InstructionExRes0D();
    opcode[0x83] = new InstructionExRes0E();
    opcode[0x84] = new InstructionExRes0H();
    opcode[0x85] = new InstructionExRes0L();
    opcode[0x86] = new InstructionExRes0HL();
    opcode[0x87] = new InstructionExRes0A();
    opcode[0x88] = new InstructionExRes1B();
    opcode[0x89] = new InstructionExRes1C();
    opcode[0x8A] = new InstructionExRes1D();
    opcode[0x8B] = new InstructionExRes1E();
    opcode[0x8C] = new InstructionExRes1H();
    opcode[0x8D] = new InstructionExRes1L();
    opcode[0x8E] = new InstructionExRes1HL();
    opcode[0x8F] = new InstructionExRes1A();

    opcode[0x90] = new InstructionExRes2B();
    opcode[0x91] = new InstructionExRes2C();
    opcode[0x92] = new InstructionExRes2D();
    opcode[0x93] = new InstructionExRes2E();
    opcode[0x94] = new InstructionExRes2H();
    opcode[0x95] = new InstructionExRes2L();
    opcode[0x96] = new InstructionExRes2HL();
    opcode[0x97] = new InstructionExRes2A();
    opcode[0x98] = new InstructionExRes3B();
    opcode[0x99] = new InstructionExRes3C();
    opcode[0x9A] = new InstructionExRes3D();
    opcode[0x9B] = new InstructionExRes3E();
    opcode[0x9C] = new InstructionExRes3H();
    opcode[0x9D] = new InstructionExRes3L();
    opcode[0x9E] = new InstructionExRes3HL();
    opcode[0x9F] = new InstructionExRes3A();



    for ( int i = 0; i < _countof( opcode ); i++ ) {
        if ( opcode[i] == nullptr ) {
            opcode[i] = new Instruction();
        }
    }
}
