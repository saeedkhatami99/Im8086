#ifndef IMGUI_FILE_DIALOG_H
#define IMGUI_FILE_DIALOG_H

#include <string>
#include <vector>
#include <filesystem>

class ImGuiFileDialog {
public:
    ImGuiFileDialog();
    ~ImGuiFileDialog() = default;

    /**
     * Display the file dialog. Call this in your ImGui render loop.
     * @return true if a file was selected, false if cancelled or still open
     */
    bool display(const char* title = "Open File");

    /**
     * Get the selected file path (only valid when display() returns true)
     */
    const std::string& getSelectedFile() const { return selectedFile; }

    /**
     * Check if the dialog is currently open
     */
    bool isOpen() const { return isDialogOpen; }

    /**
     * Programmatically open the dialog
     */
    void open();

    /**
     * Set file filters (e.g., {".asm", ".s", ".txt"})
     */
    void setFileFilters(const std::vector<std::string>& filters);

private:
    void refreshCurrentDirectory();
    void navigateToDirectory(const std::filesystem::path& path);
    bool matchesFilter(const std::filesystem::path& file) const;
    std::string formatFileSize(std::uintmax_t size) const;

    struct FileEntry {
        std::string name;
        std::string fullPath;
        bool isDirectory;
        std::uintmax_t size;
        std::string sizeString;
    };

    std::filesystem::path currentPath;
    std::vector<FileEntry> directoryContents;
    std::vector<std::string> fileFilters;
    std::string selectedFile;
    int selectedIndex;
    bool isDialogOpen;
    bool fileSelected;
    char pathBuffer[512];
};

#endif // IMGUI_FILE_DIALOG_H
