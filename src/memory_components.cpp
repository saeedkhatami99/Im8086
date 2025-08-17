#include "memory_components.h"

MemoryAddress::MemoryAddress() : segment(0), offset(0), hasSegmentOverride(false) {}

MemoryOperand::MemoryOperand()
    : base(0), index(0), displacement(0), hasBase(false), hasIndex(false), hasDisplacement(false) {}