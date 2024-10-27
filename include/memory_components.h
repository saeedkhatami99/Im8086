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
    uint16_t base;     
    uint16_t index;    
    int16_t displacement;
    bool hasBase;
    bool hasIndex;
    bool hasDisplacement;
    
    MemoryOperand();
};

#endif