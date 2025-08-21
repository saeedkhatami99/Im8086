#ifndef PLATFORM_DIALOGS_H
#define PLATFORM_DIALOGS_H

#include <string>

namespace PlatformDialogs {
    /**
     * Opens a file dialog to select an assembly file
     * @return Selected file path, or empty string if cancelled
     */
    std::string openFileDialog();
}

#endif
