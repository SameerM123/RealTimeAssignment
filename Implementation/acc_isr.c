#include "acc_types.h"
#include "acc_config.h"
#include "acc_hardware.h"

// ISR (IRQ_sensors) - Timer Interrupt Service Routine
void IRQ_sensors_ISR(void)
{
    OS_ERR err;
    
    // ISR Prologue (save CPU context)
    OSIntEnter();
    
    // Clear interrupt flag (hardware-specific)
    Hardware_Timer_ClearFlag();
    
    // Post semaphore to Sensors task
    OSSemPost(&TimerSemaphore,
              OS_OPT_POST_1,
              &err);
    
    // ISR Epilogue (restore CPU context, check for higher priority task)
    OSIntExit();
}


