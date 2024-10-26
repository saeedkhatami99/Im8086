#ifndef MEMORY_COMPONENTS_H
#define MEMORY_COMPONENTS_H

#include <cstdint>

struct MemoryAddress {
    uint16_t segment;
    uint16_t offset;
    bool hasSegmentOverride;
    
    MemoryAddress();
};

struct MemoryOperand {
    uint16_t base;     // BX, BP
    uint16_t index;    // SI, DI
    int16_t displacement;
    bool hasBase;
    bool hasIndex;
    bool hasDisplacement;
    
    MemoryOperand();
};

#endif