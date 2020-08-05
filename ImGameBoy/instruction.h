#pragma once
#include <stdint.h>
#include "memory.h"
#include "registers.h"

class Instruction
{
private:

public:
    const char* dis;
    int length;

    Instruction( void )
        : dis( "Unknown instruction" ), length( 1 )
    {
    }

    Instruction( const char* d, const int l )
    {
        dis = d;
        length = l;
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->PC += length;
    }
};


class InstructionNop : public Instruction
{
public:
    InstructionNop( void )
        : Instruction::Instruction( "NOP", 1 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->PC += length;
    }
};


class InstructionLdBC : public Instruction
{
public:
    InstructionLdBC( void )
        : Instruction::Instruction( "LD BC, u16", 3 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
        r->BC = *(uint16_t*)&m->map[r->PC + 1];
        r->PC += length;
    }
};
