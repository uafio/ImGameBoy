#pragma once
#include "memory.h"
#include "registers.h"

class Instruction
{
private:

public:
    const char* dis;
    const int length;

    Instruction( void )
        : dis( "Unknown instruction" ), length( 2 )
    {
    }

    virtual void execute( Memory* m, Registers* r )
    {
    }
};


