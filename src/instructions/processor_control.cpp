#include "instructions/processor_control.h"

#include <iostream>
#include <stdexcept>

#include "emulator8086.h"

ProcessorControlInstructions::ProcessorControlInstructions(Emulator8086* emu) : emulator(emu) {}

void ProcessorControlInstructions::clc(const std::vector<std::string>& operands) {
    if (!operands.empty())
        throw std::runtime_error("CLC takes no operands");
    emulator->getRegisters().FLAGS &= ~Registers::CF;
}

void ProcessorControlInstructions::cmc(const std::vector<std::string>& operands) {
    if (!operands.empty())
        throw std::runtime_error("CMC takes no operands");
    emulator->getRegisters().FLAGS ^= Registers::CF;
}

void ProcessorControlInstructions::stc(const std::vector<std::string>& operands) {
    if (!operands.empty())
        throw std::runtime_error("STC takes no operands");
    emulator->getRegisters().FLAGS |= Registers::CF;
}

void ProcessorControlInstructions::cld(const std::vector<std::string>& operands) {
    if (!operands.empty())
        throw std::runtime_error("CLD takes no operands");
    emulator->getRegisters().FLAGS &= ~Registers::DF;
}

void ProcessorControlInstructions::std(const std::vector<std::string>& operands) {
    if (!operands.empty())
        throw std::runtime_error("STD takes no operands");
    emulator->getRegisters().FLAGS |= Registers::DF;
}

void ProcessorControlInstructions::cli(const std::vector<std::string>& operands) {
    if (!operands.empty())
        throw std::runtime_error("CLI takes no operands");
    emulator->getRegisters().FLAGS &= ~Registers::IF;
}

void ProcessorControlInstructions::sti(const std::vector<std::string>& operands) {
    if (!operands.empty())
        throw std::runtime_error("STI takes no operands");
    emulator->getRegisters().FLAGS |= Registers::IF;
}

void ProcessorControlInstructions::hlt(const std::vector<std::string>& operands) {
    if (!operands.empty())
        throw std::runtime_error("HLT takes no operands");
    std::cout << "CPU halted. Program terminated.\n";
    // exit(0);
}

void ProcessorControlInstructions::wait(const std::vector<std::string>& operands) {
    if (!operands.empty())
        throw std::runtime_error("WAIT takes no operands");
}

void ProcessorControlInstructions::esc(const std::vector<std::string>& operands) {
    if (operands.empty())
        throw std::runtime_error("ESC requires operands");

    std::cout << "ESC instruction: Coprocessor operation - ";
    for (const auto& op : operands) {
        std::cout << op << " ";
    }
    std::cout << "(simulated)\n";
}

void ProcessorControlInstructions::lock(const std::vector<std::string>& operands) {
    if (operands.empty())
        throw std::runtime_error("LOCK requires an instruction to lock");

    std::cout << "LOCK prefix applied to: " << operands[0] << "\n";
}

void ProcessorControlInstructions::nop(const std::vector<std::string>& operands) {
    if (!operands.empty())
        throw std::runtime_error("NOP takes no operands");
}

void ProcessorControlInstructions::int_op(const std::vector<std::string>& operands) {
    if (operands.size() != 1)
        throw std::runtime_error("INT requires 1 operand");

    try {
        int intNum;
        try {
            intNum = std::stoi(operands[0], nullptr, 16);
        } catch (const std::invalid_argument& e) {
            throw std::runtime_error("Invalid interrupt number: " + operands[0]);
        } catch (const std::out_of_range& e) {
            throw std::runtime_error("Interrupt number out of range: " + operands[0]);
        }
        switch (intNum) {
            case 0x10:
                std::cout << "BIOS Video Interrupt (INT 10h) - simulated\n";
                break;
            case 0x13:
                std::cout << "BIOS Disk Interrupt (INT 13h) - simulated\n";
                break;
            case 0x16:
                std::cout << "BIOS Keyboard Interrupt (INT 16h) - simulated\n";
                break;
            case 0x20:
                std::cout << "DOS Function Call (INT 20h) - simulated\n";
                break;
            case 0x21:
                std::cout << "DOS Function Call (INT 21h) - simulated\n";
                break;
            default:
                std::cout << "Software Interrupt " << std::hex << intNum << "h - simulated\n";
                break;
        }

        emulator->getRegisters().SP -= 2;
        emulator->writeMemoryWord(emulator->getRegisters().SP, emulator->getRegisters().FLAGS);
        emulator->getRegisters().SP -= 2;
        emulator->writeMemoryWord(emulator->getRegisters().SP, emulator->getRegisters().CS);
        emulator->getRegisters().SP -= 2;
        emulator->writeMemoryWord(emulator->getRegisters().SP, emulator->getRegisters().IP);

        emulator->getRegisters().FLAGS &= ~Registers::IF;
    } catch (const std::exception&) {
        throw std::runtime_error("Invalid interrupt number: " + operands[0]);
    }
}

void ProcessorControlInstructions::into(const std::vector<std::string>& operands) {
    if (!operands.empty())
        throw std::runtime_error("INTO takes no operands");

    if (emulator->getRegisters().FLAGS & Registers::OF) {
        std::cout << "INTO: Overflow detected, generating interrupt 4\n";
        int_op({"4"});
    }
}

void ProcessorControlInstructions::iret(const std::vector<std::string>& operands) {
    if (!operands.empty())
        throw std::runtime_error("IRET takes no operands");

    emulator->getRegisters().IP = emulator->readMemoryWord(emulator->getRegisters().SP);
    emulator->getRegisters().SP += 2;
    emulator->getRegisters().CS = emulator->readMemoryWord(emulator->getRegisters().SP);
    emulator->getRegisters().SP += 2;
    emulator->getRegisters().FLAGS = emulator->readMemoryWord(emulator->getRegisters().SP);
    emulator->getRegisters().SP += 2;
}

void ProcessorControlInstructions::in_op(const std::vector<std::string>& operands) {
    if (operands.size() != 2)
        throw std::runtime_error("IN requires 2 operands");

    try {
        uint16_t port;
        try {
            port = std::stoi(operands[1], nullptr, 16);
        } catch (const std::invalid_argument& e) {
            throw std::runtime_error("Invalid port number: " + operands[1]);
        } catch (const std::out_of_range& e) {
            throw std::runtime_error("Port number out of range: " + operands[1]);
        }

        uint16_t value = 0;
        switch (port) {
            case 0x60:
                value = 0x1C;
                std::cout << "IN from keyboard port (60h): " << std::hex << value << "\n";
                break;
            case 0x61:
                value = 0x00;
                std::cout << "IN from keyboard status port (61h): " << std::hex << value << "\n";
                break;
            case 0x3F8:
                value = 0xFF;
                std::cout << "IN from COM1 port (3F8h): " << std::hex << value << "\n";
                break;
            default:
                value = 0x00;
                std::cout << "IN from port " << std::hex << port << "h: " << value
                          << " (simulated)\n";
                break;
        }

        if (emulator->is8BitRegister(operands[0]))
            emulator->getRegister8(operands[0]) = value & 0xFF;
        else
            emulator->getRegister(operands[0]) = value;
    } catch (const std::exception&) {
        throw std::runtime_error("Invalid port number: " + operands[1]);
    }
}

void ProcessorControlInstructions::out(const std::vector<std::string>& operands) {
    if (operands.size() != 2)
        throw std::runtime_error("OUT requires 2 operands");

    try {
        uint16_t port;
        try {
            port = std::stoi(operands[0], nullptr, 16);
        } catch (const std::invalid_argument& e) {
            throw std::runtime_error("Invalid port number: " + operands[0]);
        } catch (const std::out_of_range& e) {
            throw std::runtime_error("Port number out of range: " + operands[0]);
        }
        uint16_t value = emulator->getValue(operands[1]);

        switch (port) {
            case 0x61:
                std::cout << "OUT to system control port (61h): " << std::hex << value << "\n";
                break;
            case 0x3F8:
                std::cout << "OUT to COM1 port (3F8h): " << static_cast<char>(value & 0xFF) << "\n";
                break;
            case 0x378:
                std::cout << "OUT to LPT1 port (378h): " << std::hex << value << "\n";
                break;
            default:
                std::cout << "OUT to port " << std::hex << port << "h: " << value
                          << " (simulated)\n";
                break;
        }
    } catch (const std::exception&) {
        throw std::runtime_error("Invalid port number: " + operands[0]);
    }
}
