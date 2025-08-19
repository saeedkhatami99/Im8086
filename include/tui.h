#ifndef IM8086_TUI_H
#define IM8086_TUI_H

#include <cstdint>
#include <set>
#include <string>
#include <vector>

class Emulator8086;

class EmulatorTUI {
  public:
    explicit EmulatorTUI(Emulator8086* emu);
    ~EmulatorTUI();
    void run();

  private:
    Emulator8086* emulator;
    bool running = false;
    bool quit = false;
    int memWindowStart = 0;
    int memWindowSize = 128;
    std::set<size_t> breakpoints;
    int selectedPane = 0;
    bool showLabels = false;

    void draw();
    void drawCode(int h, int w);
    void drawRegisters(int starty, int startx, int w, int h);
    void drawStack(int starty, int startx, int w, int h);
    void drawMemory(int starty, int startx, int w, int h);
    void drawLabels(int h, int w);
    void toggleBreakpoint();
    void step();
};

#endif
