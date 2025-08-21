#include "theme_detector.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#elif defined(__APPLE__)
    #include <cstdio>
#else
    #include <unistd.h>
    #include <pwd.h>
    #include <sys/types.h>
#endif

namespace ThemeDetector {

#ifdef _WIN32
Theme detectWindowsTheme() {
    try {
        HKEY hKey;
        DWORD dwType = REG_DWORD;
        DWORD dwData = 0;
        DWORD dwSize = sizeof(dwData);
        
        LONG result = RegOpenKeyExA(
            HKEY_CURRENT_USER,
            "Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
            0,
            KEY_READ,
            &hKey
        );
        
        if (result == ERROR_SUCCESS) {
            result = RegQueryValueExA(
                hKey,
                "AppsUseDarkTheme",
                nullptr,
                &dwType,
                reinterpret_cast<LPBYTE>(&dwData),
                &dwSize
            );
            
            RegCloseKey(hKey);
            
            if (result == ERROR_SUCCESS) {
                return dwData ? Theme::DARK : Theme::LIGHT;
            }
        }
        
    } catch (...) {
        std::cerr << "Error detecting Windows theme" << std::endl;
    }
    
    return Theme::LIGHT;
}
}

#elif defined(__APPLE__)
Theme detectMacOSTheme() {
    FILE* pipe = popen("defaults read -g AppleInterfaceStyle 2>/dev/null", "r");
    if (!pipe) {
        return Theme::UNKNOWN;
    }
    
    char buffer[256];
    std::string result = "";
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        result += buffer;
    }
    pclose(pipe);
    
    result.erase(result.find_last_not_of(" \n\r\t") + 1);
    
    if (result == "Dark") {
        return Theme::DARK;
    } else if (result.empty() || result.find("does not exist") != std::string::npos) {
        return Theme::LIGHT;
    }
    
    return Theme::LIGHT;
}

#else
Theme detectLinuxTheme() {
    try {
        const char* gtkTheme = getenv("GTK_THEME");
        if (gtkTheme) {
            std::string theme(gtkTheme);
            if (theme.find("dark") != std::string::npos || theme.find("Dark") != std::string::npos) {
                return Theme::DARK;
            }
        }
        
        const char* qtStyle = getenv("QT_STYLE_OVERRIDE");
        if (qtStyle) {
            std::string style(qtStyle);
            if (style.find("dark") != std::string::npos || style.find("Dark") != std::string::npos) {
                return Theme::DARK;
            }
        }
        
        FILE* pipe = popen("gsettings get org.gnome.desktop.interface gtk-theme 2>/dev/null", "r");
        if (pipe) {
            char buffer[256];
            if (fgets(buffer, sizeof(buffer), pipe)) {
                std::string theme(buffer);
                pclose(pipe);
                if (theme.find("dark") != std::string::npos || theme.find("Dark") != std::string::npos) {
                    return Theme::DARK;
                }
                if (theme.find("light") != std::string::npos || theme.find("Light") != std::string::npos) {
                    return Theme::LIGHT;
                }
            } else {
                pclose(pipe);
            }
        }
        
        const char* home = getenv("HOME");
        if (home) {
            std::string kdeglobals = std::string(home) + "/.config/kdeglobals";
            std::ifstream file(kdeglobals);
            if (file.is_open()) {
                std::string line;
                bool inColorsSection = false;
                
                while (std::getline(file, line)) {
                    if (line == "[Colors:Window]" || line == "[Colors:View]") {
                        inColorsSection = true;
                        continue;
                    }
                    
                    if (line.empty() || line[0] == '[') {
                        inColorsSection = false;
                        continue;
                    }
                    
                    if (inColorsSection && line.find("BackgroundNormal=") == 0) {
                        size_t pos = line.find('=');
                        if (pos != std::string::npos) {
                            std::string rgbValues = line.substr(pos + 1);
                            size_t comma1 = rgbValues.find(',');
                            if (comma1 != std::string::npos) {
                                int r = std::stoi(rgbValues.substr(0, comma1));
                                size_t comma2 = rgbValues.find(',', comma1 + 1);
                                if (comma2 != std::string::npos) {
                                    int g = std::stoi(rgbValues.substr(comma1 + 1, comma2 - comma1 - 1));
                                    int b = std::stoi(rgbValues.substr(comma2 + 1));
                                    
                                    double luminance = (0.299 * r + 0.587 * g + 0.114 * b) / 255.0;
                                    file.close();
                                    return luminance < 0.5 ? Theme::DARK : Theme::LIGHT;
                                }
                            }
                        }
                    }
                }
                file.close();
            }
        }
        
        if (home) {
            std::string xfceSettings = std::string(home) + "/.config/xfce4/xfconf/xfce-perchannel-xml/xsettings.xml";
            std::ifstream file(xfceSettings);
            if (file.is_open()) {
                std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                file.close();
                
                if (content.find("dark") != std::string::npos || content.find("Dark") != std::string::npos) {
                    return Theme::DARK;
                }
            }
        }
        
        const char* desktop = getenv("XDG_CURRENT_DESKTOP");
        if (desktop) {
            std::string de(desktop);
            return Theme::LIGHT;
        }
        
    } catch (...) {
        std::cerr << "Error detecting Linux theme" << std::endl;
    }
    
    return Theme::UNKNOWN;
}
#endif

Theme detectSystemTheme() {
#ifdef _WIN32
    return detectWindowsTheme();
#elif defined(__APPLE__)
    return detectMacOSTheme();
#else
    return detectLinuxTheme();
#endif
}

std::string getThemeAwareLogo(const std::string& basePath) {
    Theme theme = detectSystemTheme();
    
    std::string logoPath = basePath;
    if (!logoPath.empty() && logoPath.back() != '/') {
        logoPath += "/";
    }
    
    switch (theme) {
        case Theme::DARK:
            logoPath += "LogoWithBGDark.png";
            break;
        case Theme::LIGHT:
            logoPath += "LogoWithBGLight.png";
            break;
        case Theme::UNKNOWN:
        default:
            logoPath += "Logo.png";
            break;
    }
    
    std::cout << "Theme detected: " << themeToString(theme) << ", using logo: " << logoPath << std::endl;
    
    return logoPath;
}

std::string themeToString(Theme theme) {
    switch (theme) {
        case Theme::DARK: return "Dark";
        case Theme::LIGHT: return "Light";
        case Theme::UNKNOWN: return "Unknown";
        default: return "Invalid";
    }
}

}
