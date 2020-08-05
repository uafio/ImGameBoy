#pragma once
#include "imgui/imgui.h"
#include "imgui/imgui_memory_editor.h"
#include "memory.h"
#include "registers.h"
#include "instruction.h"

enum class StepState
{
    STOP = 0,
    RUN,
    STEP,
};

class Debugger
{
private:
    MemoryEditor mViewer;

public:
    StepState sstate;
    Debugger( void )
    {
    }

    ~Debugger( void )
    {
    }

    void show_memory( Memory* mem )
    {
        mViewer.DrawWindow( "Memory", mem->map, sizeof( mem->map ) );
    }

    void show_registers( Registers* r )
    {
        ImGui::Begin( "Registers" );
        ImGui::Indent( 10.0f );
        ImGui::AlignTextToFramePadding();
        ImGui::PushItemWidth( 30.0f );

        ImGui::Text( "A" );
        ImGui::SameLine();
        ImGui::InputScalar( "##A", ImGuiDataType_U8, &r->A, nullptr, nullptr, "%02X", ImGuiInputTextFlags_CharsHexadecimal );
        ImGui::SameLine();
        ImGui::Text( "BC" );
        ImGui::SameLine();
        ImGui::PushItemWidth( 40.0f );
        ImGui::InputScalar( "##BC", ImGuiDataType_U16, &r->BC, nullptr, nullptr, "%X", ImGuiInputTextFlags_CharsHexadecimal );
        ImGui::PopItemWidth();

        ImGui::Text( "B" );
        ImGui::SameLine();
        ImGui::InputScalar( "##B", ImGuiDataType_U8, &r->B, nullptr, nullptr, "%02X", ImGuiInputTextFlags_CharsHexadecimal );
        ImGui::SameLine();
        ImGui::Text( "DE" );
        ImGui::SameLine();
        ImGui::PushItemWidth( 40.0f );
        ImGui::InputScalar( "##DE", ImGuiDataType_U16, &r->DE, nullptr, nullptr, "%X", ImGuiInputTextFlags_CharsHexadecimal );
        ImGui::PopItemWidth();

        ImGui::Text( "C" );
        ImGui::SameLine();
        ImGui::InputScalar( "##C", ImGuiDataType_U8, &r->C, nullptr, nullptr, "%02X", ImGuiInputTextFlags_CharsHexadecimal );
        ImGui::SameLine();
        ImGui::Text( "HL" );
        ImGui::SameLine();
        ImGui::PushItemWidth( 40.0f );
        ImGui::InputScalar( "##HL", ImGuiDataType_U16, &r->HL, nullptr, nullptr, "%X", ImGuiInputTextFlags_CharsHexadecimal );
        ImGui::PopItemWidth();

        ImGui::Text( "D" );
        ImGui::SameLine();
        ImGui::InputScalar( "##D", ImGuiDataType_U8, &r->D, nullptr, nullptr, "%02X", ImGuiInputTextFlags_CharsHexadecimal );

        ImGui::Text( "E" );
        ImGui::SameLine();
        ImGui::InputScalar( "##E", ImGuiDataType_U8, &r->E, nullptr, nullptr, "%02X", ImGuiInputTextFlags_CharsHexadecimal );

        ImGui::Text( "H" );
        ImGui::SameLine();
        ImGui::InputScalar( "##H", ImGuiDataType_U8, &r->H, nullptr, nullptr, "%02X", ImGuiInputTextFlags_CharsHexadecimal );

        ImGui::Text( "L" );
        ImGui::SameLine();
        ImGui::InputScalar( "##L", ImGuiDataType_U8, &r->L, nullptr, nullptr, "%02X", ImGuiInputTextFlags_CharsHexadecimal );
        ImGui::SameLine();
        ImGui::Text( "SP" );
        ImGui::SameLine();
        ImGui::PushItemWidth( 40.0f );
        ImGui::InputScalar( "##SP", ImGuiDataType_U16, &r->SP, nullptr, nullptr, "%X", ImGuiInputTextFlags_CharsHexadecimal );

        ImGui::NewLine();
        ImGui::Indent( -7.0f );
        ImGui::Text( "PC" );
        ImGui::SameLine();
        ImGui::InputScalar( "##PC", ImGuiDataType_U16, &r->PC, nullptr, nullptr, "%X", ImGuiInputTextFlags_CharsHexadecimal );
        ImGui::PopItemWidth();
        ImGui::PopItemWidth();

        ImGui::Text( "FLAGS: %X", r->F );

        if ( r->Flag.Z ) {
            ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.0f, 1.0f, 0.85f, 1.0f ) );
        } else {
            ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.0f, 0.0f, 0.0f, 1.0f ) );
        }
        ImGui::Text( "Z %d ", r->Flag.Z );
        if ( ImGui::IsItemClicked( ImGuiMouseButton_Left ) ) {
            r->Flag.Z ^= 1;
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();


        if ( r->Flag.N ) {
            ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.0f, 1.0f, 0.85f, 1.0f ) );
        } else {
            ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.0f, 0.0f, 0.0f, 1.0f ) );
        }
        ImGui::Text( "N %d ", r->Flag.N );
        if ( ImGui::IsItemClicked( ImGuiMouseButton_Left ) ) {
            r->Flag.N ^= 1;
        }
        ImGui::PopStyleColor();

        ImGui::SameLine();
        if ( r->Flag.H ) {
            ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.0f, 1.0f, 0.85f, 1.0f ) );
        } else {
            ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.0f, 0.0f, 0.0f, 1.0f ) );
        }
        ImGui::Text( "H %d ", r->Flag.H );
        if ( ImGui::IsItemClicked( ImGuiMouseButton_Left ) ) {
            r->Flag.H ^= 1;
        }
        ImGui::PopStyleColor();        
        
        ImGui::SameLine();
        if ( r->Flag.C ) {
            ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.0f, 1.0f, 0.85f, 1.0f ) );
        } else {
            ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.0f, 0.0f, 0.0f, 1.0f ) );
        }
        ImGui::Text( "C %d ", r->Flag.C );
        if ( ImGui::IsItemClicked( ImGuiMouseButton_Left ) ) {
            r->Flag.C ^= 1;
        }
        ImGui::PopStyleColor();   


        if ( ImGui::Button( "Run" ) ) {
            sstate = StepState::RUN;
        }
        ImGui::SameLine();
        if ( ImGui::Button( "Step" ) ) {
            sstate = StepState::STEP;
        }
        ImGui::SameLine();
        if ( ImGui::Button( "Break" ) ) {
            sstate = StepState::STOP;
        }


        ImGui::Unindent();
        ImGui::End();
    }

    void show_disassembly( Memory* mem, Registers* r, Instruction** op )
    {
        ImGui::Begin( "Disassembly" );

        // TODO: Implement ImGui List Clipper
        for ( uint16_t i = 0; i < _countof( mem->rom ); ) {
            Instruction* cur = op[mem->rom[i]];
            if ( r->PC == i ) {
                ImGui::Text( "> %04X: %s", i, cur->dis );
            } else {
                ImGui::Text( "  %04X: %s", i, cur->dis );
            }
            i += cur->length;
        }


        ImGui::End();
    }

};