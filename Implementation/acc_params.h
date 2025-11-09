#ifndef ACC_PARAMS_H
#define ACC_PARAMS_H

#include <stdint.h>

// Parameter Memory Block Structure
typedef struct {
    uint8_t seq;              // Sequence counter (fresh-data guarantee)
                              // Note: uint8_t wraps quickly (0-255), which is fine for freshness check
                              // If computing deltas, consider uint16_t
    uint8_t ACC01;            // ACC-on-off flag
    float K1, K2, K3;         // Controller parameters
    float Vcruise;            // Set cruise speed
    float Vset;               // Current cycle speed reference
    float Xset;               // Minimum safe distance
    float Xn;                 // Current distance (nth cycle)
    float Vn;                 // Current speed (nth cycle)
    float Vn1;                // Speed at cycle (n-1)
    float Vn2;                // Speed at cycle (n-2)
    float dMn;                // Manipulated variable
    float deltaV;             // Speed reduction parameter (for Equation 4)
} ACC_Parameters_t;

#endif // ACC_PARAMS_H


