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

#include "registers.h"
#include "memory.h"

void wingb( GLFWwindow* window )
{

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

        ImGui::Indent( ImGui::GetWindowWidth() - ImGui::GetFontSize() * 7 );
        ImGui::Text( "(%.1f FPS)", ImGui::GetIO().Framerate );
        ImGui::Unindent();

        ImGui::EndMainMenuBar();
    }

    ImGui::Begin( "Main" );


    ImGui::End();
}
