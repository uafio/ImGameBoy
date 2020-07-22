#pragma once
#include <stdint.h>
#include <fstream>


enum class CartridgeType
{
    ROMONLY = 0,
};



class Cartridge
{
private:

protected:
    CartridgeType type;
};



class CartridgeRomOnly : public Cartridge
{
private:
    CartridgeRomOnly( void )
    {
        type = CartridgeType::ROMONLY;
        memset( rom, 0, sizeof( rom ) ); // doc says ROM is not initialized to 0 but we will for now
    }

protected:
    uint8_t rom[0x7fff];

public:
    CartridgeRomOnly( void* data )
        : CartridgeRomOnly()
    {
    }
};



class CartridgeFactory : private Cartridge
{
private:
    void* get_file_content( const char* fname )
    {
        std::fstream infile( fname, std::fstream::in | std::fstream::binary | std::fstream::ate );
        if ( infile.is_open() == false ) {
            std::perror( __FUNCTION__ );
            return nullptr;
        }

        size_t size = infile.tellg();
        void* data = malloc( size );

        infile.seekg( std::fstream::beg );

        infile.read( reinterpret_cast< char* >( data ), size );

        // TODO: Verify Cartridge header

        return data;
    }

    CartridgeType get_cartridge_type( void* cartridge )
    {
        return static_cast< CartridgeType >( reinterpret_cast< uint8_t* >( cartridge )[0x147] );
    }


public:
    Cartridge* CreateCartridge( const char* fname )
    {
        void* data = get_file_content( fname );
        if ( data == nullptr ) {
            return nullptr;
        }

        if ( get_cartridge_type( data ) == CartridgeType::ROMONLY ) {
            return new CartridgeRomOnly( data );
        }
    }
};

