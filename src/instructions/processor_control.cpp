#include "instructions/processor_control.h"
#include "emulator8086.h"
#include <stdexcept>
#include <iostream>

ProcessorControlInstructions::ProcessorControlInstructions(Emulator8086* emu) : emulator(emu) {}

// Flag control instructions
void ProcessorControlInstructions::clc(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("CLC takes no operands");
    emulator->getRegisters().FLAGS &= ~Registers::CF;
}

void ProcessorControlInstructions::cmc(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("CMC takes no operands");
    emulator->getRegisters().FLAGS ^= Registers::CF;
}

void ProcessorControlInstructions::stc(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("STC takes no operands");
    emulator->getRegisters().FLAGS |= Registers::CF;
}

void ProcessorControlInstructions::cld(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("CLD takes no operands");
    emulator->getRegisters().FLAGS &= ~Registers::DF;
}

void ProcessorControlInstructions::std(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("STD takes no operands");
    emulator->getRegisters().FLAGS |= Registers::DF;
}

void ProcessorControlInstructions::cli(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("CLI takes no operands");
    emulator->getRegisters().FLAGS &= ~Registers::IF;
}

void ProcessorControlInstructions::sti(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("STI takes no operands");
    emulator->getRegisters().FLAGS |= Registers::IF;
}

// Processor control instructions
void ProcessorControlInstructions::hlt(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("HLT takes no operands");
    std::cout << "CPU halted. Program terminated.\n";
    exit(0);
}

void ProcessorControlInstructions::wait(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("WAIT takes no operands");
    // WAIT instruction - in a real implementation, this would wait for coprocessor
    // For simulation purposes, we'll just continue
}

void ProcessorControlInstructions::esc(const std::vector<std::string> &operands)
{
    if (operands.empty())
        throw std::runtime_error("ESC requires operands");
    // ESC instruction - Enhanced implementation for coprocessor communication
    std::cout << "ESC instruction: Coprocessor operation - ";
    for (const auto& op : operands) {
        std::cout << op << " ";
    }
    std::cout << "(simulated)\n";
}

void ProcessorControlInstructions::lock(const std::vector<std::string> &operands)
{
    if (operands.empty())
        throw std::runtime_error("LOCK requires an instruction to lock");
    // LOCK prefix - in a real implementation, this would lock the bus
    // For simulation purposes, we'll just note it
    std::cout << "LOCK prefix applied to: " << operands[0] << "\n";
}

void ProcessorControlInstructions::nop(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("NOP takes no operands");
    // No operation - just continue
}

// Enhanced interrupt instructions
void ProcessorControlInstructions::int_op(const std::vector<std::string> &operands)
{
    if (operands.size() != 1)
        throw std::runtime_error("INT requires 1 operand");
    
    // Enhanced interrupt handling - simulate common interrupts
    try {
        int intNum = std::stoi(operands[0], nullptr, 16);
        switch (intNum) {
            case 0x10: // BIOS video services
                std::cout << "BIOS Video Interrupt (INT 10h) - simulated\n";
                break;
            case 0x13: // BIOS disk services
                std::cout << "BIOS Disk Interrupt (INT 13h) - simulated\n";
                break;
            case 0x16: // BIOS keyboard services
                std::cout << "BIOS Keyboard Interrupt (INT 16h) - simulated\n";
                break;
            case 0x20: // DOS function calls
                std::cout << "DOS Function Call (INT 20h) - simulated\n";
                break;
            case 0x21: // DOS function calls
                std::cout << "DOS Function Call (INT 21h) - simulated\n";
                break;
            default:
                std::cout << "Software Interrupt " << std::hex << intNum << "h - simulated\n";
                break;
        }
        
        // Simulate interrupt processing
        emulator->getRegisters().SP -= 2;
        emulator->writeMemoryWord(emulator->getRegisters().SP, emulator->getRegisters().FLAGS);
        emulator->getRegisters().SP -= 2;
        emulator->writeMemoryWord(emulator->getRegisters().SP, emulator->getRegisters().CS);
        emulator->getRegisters().SP -= 2;
        emulator->writeMemoryWord(emulator->getRegisters().SP, emulator->getRegisters().IP);
        
        // Clear interrupt flag
        emulator->getRegisters().FLAGS &= ~Registers::IF;
        
    } catch (const std::exception&) {
        throw std::runtime_error("Invalid interrupt number: " + operands[0]);
    }
}

void ProcessorControlInstructions::into(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("INTO takes no operands");
    
    // Interrupt on overflow - enhanced implementation
    if (emulator->getRegisters().FLAGS & Registers::OF)
    {
        std::cout << "INTO: Overflow detected, generating interrupt 4\n";
        int_op({"4"}); // Generate interrupt 4
    }
}

void ProcessorControlInstructions::iret(const std::vector<std::string> &operands)
{
    if (!operands.empty())
        throw std::runtime_error("IRET takes no operands");
    
    // Enhanced interrupt return implementation
    emulator->getRegisters().IP = emulator->readMemoryWord(emulator->getRegisters().SP);
    emulator->getRegisters().SP += 2;
    emulator->getRegisters().CS = emulator->readMemoryWord(emulator->getRegisters().SP);
    emulator->getRegisters().SP += 2;
    emulator->getRegisters().FLAGS = emulator->readMemoryWord(emulator->getRegisters().SP);
    emulator->getRegisters().SP += 2;
}

// Enhanced I/O instructions
void ProcessorControlInstructions::in_op(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("IN requires 2 operands");
    
    // Enhanced I/O port simulation
    try {
        uint16_t port = std::stoi(operands[1], nullptr, 16);
        
        // Simulate common I/O ports
        uint16_t value = 0;
        switch (port) {
            case 0x60: // Keyboard data port
                value = 0x1C; // Simulate 'A' key
                std::cout << "IN from keyboard port (60h): " << std::hex << value << "\n";
                break;
            case 0x61: // Keyboard status port
                value = 0x00;
                std::cout << "IN from keyboard status port (61h): " << std::hex << value << "\n";
                break;
            case 0x3F8: // COM1 serial port
                value = 0xFF;
                std::cout << "IN from COM1 port (3F8h): " << std::hex << value << "\n";
                break;
            default:
                value = 0x00;
                std::cout << "IN from port " << std::hex << port << "h: " << value << " (simulated)\n";
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

void ProcessorControlInstructions::out(const std::vector<std::string> &operands)
{
    if (operands.size() != 2)
        throw std::runtime_error("OUT requires 2 operands");
    
    // Enhanced I/O port simulation
    try {
        uint16_t port = std::stoi(operands[0], nullptr, 16);
        uint16_t value = emulator->getValue(operands[1]);
        
        // Simulate common I/O ports
        switch (port) {
            case 0x61: // System control port
                std::cout << "OUT to system control port (61h): " << std::hex << value << "\n";
                break;
            case 0x3F8: // COM1 serial port
                std::cout << "OUT to COM1 port (3F8h): " << static_cast<char>(value & 0xFF) << "\n";
                break;
            case 0x378: // LPT1 parallel port
                std::cout << "OUT to LPT1 port (378h): " << std::hex << value << "\n";
                break;
            default:
                std::cout << "OUT to port " << std::hex << port << "h: " << value << " (simulated)\n";
                break;
        }
        
    } catch (const std::exception&) {
        throw std::runtime_error("Invalid port number: " + operands[0]);
    }
}
