#include "platform_dialogs.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <commdlg.h>
#include <cstring>

namespace PlatformDialogs {
    std::string openFileDialog() {
        char filename[MAX_PATH] = {0};
        
        OPENFILENAMEA ofn;
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL;
        ofn.lpstrFile = filename;
        ofn.nMaxFile = sizeof(filename);
        ofn.lpstrFilter = "Assembly Files\0*.asm;*.s;*.txt\0All Files\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
        
        if (GetOpenFileNameA(&ofn)) {
            return std::string(filename);
        }
        
        return "";
    }
}

#else
#include <cstdlib>
#include <cstdio>

namespace PlatformDialogs {
    std::string openFileDialog() {
        std::string command;
        std::string filters = "*.asm *.s *.txt";
        
        #ifdef __APPLE__
        command = "osascript -e 'tell application \"System Events\" to activate' -e 'tell application \"System Events\" to set theFile to choose file with prompt \"Select Assembly File\" of type {\"public.plain-text\", \"public.assembly-source\"}' -e 'POSIX path of theFile'";
        #else
        command = "kdialog --getopenfilename . \"" + filters + "|Assembly files\"";
        #endif
        
        FILE* pipe = popen(command.c_str(), "r");
        if (!pipe) return "";
        
        char buffer[1024];
        std::string result = "";
        while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            result += buffer;
        }
        pclose(pipe);
        
        if (!result.empty() && result.back() == '\n') {
            result.pop_back();
        }
        
        return result;
    }
}
#endif
