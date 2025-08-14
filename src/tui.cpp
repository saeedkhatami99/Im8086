#include "tui.h"
#include "emulator8086.h"
#include <ncurses.h>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>

EmulatorTUI::EmulatorTUI(Emulator8086 *emu) : emulator(emu)
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);
    start_color();
    use_default_colors();
    init_pair(1, COLOR_GREEN, -1);
    init_pair(2, COLOR_RED, -1);
    init_pair(3, COLOR_YELLOW, -1);
}

EmulatorTUI::~EmulatorTUI()
{
    endwin();
}

void EmulatorTUI::drawRegisters(int y, int x, int w)
{
    auto &r = emulator->getRegisters();
    mvprintw(y++, x, "REGISTERS");
    mvprintw(y++, x, "AX=%04X (AH=%02X AL=%02X)  BX=%04X (BH=%02X BL=%02X)", r.AX.x, r.AX.h, r.AX.l, r.BX.x, r.BX.h, r.BX.l);
    mvprintw(y++, x, "CX=%04X (CH=%02X CL=%02X)  DX=%04X (DH=%02X DL=%02X)", r.CX.x, r.CX.h, r.CX.l, r.DX.x, r.DX.h, r.DX.l);
    mvprintw(y++, x, "SI=%04X DI=%04X BP=%04X SP=%04X", r.SI, r.DI, r.BP, r.SP);
    mvprintw(y++, x, "CS=%04X DS=%04X ES=%04X SS=%04X", r.CS, r.DS, r.ES, r.SS);
    mvprintw(y++, x, "IP=%04X", r.IP);
    mvprintw(y++, x, "FLAGS=%04X [O=%d D=%d I=%d T=%d S=%d Z=%d A=%d P=%d C=%d]", r.FLAGS,
             !!(r.FLAGS & Registers::OF), !!(r.FLAGS & Registers::DF), !!(r.FLAGS & Registers::IF), !!(r.FLAGS & Registers::TF),
             !!(r.FLAGS & Registers::SF), !!(r.FLAGS & Registers::ZF), !!(r.FLAGS & Registers::AF), !!(r.FLAGS & Registers::PF), !!(r.FLAGS & Registers::CF));
}

void EmulatorTUI::drawStack(int y, int x, int w)
{
    auto &r = emulator->getRegisters();
    mvprintw(y++, x, "STACK (top 16 words)");
    uint16_t sp = r.SP;
    for (int i = 0; i < 16; ++i)
    {
        uint16_t addr = sp + i * 2;
        if (addr >= 0xFFFF)
            break;
        try
        {
            uint16_t val = emulator->readMemoryWord(addr);
            mvprintw(y + i, x, "%04X: %04X", addr, val);
        }
        catch (...)
        {
            break;
        }
    }
}

void EmulatorTUI::drawMemory(int y, int x, int w)
{
    mvprintw(y++, x, "MEMORY %04X..%04X (+/- PgUp/PgDn, arrows scroll)", memWindowStart, memWindowStart + memWindowSize);
    int bytesPerRow = 16;
    int rows = memWindowSize / bytesPerRow;
    for (int row = 0; row < rows; ++row)
    {
        int addr = memWindowStart + row * bytesPerRow;
        if (addr >= (int)emulator->getMemory().size())
            break;
        std::ostringstream oss;
        oss << std::hex << std::uppercase << std::setfill('0');
        oss << std::setw(4) << addr << ": ";
        for (int b = 0; b < bytesPerRow; ++b)
        {
            int a = addr + b;
            if (a >= (int)emulator->getMemory().size())
                break;
            uint8_t val = emulator->readMemoryByte(a);
            oss << std::setw(2) << (int)val << ' ';
        }
        mvprintw(y + row, x, "%s", oss.str().c_str());
    }
}

void EmulatorTUI::drawCode(int h, int w)
{
    int codeWidth = w / 2;
    int x = 0, y = 0;
    mvprintw(y++, x, "CODE (F10 step, F5 run/stop, b breakpoint, c continue, q quit)");
    const auto &prog = emulator->getProgram();
    size_t ip = emulator->getIP();
    int linesAvail = h - 2;
    int first = 0;
    if ((int)ip > linesAvail / 2)
        first = ip - linesAvail / 2;
    for (int i = 0; i < linesAvail && first + i < (int)prog.size(); ++i)
    {
        size_t idx = first + i;
        bool isBP = breakpoints.count(idx);
        bool isIP = idx == ip;
        if (isIP && isBP)
            attron(COLOR_PAIR(3));
        else if (isIP)
            attron(COLOR_PAIR(1));
        else if (isBP)
            attron(COLOR_PAIR(2));
        mvprintw(y + i, x, "%c%04zu: %s", isBP ? '*' : ' ', idx, prog[idx].c_str());
        if (isIP && isBP)
            attroff(COLOR_PAIR(3));
        else if (isIP)
            attroff(COLOR_PAIR(1));
        else if (isBP)
            attroff(COLOR_PAIR(2));
    }
}

void EmulatorTUI::drawInspector(int h, int w)
{
    int rightX = w / 2 + 1;
    int y = 0;
    drawRegisters(y, rightX, w / 2 - 1);
    y += 8;
    drawStack(y, rightX, w / 2 - 1);
    y += 18;
    drawMemory(y, rightX, w / 2 - 1);
}

void EmulatorTUI::draw()
{
    int h, w;
    getmaxyx(stdscr, h, w);
    erase();
    drawCode(h, w);
    drawInspector(h, w);
    mvprintw(h - 1, 0, "Mode: %s  IP=%zu  (help keys: h)", running ? "RUN" : "PAUSE", (size_t)emulator->getIP());
    refresh();
}

void EmulatorTUI::toggleBreakpoint()
{
    size_t ip = emulator->getIP();
    if (breakpoints.count(ip))
        breakpoints.erase(ip);
    else
        breakpoints.insert(ip);
}

void EmulatorTUI::step()
{
    emulator->step();
}

void EmulatorTUI::run()
{
    while (!quit)
    {
        int ch = getch();
        if (ch != ERR)
        {
            switch (ch)
            {
            case KEY_F(5):
                running = !running;
                break;
            case KEY_F(10):
                running = false;
                step();
                break;
            case 'q':
            case 'Q':
                quit = true;
                break;
            case 'b':
            case 'B':
                toggleBreakpoint();
                break;
            case 'c':
            case 'C':
                running = true;
                break;
            case KEY_UP:
                memWindowStart = std::max(0, memWindowStart - 16);
                break;
            case KEY_DOWN:
                memWindowStart = std::min((int)emulator->getMemory().size() - memWindowSize, memWindowStart + 16);
                break;
            case KEY_PPAGE:
                memWindowStart = std::max(0, memWindowStart - memWindowSize);
                break;
            case KEY_NPAGE:
                memWindowStart = std::min((int)emulator->getMemory().size() - memWindowSize, memWindowStart + memWindowSize);
                break;
            default:
                break;
            }
        }
        if (running)
        {
            if (breakpoints.count(emulator->getIP()) && ch != KEY_F(10))
            {
                running = false;
            }
            else
            {
                bool cont = emulator->step();
                if (!cont)
                    running = false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        draw();
    }
}
