#include "ide_tui.h"
#include "emulator8086.h"
#include <ncurses.h>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <fstream>
#include <cctype>

EmulatorIDETUI::EmulatorIDETUI(Emulator8086 *emu) : emulator(emu)
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(1);
    start_color();
    use_default_colors();
    init_pair(1, COLOR_GREEN, -1);
    init_pair(2, COLOR_RED, -1);
    init_pair(3, COLOR_YELLOW, -1);
    init_pair(4, COLOR_CYAN, -1);
    init_pair(5, COLOR_MAGENTA, -1);
    init_pair(6, COLOR_WHITE, COLOR_BLUE);

    editorLines.push_back("; 8086 Assembly Program");
    editorLines.push_back("MOV AX, 10h      ; Load 16 into AX");
    editorLines.push_back("MOV BX, 20h      ; Load 32 into BX");
    editorLines.push_back("ADD AX, BX       ; Add BX to AX");
    editorLines.push_back("HLT              ; Halt");
    editorLines.push_back("");
}

EmulatorIDETUI::~EmulatorIDETUI()
{
    endwin();
}

void EmulatorIDETUI::draw()
{
    int h, w;
    getmaxyx(stdscr, h, w);
    erase();

    if (currentMode == 0)
    {
        drawEditor(h, w);
    }
    else
    {
        drawDebugger(h, w);
    }

    drawStatus(h, w);
    refresh();
}

void EmulatorIDETUI::drawEditor(int h, int w)
{
    int editorHeight = h - 2;

    attron(COLOR_PAIR(4));
    mvprintw(0, 0, "%-*s", w, "8086 Assembly IDE - EDITOR MODE");
    attroff(COLOR_PAIR(4));

    int startRow = editorScrollY;
    int endRow = std::min((int)editorLines.size(), startRow + editorHeight - 1);

    for (int i = 0; i < editorHeight - 1; ++i)
    {
        int lineNum = startRow + i;
        int screenY = i + 1;

        if (lineNum < (int)editorLines.size())
        {

            if (lineNum == cursorRow)
            {
                attron(COLOR_PAIR(6));
                mvprintw(screenY, 0, "%-*s", w, "");
                attroff(COLOR_PAIR(6));
            }

            mvprintw(screenY, 0, "%4d ", lineNum + 1);

            std::string line = editorLines[lineNum];
            if (editorScrollX < (int)line.length())
            {
                std::string visible = line.substr(editorScrollX);
                if (visible.length() > (size_t)(w - 6))
                    visible = visible.substr(0, w - 6);
                mvprintw(screenY, 5, "%s", visible.c_str());
            }
        }
        else
        {
            mvprintw(screenY, 0, "~");
        }
    }

    int screenCursorY = cursorRow - editorScrollY + 1;
    int screenCursorX = cursorCol - editorScrollX + 5;
    if (screenCursorY >= 1 && screenCursorY < editorHeight &&
        screenCursorX >= 5 && screenCursorX < w)
    {
        move(screenCursorY, screenCursorX);
    }
}

void EmulatorIDETUI::drawDebugger(int h, int w)
{

    drawCode(h, w);
    drawInspector(h, w);
}

void EmulatorIDETUI::drawCode(int h, int w)
{
    int codeWidth = w / 2;
    int x = 0, y = 0;

    attron(COLOR_PAIR(4));
    mvprintw(y++, x, "%-*s", codeWidth, "CODE - DEBUG MODE");
    attroff(COLOR_PAIR(4));

    const auto &prog = emulator->getProgram();
    const auto &labels = emulator->getLabels();
    size_t ip = emulator->getIP();
    int linesAvail = h - 3;
    int first = 0;

    if ((int)ip > linesAvail / 2)
        first = ip - linesAvail / 2;

    for (int i = 0; i < linesAvail && first + i < (int)prog.size(); ++i)
    {
        size_t idx = first + i;
        bool isBP = breakpoints.count(idx);
        bool isIP = idx == ip;

        std::string labelPrefix = "";
        for (const auto &labelPair : labels)
        {
            if (labelPair.second == idx)
            {
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

        std::string line = std::string(1, isBP ? '*' : ' ') +
                           std::to_string(idx) + ": " +
                           labelPrefix + prog[idx];
        if (line.length() > (size_t)codeWidth - 1)
            line = line.substr(0, codeWidth - 4) + "...";

        mvprintw(y + i, x, "%-*s", codeWidth - 1, line.c_str());

        if (isIP && isBP)
            attroff(COLOR_PAIR(3));
        else if (isIP)
            attroff(COLOR_PAIR(1));
        else if (isBP)
            attroff(COLOR_PAIR(2));
    }
}

void EmulatorIDETUI::drawInspector(int h, int w)
{
    int rightX = w / 2 + 1;
    int y = 0;

    attron(COLOR_PAIR(4));
    mvprintw(y++, rightX, "%-*s", w / 2 - 1, "REGISTERS & MEMORY");
    attroff(COLOR_PAIR(4));

    drawRegisters(y, rightX, w / 2 - 1);
    y += 8;
    drawStack(y, rightX, w / 2 - 1);
    y += 18;
    drawMemory(y, rightX, w / 2 - 1);
}

void EmulatorIDETUI::drawRegisters(int y, int x, int w)
{
    auto &r = emulator->getRegisters();
    mvprintw(y++, x, "REGISTERS");
    mvprintw(y++, x, "AX=%04X (AH=%02X AL=%02X)  BX=%04X", r.AX.x, r.AX.bytes.h, r.AX.bytes.l, r.BX.x);
    mvprintw(y++, x, "CX=%04X (CH=%02X CL=%02X)  DX=%04X", r.CX.x, r.CX.bytes.h, r.CX.bytes.l, r.DX.x);
    mvprintw(y++, x, "SI=%04X DI=%04X BP=%04X SP=%04X", r.SI, r.DI, r.BP, r.SP);
    mvprintw(y++, x, "CS=%04X DS=%04X ES=%04X SS=%04X", r.CS, r.DS, r.ES, r.SS);
    mvprintw(y++, x, "IP=%04X", r.IP);
    mvprintw(y++, x, "FLAGS=%04X [O=%d D=%d I=%d T=%d S=%d Z=%d A=%d P=%d C=%d]", r.FLAGS,
             !!(r.FLAGS & Registers::OF), !!(r.FLAGS & Registers::DF), !!(r.FLAGS & Registers::IF), !!(r.FLAGS & Registers::TF),
             !!(r.FLAGS & Registers::SF), !!(r.FLAGS & Registers::ZF), !!(r.FLAGS & Registers::AF), !!(r.FLAGS & Registers::PF), !!(r.FLAGS & Registers::CF));
}

void EmulatorIDETUI::drawStack(int y, int x, int w)
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

void EmulatorIDETUI::drawMemory(int y, int x, int w)
{
    mvprintw(y++, x, "MEMORY %04X..%04X", memWindowStart, memWindowStart + memWindowSize);
    int bytesPerRow = 8;
    int rows = std::min(8, memWindowSize / bytesPerRow);
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

void EmulatorIDETUI::drawStatus(int h, int w)
{
    attron(COLOR_PAIR(5));
    std::string mode = currentMode == 0 ? "EDIT" : (running ? "DEBUG-RUN" : "DEBUG-PAUSE");
    std::string status = "[" + mode + "] " + statusMessage;
    if (currentMode == 0)
    {
        status += " | Line: " + std::to_string(cursorRow + 1) +
                  ", Col: " + std::to_string(cursorCol + 1);
    }
    else
    {
        status += " | IP: " + std::to_string(emulator->getIP());
    }
    mvprintw(h - 1, 0, "%-*s", w, status.c_str());
    attroff(COLOR_PAIR(5));
}

void EmulatorIDETUI::run()
{
    setStatus("F1=Help F2=Compile(WIP) F3=Debug F5=Run F10=Step Ctrl+S=Save Ctrl+O=Open Ctrl+N=New");

    while (!quit)
    {
        int ch = getch();

        if (ch != ERR)
        {
            if (currentMode == 0)
            {
                handleEditorInput(ch);
            }
            else
            {
                handleDebuggerInput(ch);
            }
        }

        if (running && currentMode == 1)
        {
            if (breakpoints.count(emulator->getIP()))
            {
                running = false;
                setStatus("Hit breakpoint at IP=" + std::to_string(emulator->getIP()));
            }
            else
            {
                bool cont = emulator->step();
                if (!cont)
                {
                    running = false;
                    setStatus("Program finished");
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        draw();
    }
}

void EmulatorIDETUI::handleEditorInput(int ch)
{
    switch (ch)
    {
    case KEY_F(1):

        setStatus("F1=Help F2=Compile(WIP) F3=Debug F5=Run F10=Step Ctrl+S=Save Ctrl+O=Open Ctrl+N=New");
        break;
    case KEY_F(2):
        compileAndLoad();
        break;
    case KEY_F(3):
        compileAndLoad();
        currentMode = 1;
        setStatus("Switched to debug mode. F3 to return to editor.");
        break;
    case KEY_F(5):
        compileAndLoad();
        currentMode = 1;
        running = true;
        setStatus("Running program...");
        break;
    case 15:
        loadProgram();
        break;
    case 19:
        saveProgram();
        break;
    case 14:
        newProgram();
        break;
    case 'q':
    case 'Q':
        quit = true;
        break;
    case KEY_UP:
        moveCursor(-1, 0);
        break;
    case KEY_DOWN:
        moveCursor(1, 0);
        break;
    case KEY_LEFT:
        moveCursor(0, -1);
        break;
    case KEY_RIGHT:
        moveCursor(0, 1);
        break;
    case KEY_HOME:
        cursorCol = 0;
        break;
    case KEY_END:
        cursorCol = getCurrentLine().length();
        break;
    case KEY_BACKSPACE:
    case 127:
    case 8:
        deleteChar();
        break;
    case KEY_ENTER:
    case 10:
    case 13:
        newLine();
        break;
    case KEY_DC:
        if (cursorCol < getCurrentLine().length())
        {
            std::string line = getCurrentLine();
            line.erase(cursorCol, 1);
            setCurrentLine(line);
        }
        break;
    default:
        if (ch >= 32 && ch <= 126)
        {
            insertChar(ch);
        }
        break;
    }

    int h, w;
    getmaxyx(stdscr, h, w);
    ensureCursorVisible(h, w);
}

void EmulatorIDETUI::handleDebuggerInput(int ch)
{
    switch (ch)
    {
    case KEY_F(3):
        currentMode = 0;
        running = false;
        setStatus("Switched to edit mode. F3 to return to debugger.");
        break;
    case KEY_F(5):
        running = !running;
        setStatus(running ? "Running..." : "Paused");
        break;
    case KEY_F(10):
        running = false;
        step();
        break;
    case 'b':
    case 'B':
        toggleBreakpoint();
        break;
    case 'c':
    case 'C':
        running = true;
        setStatus("Continuing...");
        break;
    case 'r':
    case 'R':
        emulator->reset();
        compileAndLoad();
        setStatus("Program reset and reloaded");
        break;
    case 'q':
    case 'Q':
        quit = true;
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
    }
}

void EmulatorIDETUI::compileAndLoad()
{
    try
    {
        emulator->reset();
        emulator->loadProgram(editorLines);
        setStatus("Program compiled and loaded successfully. " +
                  std::to_string(emulator->getProgram().size()) + " instructions, " +
                  std::to_string(emulator->getLabels().size()) + " labels.");
    }
    catch (const std::exception &e)
    {
        setStatus("Compilation error: " + std::string(e.what()));
    }
}

void EmulatorIDETUI::newProgram()
{
    editorLines.clear();
    editorLines.push_back("; New 8086 Assembly Program");
    editorLines.push_back("");
    cursorRow = 1;
    cursorCol = 0;
    editorScrollY = 0;
    editorScrollX = 0;
    setStatus("New program created");
}

void EmulatorIDETUI::saveProgram()
{

    std::ofstream file("program.asm");
    if (file)
    {
        for (const auto &line : editorLines)
        {
            file << line << "\n";
        }
        setStatus("Program saved to program.asm");
    }
    else
    {
        setStatus("Error: Could not save file");
    }
}

void EmulatorIDETUI::loadProgram()
{

    std::ifstream file("program.asm");
    if (file)
    {
        editorLines.clear();
        std::string line;
        while (std::getline(file, line))
        {
            editorLines.push_back(line);
        }
        if (editorLines.empty())
            editorLines.push_back("");
        cursorRow = 0;
        cursorCol = 0;
        editorScrollY = 0;
        editorScrollX = 0;
        setStatus("Program loaded from program.asm");
    }
    else
    {
        setStatus("Error: Could not open program.asm");
    }
}

void EmulatorIDETUI::insertChar(char ch)
{
    std::string line = getCurrentLine();
    line.insert(cursorCol, 1, ch);
    setCurrentLine(line);
    cursorCol++;
}

void EmulatorIDETUI::deleteChar()
{
    if (cursorCol > 0)
    {
        std::string line = getCurrentLine();
        line.erase(cursorCol - 1, 1);
        setCurrentLine(line);
        cursorCol--;
    }
    else if (cursorRow > 0)
    {

        std::string currentLine = getCurrentLine();
        cursorRow--;
        cursorCol = getCurrentLine().length();
        setCurrentLine(getCurrentLine() + currentLine);
        editorLines.erase(editorLines.begin() + cursorRow + 1);
    }
}

void EmulatorIDETUI::newLine()
{
    std::string line = getCurrentLine();
    std::string newLine = line.substr(cursorCol);
    setCurrentLine(line.substr(0, cursorCol));

    cursorRow++;
    cursorCol = 0;
    editorLines.insert(editorLines.begin() + cursorRow, newLine);
}

void EmulatorIDETUI::moveCursor(int dy, int dx)
{
    cursorRow = std::max(0, std::min((int)editorLines.size() - 1, cursorRow + dy));
    cursorCol = std::max(0, cursorCol + dx);

    if (cursorCol > (int)getCurrentLine().length())
        cursorCol = getCurrentLine().length();
}

void EmulatorIDETUI::ensureCursorVisible(int h, int w)
{
    int editorHeight = h - 2;

    if (cursorRow < editorScrollY)
        editorScrollY = cursorRow;
    else if (cursorRow >= editorScrollY + editorHeight - 1)
        editorScrollY = cursorRow - editorHeight + 2;

    int availableWidth = w - 6;
    if (cursorCol < editorScrollX)
        editorScrollX = cursorCol;
    else if (cursorCol >= editorScrollX + availableWidth)
        editorScrollX = cursorCol - availableWidth + 1;
}

void EmulatorIDETUI::toggleBreakpoint()
{
    size_t ip = emulator->getIP();
    if (breakpoints.count(ip))
    {
        breakpoints.erase(ip);
        setStatus("Breakpoint removed at IP=" + std::to_string(ip));
    }
    else
    {
        breakpoints.insert(ip);
        setStatus("Breakpoint set at IP=" + std::to_string(ip));
    }
}

void EmulatorIDETUI::step()
{
    try
    {
        bool cont = emulator->step();
        if (!cont)
            setStatus("Program finished");
        else
            setStatus("Stepped to IP=" + std::to_string(emulator->getIP()));
    }
    catch (const std::exception &e)
    {
        setStatus("Execution error: " + std::string(e.what()));
        running = false;
    }
}

void EmulatorIDETUI::setStatus(const std::string &msg)
{
    statusMessage = msg;
}

std::string EmulatorIDETUI::getCurrentLine()
{
    if (cursorRow >= 0 && cursorRow < (int)editorLines.size())
        return editorLines[cursorRow];
    return "";
}

void EmulatorIDETUI::setCurrentLine(const std::string &line)
{
    if (cursorRow >= 0 && cursorRow < (int)editorLines.size())
        editorLines[cursorRow] = line;
    else if (cursorRow == (int)editorLines.size())
        editorLines.push_back(line);
}
