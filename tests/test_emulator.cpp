#include "test_framework.h"
#include "emulator8086.h"
#include <vector>
#include <string>

TEST_CASE(EmulatorBasicInitialization) {
    Emulator8086 emulator;
    
    // Test initial state
    const auto& regs = emulator.getRegisters();
    REQUIRE_EQ(regs.AX.x, 0);
    REQUIRE_EQ(regs.BX.x, 0);
    REQUIRE_EQ(regs.CX.x, 0);
    REQUIRE_EQ(regs.DX.x, 0);
    REQUIRE_EQ(regs.IP, 0);
    REQUIRE_EQ(regs.SP, 0xFFFE);
    
    // Test memory is initialized
    const auto& memory = emulator.getMemory();
    REQUIRE(memory.size() > 0);
}

TEST_CASE(EmulatorBasicMOVInstruction) {
    Emulator8086 emulator;
    
    // Test MOV instruction
    emulator.executeInstruction("MOV AX, 1234h");
    
    const auto& regs = emulator.getRegisters();
    REQUIRE_EQ(regs.AX.x, 0x1234);
    REQUIRE_EQ(regs.AX.bytes.h, 0x12);
    REQUIRE_EQ(regs.AX.bytes.l, 0x34);
}

TEST_CASE(EmulatorRegisterToRegisterMOV) {
    Emulator8086 emulator;
    
    // Load a value into AX and copy to BX
    emulator.executeInstruction("MOV AX, 5678h");
    emulator.executeInstruction("MOV BX, AX");
    
    const auto& regs = emulator.getRegisters();
    REQUIRE_EQ(regs.AX.x, 0x5678);
    REQUIRE_EQ(regs.BX.x, 0x5678);
}

TEST_CASE(EmulatorArithmeticADD) {
    Emulator8086 emulator;
    
    // Test ADD instruction
    emulator.executeInstruction("MOV AX, 10h");
    emulator.executeInstruction("MOV BX, 20h");
    emulator.executeInstruction("ADD AX, BX");
    
    const auto& regs = emulator.getRegisters();
    REQUIRE_EQ(regs.AX.x, 0x30);
    REQUIRE_EQ(regs.BX.x, 0x20);
}

TEST_CASE(EmulatorProgramLoading) {
    Emulator8086 emulator;
    
    std::vector<std::string> program = {
        "MOV AX, 100h",
        "MOV BX, 200h",
        "ADD AX, BX"
    };
    
    emulator.loadProgram(program);
    const auto& loadedProgram = emulator.getProgram();
    
    REQUIRE_EQ(loadedProgram.size(), 3);
    REQUIRE_EQ(loadedProgram[0], "MOV AX, 100h");
    REQUIRE_EQ(loadedProgram[1], "MOV BX, 200h");
    REQUIRE_EQ(loadedProgram[2], "ADD AX, BX");
}

TEST_CASE(EmulatorStepExecution) {
    Emulator8086 emulator;
    
    std::vector<std::string> program = {
        "MOV AX, 100h",
        "MOV BX, 200h"
    };
    
    emulator.loadProgram(program);
    
    // Step through program
    bool canContinue = emulator.step();
    REQUIRE(canContinue);
    
    const auto& regs = emulator.getRegisters();
    REQUIRE_EQ(regs.AX.x, 0x100);
    REQUIRE_EQ(regs.IP, 1);
    
    // Second step
    canContinue = emulator.step();
    REQUIRE_EQ(regs.BX.x, 0x200);
    REQUIRE_EQ(regs.IP, 2);
}

TEST_CASE(EmulatorReset) {
    Emulator8086 emulator;
    
    // Modify emulator state
    emulator.executeInstruction("MOV AX, FFFFh");
    emulator.executeInstruction("MOV BX, FFFFh");
    
    // Reset
    emulator.reset();
    
    const auto& regs = emulator.getRegisters();
    REQUIRE_EQ(regs.AX.x, 0);
    REQUIRE_EQ(regs.BX.x, 0);
    REQUIRE_EQ(regs.IP, 0);
    REQUIRE_EQ(regs.SP, 0xFFFE);
}

TEST_CASE(EmulatorMemoryOperations) {
    Emulator8086 emulator;
    
    // Test memory read/write
    uint16_t testAddress = 0x1000;
    uint8_t testByte = 0xAB;
    uint16_t testWord = 0x1234;
    
    emulator.writeMemoryByte(testAddress, testByte);
    uint8_t readByte = emulator.readMemoryByte(testAddress);
    REQUIRE_EQ(readByte, testByte);
    
    emulator.writeMemoryWord(testAddress, testWord);
    uint16_t readWord = emulator.readMemoryWord(testAddress);
    REQUIRE_EQ(readWord, testWord);
}

int main() {
    return TestFramework::instance().runAll();
}
