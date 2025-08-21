#include "imgui_file_dialog.h"
#include <imgui.h>
#include <algorithm>
#include <iostream>

ImGuiFileDialog::ImGuiFileDialog() 
    : currentPath(std::filesystem::current_path())
    , selectedIndex(-1)
    , isDialogOpen(false)
    , fileSelected(false) {
    
    fileFilters = {".asm", ".s", ".txt"};
    
    std::string currentPathStr = currentPath.string();
    strncpy(pathBuffer, currentPathStr.c_str(), sizeof(pathBuffer) - 1);
    pathBuffer[sizeof(pathBuffer) - 1] = '\0';
    
    refreshCurrentDirectory();
}

void ImGuiFileDialog::open() {
    isDialogOpen = true;
    fileSelected = false;
    selectedFile.clear();
    selectedIndex = -1;
    refreshCurrentDirectory();
}

void ImGuiFileDialog::setFileFilters(const std::vector<std::string>& filters) {
    fileFilters = filters;
    refreshCurrentDirectory(); // Refresh to apply new filters
}

bool ImGuiFileDialog::display(const char* title) {
    if (!isDialogOpen) return false;

    fileSelected = false;
    
    // Center the modal
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(700, 500), ImGuiCond_FirstUseEver);

    if (ImGui::BeginPopupModal(title, &isDialogOpen, ImGuiWindowFlags_NoResize)) {
        
        // Current path display and navigation
        ImGui::Text("Current Directory:");
        ImGui::SameLine();
        
        // Path input field (editable)
        if (ImGui::InputText("##path", pathBuffer, sizeof(pathBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
            try {
                std::filesystem::path newPath(pathBuffer);
                if (std::filesystem::exists(newPath) && std::filesystem::is_directory(newPath)) {
                    navigateToDirectory(newPath);
                }
            } catch (const std::exception&) {
                // Invalid path, revert
                std::string currentPathStr = currentPath.string();
                strncpy(pathBuffer, currentPathStr.c_str(), sizeof(pathBuffer) - 1);
                pathBuffer[sizeof(pathBuffer) - 1] = '\0';
            }
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Up")) {
            if (currentPath.has_parent_path() && currentPath != currentPath.root_path()) {
                navigateToDirectory(currentPath.parent_path());
            }
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Home")) {
            navigateToDirectory(std::filesystem::path(std::getenv("HOME") ? std::getenv("HOME") : "."));
        }

        ImGui::Separator();

        // File list
        if (ImGui::BeginChild("FileList", ImVec2(0, -60), true)) {
            
            // Column headers
            ImGui::Columns(3, "FileColumns");
            ImGui::Text("Name");
            ImGui::NextColumn();
            ImGui::Text("Type");
            ImGui::NextColumn();
            ImGui::Text("Size");
            ImGui::NextColumn();
            ImGui::Separator();
            
            for (size_t i = 0; i < directoryContents.size(); ++i) {
                const auto& entry = directoryContents[i];
                
                bool isSelected = (static_cast<size_t>(selectedIndex) == i);
                
                // File/folder icon
                const char* icon = entry.isDirectory ? "📁" : "📄";
                std::string displayName = std::string(icon) + " " + entry.name;
                
                if (ImGui::Selectable(displayName.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns)) {
                    selectedIndex = i;
                    
                    if (entry.isDirectory) {
                        // Double-click to enter directory
                        if (ImGui::IsMouseDoubleClicked(0)) {
                            navigateToDirectory(std::filesystem::path(entry.fullPath));
                        }
                    } else {
                        selectedFile = entry.fullPath;
                    }
                }
                
                // Double-click to open file
                if (!entry.isDirectory && isSelected && ImGui::IsMouseDoubleClicked(0)) {
                    selectedFile = entry.fullPath;
                    fileSelected = true;
                    isDialogOpen = false;
                }
                
                ImGui::NextColumn();
                ImGui::Text(entry.isDirectory ? "Folder" : "File");
                ImGui::NextColumn();
                ImGui::Text("%s", entry.sizeString.c_str());
                ImGui::NextColumn();
            }
            
            ImGui::Columns(1);
        }
        ImGui::EndChild();

        ImGui::Separator();

        // File filters
        ImGui::Text("File Filters: ");
        ImGui::SameLine();
        std::string filterText = "";
        for (size_t i = 0; i < fileFilters.size(); ++i) {
            if (i > 0) filterText += ", ";
            filterText += "*" + fileFilters[i];
        }
        ImGui::Text("%s", filterText.c_str());

        // Buttons
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);
        
        bool hasFileSelected = (selectedIndex >= 0 && !directoryContents.empty() && 
                               !directoryContents[selectedIndex].isDirectory);
        
        if (!hasFileSelected) {
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
        }
        
        if (ImGui::Button("Open", ImVec2(100, 0)) && hasFileSelected) {
            selectedFile = directoryContents[selectedIndex].fullPath;
            fileSelected = true;
            isDialogOpen = false;
        }
        
        if (!hasFileSelected) {
            ImGui::PopStyleVar();
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(100, 0))) {
            isDialogOpen = false;
            selectedFile.clear();
        }

        ImGui::EndPopup();
    }

    return fileSelected;
}

void ImGuiFileDialog::refreshCurrentDirectory() {
    directoryContents.clear();
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(currentPath)) {
            FileEntry fileEntry;
            fileEntry.fullPath = entry.path().string();
            fileEntry.name = entry.path().filename().string();
            fileEntry.isDirectory = entry.is_directory();
            
            if (fileEntry.isDirectory || matchesFilter(entry.path())) {
                if (!fileEntry.isDirectory) {
                    try {
                        fileEntry.size = std::filesystem::file_size(entry.path());
                        fileEntry.sizeString = formatFileSize(fileEntry.size);
                    } catch (...) {
                        fileEntry.size = 0;
                        fileEntry.sizeString = "Unknown";
                    }
                } else {
                    fileEntry.size = 0;
                    fileEntry.sizeString = "";
                }
                
                directoryContents.push_back(fileEntry);
            }
        }
        
        // Sort: directories first, then files, alphabetically
        std::sort(directoryContents.begin(), directoryContents.end(), 
                  [](const FileEntry& a, const FileEntry& b) {
                      if (a.isDirectory != b.isDirectory) {
                          return a.isDirectory > b.isDirectory;
                      }
                      return a.name < b.name;
                  });
                  
    } catch (const std::exception& e) {
        std::cerr << "Error reading directory: " << e.what() << std::endl;
    }
}

void ImGuiFileDialog::navigateToDirectory(const std::filesystem::path& path) {
    try {
        currentPath = std::filesystem::canonical(path);
        
        // Update path buffer
        std::string currentPathStr = currentPath.string();
        strncpy(pathBuffer, currentPathStr.c_str(), sizeof(pathBuffer) - 1);
        pathBuffer[sizeof(pathBuffer) - 1] = '\0';
        
        selectedIndex = -1;
        refreshCurrentDirectory();
    } catch (const std::exception& e) {
        std::cerr << "Error navigating to directory: " << e.what() << std::endl;
    }
}

bool ImGuiFileDialog::matchesFilter(const std::filesystem::path& file) const {
    if (fileFilters.empty()) return true;
    
    std::string extension = file.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    for (const auto& filter : fileFilters) {
        std::string filterLower = filter;
        std::transform(filterLower.begin(), filterLower.end(), filterLower.begin(), ::tolower);
        
        if (extension == filterLower) {
            return true;
        }
    }
    
    return false;
}

std::string ImGuiFileDialog::formatFileSize(std::uintmax_t size) const {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double sizeFloat = static_cast<double>(size);
    
    while (sizeFloat >= 1024.0 && unitIndex < 4) {
        sizeFloat /= 1024.0;
        unitIndex++;
    }
    
    char buffer[32];
    if (unitIndex == 0) {
        snprintf(buffer, sizeof(buffer), "%d %s", static_cast<int>(sizeFloat), units[unitIndex]);
    } else {
        snprintf(buffer, sizeof(buffer), "%.1f %s", sizeFloat, units[unitIndex]);
    }
    
    return std::string(buffer);
}
