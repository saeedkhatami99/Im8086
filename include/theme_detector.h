#ifndef THEME_DETECTOR_H
#define THEME_DETECTOR_H

#include <string>

namespace ThemeDetector {
    
    enum class Theme {
        LIGHT,
        DARK,
        UNKNOWN
    };
    
    /**
     * Detects the current system theme across all platforms
     * @return Theme::DARK if dark theme is detected, Theme::LIGHT if light theme, Theme::UNKNOWN if detection fails
     */
    Theme detectSystemTheme();
    
    /**
     * Gets the appropriate logo path based on system theme
     * @param basePath Base path to the resources directory
     * @return Full path to the appropriate logo file
     */
    std::string getThemeAwareLogo(const std::string& basePath = "resources");
    
    /**
     * Converts theme enum to string for debugging
     */
    std::string themeToString(Theme theme);
    
}

#endif
