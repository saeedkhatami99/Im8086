#ifndef MEMORY_COMPONENTS_H
#define MEMORY_COMPONENTS_H

#include <cstdint>

struct MemoryAddress
{
    uint16_t segment;
    uint16_t offset;
    bool hasSegmentOverride;
    std::string segmentOverride;

    MemoryAddress() : segment(0),
                      offset(0),
                      hasSegmentOverride(false),
                      segmentOverride("DS") {}

    uint32_t getPhysicalAddress() const
    {
        return (static_cast<uint32_t>(segment) << 4) + offset;
    }
};

struct MemoryOperand
{
    bool hasBase = false;
    bool hasIndex = false;
    bool hasDisplacement = false;
    uint16_t base = 0;
    uint16_t index = 0;
    int16_t displacement = 0;
    uint8_t size = 16;
};

MemoryOperand() : base(0),
                  index(0),
                  displacement(0),
                  hasBase(false),
                  hasIndex(false),
                  hasDisplacement(false),
                  hasSegmentOverride(false),
                  segmentOverride("DS"),
                  size(16) {}

MemoryAddress toAddress() const
{
    MemoryAddress addr;
    addr.offset = 0;
    if (hasBase)
        addr.offset += base;
    if (hasIndex)
        addr.offset += index;
    if (hasDisplacement)
        addr.offset += displacement;
    addr.hasSegmentOverride = hasSegmentOverride;
    addr.segmentOverride = segmentOverride;
    return addr;
}
}
;

#endif