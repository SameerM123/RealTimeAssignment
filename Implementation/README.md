# Requirement 4 Implementation: uC/OS-III ACC System

This directory contains the complete implementation of Requirement 4 for the Adaptive Cruise Control (ACC) system using µC/OS-III.

## File Structure

```
Implementation/
├── main.c                 // Main program, OSInit, OSStart, object creation
├── acc_tasks.c           // All task implementations
├── acc_isr.c             // ISR implementation
├── acc_objects.c         // Kernel object definitions and global variables
├── acc_types.h           // Data type definitions and forward declarations
├── acc_params.h          // Parameter memory block structure
├── acc_config.h          // Configuration constants (priorities, stack sizes, etc.)
├── acc_hardware.c        // Hardware abstraction layer (sensor reads, actuator writes)
├── acc_hardware.h        // Hardware abstraction layer header
└── README.md             // This file
```

## Key Features

### Tasks
1. **Setup Task** (Priority 21): Event-driven task that monitors ACC state and manages system initialization
2. **Sensors Task** (Priority 8): Highest priority hard task, reads sensors and updates parameter block
3. **Control Task** (Priority 9): Detailed implementation with full control algorithm (Equations 1-4)
4. **Actuator Task** (Priority 10): Applies control output to actuators
5. **Display Task** (Priority 20): Soft real-time task for LCD display updates

### Synchronization & Communication
- **Timer Semaphore**: ISR → Sensors task synchronization
- **Task Semaphore**: Sensors → Control task synchronization
- **Message Queue**: Control → Actuator communication (N=3, with flow control)
- **Mutex**: Parameter memory block protection
- **Event Flags**: ACC state and fault signaling

### Safety Features
- **Fresh-Data Guarantee**: Sequence counter prevents torn reads
- **Watchdog Timer**: Monitors Control/Actuator task completion via heartbeat flags
- **Non-Blocking Flag Checks**: OSFlagAccept() prevents blocking in hard tasks
- **Flow Control**: Queue + counting semaphore prevents overflow

### Control Algorithm
The Control task implements the complete control algorithm:
- **Equation 1**: Vset = Vcruise (when Xn ≥ Xset)
- **Equation 2**: Error calculation (e_n = Vset - Vn)
- **Equation 3**: Manipulated variable (dM_n = K1×e_n + K2×e_n1 + K3×e_n2)
- **Equation 4**: Vset = Vset - deltaV (when Xn < Xset)

## Configuration Requirements

Before compiling, ensure `os_cfg.h` has the following enabled:
- `OS_CFG_SEM_EN` → `DEF_ENABLED`
- `OS_CFG_MUTEX_EN` → `DEF_ENABLED`
- `OS_CFG_Q_EN` → `DEF_ENABLED`
- `OS_CFG_FLAG_EN` → `DEF_ENABLED`
- `OS_CFG_TMR_EN` → `DEF_ENABLED`
- `OS_CFG_TASK_SEM_EN` → `DEF_ENABLED`
- `OS_CFG_TICK_RATE_HZ` → `100` (for 10ms tick)

## Priority Justification

- **Sensors (8)**: Highest among hard tasks - must complete data acquisition before next timer interrupt
- **Control (9)**: Middle priority - depends on Sensors completion, must finish calculations within 50ms
- **Actuator (10)**: Lowest among hard tasks - depends on Control completion, applies output within 50ms
- **Display (20)**: Soft real-time - can tolerate delays
- **Setup (21)**: Event-driven - non-critical initialization

Priority ordering ensures: read completes before compute; compute completes before apply—within the same 50ms frame.

## Timing Constraints

- **OS Tick**: 10ms
- **Timer Period (T_ISR)**: 50ms
- **Control Cycle**: ≤ 50ms (all hard tasks)
- **Control Timeout**: 45ms (deadline miss detection)
- **Display Period**: 2000ms (2 seconds)

## Notes

- Control task is **fully implemented** (not pseudo-code) as required
- Other tasks use pseudo-code with proper µC/OS-III function calls
- Hardware abstraction layer functions are placeholders - replace with actual hardware interfaces
- Watchdog heartbeat mechanism monitors task completion each cycle
- All API calls follow µC/OS-III conventions with proper error handling

## Building

This code requires µC/OS-III kernel headers and libraries. Include all source files in your build system and link against µC/OS-III libraries.

## References

- Week 3, 4, 5 lecture notes on µC/OS-III
- Requirement 4 Plan document
- µC/OS-III API documentation

