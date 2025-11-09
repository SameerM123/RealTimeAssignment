#ifndef ACC_CONFIG_H
#define ACC_CONFIG_H

#include "os.h"

// Time Conversion Macro (ms to ticks)
// Note: OS_CFG_TICK_RATE_HZ is defined in os_cfg.h (e.g., 100 Hz = 10ms tick)
#define MS_TO_TICKS(ms) ((OS_TICK)(((ms) * (OS_TICK)OS_CFG_TICK_RATE_HZ) / 1000u))

// Task Priorities
// Note: In µC/OS-III, smaller numbers = higher priority
#define PRIO_SENSORS          (OS_PRIO)8      // Highest among hard tasks
#define PRIO_CONTROL          (OS_PRIO)9      // Middle priority
#define PRIO_ACTUATOR         (OS_PRIO)10     // Lowest among hard tasks
#define PRIO_DISPLAY          (OS_PRIO)20     // Low priority
#define PRIO_SETUP            (OS_PRIO)21     // Low priority
// Note: ISRs don't have OS priority (interrupt level)

// Stack Sizes (increased for hard tasks with FP math)
#define STK_SIZE_SENSORS      1024    // Increased for FP operations
#define STK_SIZE_CONTROL      1152    // Increased for FP + queue + flags (+128 buffer for large ISRs if needed)
#define STK_SIZE_ACTUATOR     768     // Increased for FP operations
#define STK_SIZE_DISPLAY      512     // Soft task
#define STK_SIZE_SETUP        512     // Soft task

// Timing Constants (in milliseconds)
#define TIMER_PERIOD_MS       50      // T_ISR = 50ms
#define CONTROL_TIMEOUT_MS    45      // Timeout < T_ISR (45ms)
#define DISPLAY_PERIOD_MS     2000    // 2 seconds

// Event Flag Bits
#define ACC_ON_FLAG           (OS_FLAGS)0x01
#define ACC_OFF_FLAG          (OS_FLAGS)0x02
#define DEADLINE_MISS_FLAG    (OS_FLAGS)0x04
#define SAFE_TO_ACTUATE_FLAG  (OS_FLAGS)0x08
#define FAULT_DETECTED_FLAG   (OS_FLAGS)0x10

#endif // ACC_CONFIG_H

