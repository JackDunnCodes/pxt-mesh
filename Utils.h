#ifndef MESH_UTILS_H
#define MESH_UTILS_H

#include "MicroBit.h"

#define bit(n) (1<<n)
#define bitSet(var, mask) ((var) |= (mask))
#define bitClear(var, mask) ((var) &= ~(mask))
#define bitToggle(var, mask) ((var) ^= (mask))
#define bitRead(var,mask) ((var) & (mask))

#endif