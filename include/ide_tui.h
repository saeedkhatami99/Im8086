#ifndef IM8086_IDE_TUI_H
#define IM8086_IDE_TUI_H

#include <vector>
#include <string>
#include <set>
#include <cstdint>

class Emulator8086;

class EmulatorIDETUI
{
public:
    explicit EmulatorIDETUI(Emulator8086 *emu);
    ~EmulatorIDETUI();
    void run();

private:
    Emulator8086 *emulator;
    bool running = false;
    bool quit = false;
    int memWindowStart = 0;
    int memWindowSize = 128;
    std::set<size_t> breakpoints;
    int selectedPane = 0;
    bool showLabels = false;
    bool inEditMode = true;
    int currentMode = 0;

    std::vector<std::string> editorLines;
    int cursorRow = 0;
    int cursorCol = 0;
    int editorScrollY = 0;
    int editorScrollX = 0;

    std::string statusMessage = "IDE Mode - Press F1 for help";

    void draw();
    void drawEditor(int h, int w);
    void drawDebugger(int h, int w);
    void drawCode(int h, int w);
    void drawInspector(int h, int w);
    void drawRegisters(int starty, int startx, int w);
    void drawStack(int starty, int startx, int w);
    void drawMemory(int starty, int startx, int w);
    void drawLabels(int h, int w);
    void drawStatus(int h, int w);
    void drawHelp(int h, int w);

    void toggleBreakpoint();
    void step();
    void compileAndLoad();
    void newProgram();
    void saveProgram();
    void loadProgram();

    void handleEditorInput(int ch);
    void handleDebuggerInput(int ch);
    void insertChar(char ch);
    void deleteChar();
    void newLine();
    void moveCursor(int dy, int dx);
    void ensureCursorVisible(int h, int w);

    void setStatus(const std::string &msg);
    std::string getCurrentLine();
    void setCurrentLine(const std::string &line);
};

#endif
