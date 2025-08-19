#include "tui.h"

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>

#include <ncurses.h>

#include "emulator8086.h"

EmulatorTUI::EmulatorTUI(Emulator8086* emu) : emulator(emu) {
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

EmulatorTUI::~EmulatorTUI() {
    endwin();
}

void EmulatorTUI::drawRegisters(int y, int x, int w, int h) {
    auto& r = emulator->getRegisters();
    mvprintw(y++, x, "REGISTERS");
    mvprintw(y++, x, "AX=%04X (AH=%02X AL=%02X)", r.AX.x, r.AX.bytes.h, r.AX.bytes.l);
    mvprintw(y++, x, "BX=%04X (BH=%02X BL=%02X)", r.BX.x, r.BX.bytes.h, r.BX.bytes.l);
    mvprintw(y++, x, "CX=%04X (CH=%02X CL=%02X)", r.CX.x, r.CX.bytes.h, r.CX.bytes.l);
    mvprintw(y++, x, "DX=%04X (DH=%02X DL=%02X)", r.DX.x, r.DX.bytes.h, r.DX.bytes.l);
    mvprintw(y++, x, "SP=%04X BP=%04X", r.SP, r.BP);
    mvprintw(y++, x, "SI=%04X DI=%04X", r.SI, r.DI);
    mvprintw(y++, x, "CS=%04X DS=%04X", r.CS, r.DS);
    mvprintw(y++, x, "ES=%04X SS=%04X", r.ES, r.SS);
    mvprintw(y++, x, "IP=%04X FLAGS=%04X", r.IP, r.FLAGS);
    mvprintw(y, x, "Binary FLAGS: ");
    int offset = 14;
    for (int i = 15; i >= 0; i--) {
        mvprintw(y, x + offset++, "%d", (r.FLAGS >> i) & 1);
        if (i % 4 == 0 && i > 0) {
            mvprintw(y, x + offset++, " ");
        }
    }
    y++;
    mvprintw(y++, x, "FLAGS:");
    mvprintw(y++, x, "  CF=%d", !!(r.FLAGS & Registers::CF));
    mvprintw(y++, x, "  PF=%d", !!(r.FLAGS & Registers::PF));
    mvprintw(y++, x, "  AF=%d", !!(r.FLAGS & Registers::AF));
    mvprintw(y++, x, "  ZF=%d", !!(r.FLAGS & Registers::ZF));
    mvprintw(y++, x, "  SF=%d", !!(r.FLAGS & Registers::SF));
    mvprintw(y++, x, "  TF=%d", !!(r.FLAGS & Registers::TF));
    mvprintw(y++, x, "  IF=%d", !!(r.FLAGS & Registers::IF));
    mvprintw(y++, x, "  DF=%d", !!(r.FLAGS & Registers::DF));
    mvprintw(y++, x, "  OF=%d", !!(r.FLAGS & Registers::OF));
}

void EmulatorTUI::drawStack(int y, int x, int w, int h) {
    auto& r = emulator->getRegisters();
    mvprintw(y++, x, "STACK (SP=0x%04X)", r.SP);
    mvprintw(y++, x, "SS:SP = %04X:%04X", r.SS, r.SP);
    y++;

    uint16_t sp = r.SP;
    int displayed = 0;
    int maxLines = h - 3;
    for (int i = -16; i <= 16 && displayed < maxLines; i += 2) {
        uint16_t addr = sp + i;
        if (addr >= 0xFFFF)
            continue;

        try {
            uint16_t val = emulator->readMemoryWord(addr);
            if (i == 0) {
                attron(A_REVERSE);
                mvprintw(y + displayed, x, "SP-> %04X: %04X", addr, val);
                attroff(A_REVERSE);
            } else {
                mvprintw(y + displayed, x, "%+3d  %04X: %04X", i, addr, val);
            }
            displayed++;
        } catch (...) {
            break;
        }
    }
}

void EmulatorTUI::drawMemory(int y, int x, int w, int h) {
    mvprintw(y++,
             x,
             "MEMORY %04X..%04X (+/- PgUp/PgDn, arrows scroll)",
             memWindowStart,
             memWindowStart + memWindowSize);
    int bytesPerRow = w / 4;
    if (bytesPerRow < 8)
        bytesPerRow = 8;
    if (bytesPerRow > 16)
        bytesPerRow = 16;

    int maxRows = h - 1;
    int rows = std::min(maxRows, memWindowSize / bytesPerRow);

    for (int row = 0; row < rows; ++row) {
        int addr = memWindowStart + row * bytesPerRow;
        if (addr >= (int)emulator->getMemory().size())
            break;
        std::ostringstream oss;
        oss << std::hex << std::uppercase << std::setfill('0');
        oss << std::setw(4) << addr << ": ";
        for (int b = 0; b < bytesPerRow; ++b) {
            int a = addr + b;
            if (a >= (int)emulator->getMemory().size())
                break;
            uint8_t val = emulator->readMemoryByte(a);
            oss << std::setw(2) << (int)val << ' ';
        }
        mvprintw(y + row, x, "%s", oss.str().c_str());
    }
}

void EmulatorTUI::drawCode(int h, int w) {
    int x = 0, y = 0;
    mvprintw(y++, x, "CODE (F10 step, F5 run/stop, b breakpoint, c continue, l labels, q quit)");
    const auto& prog = emulator->getProgram();
    const auto& labels = emulator->getLabels();
    size_t ip = emulator->getIP();
    int linesAvail = h - 2;
    int first = 0;
    if ((int)ip > linesAvail / 2)
        first = ip - linesAvail / 2;
    for (int i = 0; i < linesAvail && first + i < (int)prog.size(); ++i) {
        size_t idx = first + i;
        bool isBP = breakpoints.count(idx);
        bool isIP = idx == ip;

        std::string labelPrefix = "";
        for (const auto& labelPair : labels) {
            if (labelPair.second == idx) {
                labelPrefix = labelPair.first + ": ";
                break;
            }
        }

        if (isIP && isBP)
            attron(COLOR_PAIR(3));
        else if (isIP)
            attron(COLOR_PAIR(1));
        else if (isBP)
            attron(COLOR_PAIR(2));
        mvprintw(y + i,
                 x,
                 "%c%04zu: %s%s",
                 isBP ? '*' : ' ',
                 idx,
                 labelPrefix.c_str(),
                 prog[idx].c_str());
        if (isIP && isBP)
            attroff(COLOR_PAIR(3));
        else if (isIP)
            attroff(COLOR_PAIR(1));
        else if (isBP)
            attroff(COLOR_PAIR(2));
    }
}

void EmulatorTUI::draw() {
    int h, w;
    getmaxyx(stdscr, h, w);
    erase();

    if (showLabels) {
        drawLabels(h, w);
    } else {
        int registerWidth = w / 4;
        int leftWidth = w - registerWidth;
        int stackWidth = leftWidth / 4;
        int editorWidth = leftWidth - stackWidth;
        int topHeight = h * 2 / 3;
        int memoryHeight = h - topHeight - 1;

        drawCode(topHeight, editorWidth);
        drawStack(0, editorWidth, stackWidth, topHeight);
        drawRegisters(0, leftWidth, registerWidth, h - 1);
        drawMemory(topHeight, 0, leftWidth, memoryHeight);
    }

    mvprintw(h - 1,
             0,
             "Mode: %s  IP=%zu  (help keys: h)",
             running ? "RUN" : "PAUSE",
             (size_t)emulator->getIP());
    refresh();
}

void EmulatorTUI::drawLabels(int h, int w) {
    int x = 0, y = 0;
    mvprintw(y++, x, "LABELS (press any key to return)");
    y++;

    const auto& labels = emulator->getLabels();
    if (labels.empty()) {
        mvprintw(y, x, "No labels defined in current program.");
        return;
    }

    mvprintw(y++, x, "Label Name                Address  Instruction");
    mvprintw(y++, x, "------------------------------------------------");

    std::vector<std::pair<std::string, size_t>> sortedLabels(labels.begin(), labels.end());
    std::sort(sortedLabels.begin(), sortedLabels.end(), [](const auto& a, const auto& b) {
        return a.second < b.second;
    });

    const auto& prog = emulator->getProgram();
    for (const auto& labelPair : sortedLabels) {
        if (y >= h - 2)
            break;

        std::string instruction = "";
        if (labelPair.second < prog.size()) {
            instruction = prog[labelPair.second];
            if (instruction.length() > 30)
                instruction = instruction.substr(0, 27) + "...";
        }

        mvprintw(y++,
                 x,
                 "%-20s  %04zu     %s",
                 labelPair.first.c_str(),
                 labelPair.second,
                 instruction.c_str());
    }
}

void EmulatorTUI::toggleBreakpoint() {
    size_t ip = emulator->getIP();
    if (breakpoints.count(ip))
        breakpoints.erase(ip);
    else
        breakpoints.insert(ip);
}

void EmulatorTUI::step() {
    emulator->step();
}

void EmulatorTUI::run() {
    while (!quit) {
        int ch = getch();
        if (ch != ERR) {
            switch (ch) {
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
                case 'l':
                case 'L':
                    showLabels = !showLabels;
                    break;
                case KEY_UP:
                    memWindowStart = std::max(0, memWindowStart - 16);
                    break;
                case KEY_DOWN:
                    memWindowStart = std::min((int)emulator->getMemory().size() - memWindowSize,
                                              memWindowStart + 16);
                    break;
                case KEY_PPAGE:
                    memWindowStart = std::max(0, memWindowStart - memWindowSize);
                    break;
                case KEY_NPAGE:
                    memWindowStart = std::min((int)emulator->getMemory().size() - memWindowSize,
                                              memWindowStart + memWindowSize);
                    break;
                default:
                    break;
            }
        }
        if (running) {
            if (breakpoints.count(emulator->getIP()) && ch != KEY_F(10)) {
                running = false;
            } else {
                bool cont = emulator->step();
                if (!cont)
                    running = false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        draw();
    }
}
