#include <stdint.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <time.h>

#ifdef _WIN32
#include <Windows.h>
#include <commdlg.h>
#elif __gnu_linux__
#include <byteswap.h>
#endif

#include "imgui/imgui.h"
#include "glfw3.h"

#include "emulator.h"




void wingb( GLFWwindow* window )
{
    static Emulator* emu = new Emulator();
    static bool dbg = true;

    if ( ImGui::BeginMainMenuBar() ) {
        if ( ImGui::BeginMenu( "File" ) ) {
            if ( ImGui::MenuItem( "Open" ) ) {
#ifdef _WIN32
                OPENFILENAMEA ofile = { 0 };
                char fpath[_MAX_PATH] = { 0 };

                ofile.lStructSize = sizeof( ofile );
                ofile.hwndOwner = GetActiveWindow();
                ofile.lpstrFile = fpath;
                ofile.nMaxFile = sizeof( fpath );

                if ( GetOpenFileNameA( &ofile ) ) {
                }
#elif __gnu_linux__
                char fpath[1024];
                FILE* hFile = popen( "zenity --file-selection", "r" );
                fgets( fpath, sizeof( fpath ), hFile );
                if ( fpath[strlen( fpath ) - 1] == '\n' ) {
                    fpath[strlen( fpath ) - 1] = 0;
                }
#endif

            } else if ( ImGui::MenuItem( "Exit" ) ) {
                glfwSetWindowShouldClose( window, true );
            }

            ImGui::EndMenu();
        }

        if ( ImGui::BeginMenu( "Settings" ) ) {
            ImGui::Checkbox( "Debugger", &dbg );
            ImGui::EndMenu();
        }

        ImGui::Indent( ImGui::GetWindowWidth() - ImGui::GetFontSize() * 7 );
        ImGui::Text( "(%.1f FPS)", ImGui::GetIO().Framerate );
        ImGui::Unindent();

        ImGui::EndMainMenuBar();
    }

    if ( dbg ) {
        emu->debugger();    
    }

    for ( int cycle = 0; cycle < 70221; cycle += 8 ) {
        emu->step();
    }

    ImGui::Begin( "Main" );
    emu->draw();
    ImGui::End();
}
