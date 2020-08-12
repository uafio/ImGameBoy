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

    uint8_t add( Registers* r, uint8_t rd, uint8_t rs )
    {
        uint8_t result = rd + rs;

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

    uint8_t _and( Registers * r, uint8_t rd, uint8_t rs )
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
        r->A = rlc( r, r->A );
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
        r->A = rrc( r, r->A );
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
        r->A = rl( r, r->A );
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
