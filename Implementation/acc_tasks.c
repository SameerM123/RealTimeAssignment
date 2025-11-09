#include "acc_types.h"
#include "acc_config.h"
#include "acc_hardware.h"
#include "acc_params.h"
#include <stdbool.h>
#include <stdint.h>

// Sensors Task - Pseudo-Code
void Sensors_Task(void *p_arg)
{
    OS_ERR err;
    CPU_TS ts;
    float Xn_local, Vn_local;
    
    while(1)
    {
        // Wait for timer interrupt signal
        OSSemPend(&TimerSemaphore,
                 0,
                 OS_OPT_PEND_BLOCKING,
                 &ts,
                 &err);
        
        // Read sensors (hardware I/O)
        Xn_local = Read_Distance_Sensor();
        Vn_local = Read_Speed_Sensor();
        
        // Update parameter memory block with fresh-data guarantee
        OSMutexPend(&ParamMutex,
                   0,
                   OS_OPT_PEND_BLOCKING,
                   &ts,
                   &err);
        
        // Fresh-data guarantee: seq++ → write → seq++
        // Speed history shift: Vn2 ← Vn1 ← Vn ← Vn_local (correct order)
        Parameters.seq++;
        Parameters.Vn2 = Parameters.Vn1;  // Shift: Vn1 → Vn2
        Parameters.Vn1 = Parameters.Vn;   // Shift: Vn → Vn1
        Parameters.Vn = Vn_local;         // New value
        Parameters.Xn = Xn_local;         // New distance
        Parameters.seq++;
        
        OSMutexPost(&ParamMutex,
                   OS_OPT_POST_NONE,
                   &err);
        
        // Signal Control task (task semaphore)
        OSTaskSemPost(&ControlTCB,
                     OS_OPT_POST_NONE,
                     &err);
    }
}

// Control Task - DETAILED IMPLEMENTATION
void Control_Task(void *p_arg)
{
    OS_ERR err;
    CPU_TS ts;
    OS_MSG_SIZE msg_size;
    
    // Local variables for calculations
    float Xn, Vn, Vn1, Vn2, Vset, Xset, Vcruise;
    float K1, K2, K3, deltaV;
    float e_n, e_n1, e_n2;  // Error values
    float dM_n;              // Manipulated variable
    uint8_t seq1, seq2;      // Sequence counter reads
    OS_FLAGS flags;          // Event flags
    float *msg_ptr;          // Message buffer pointer
    
    // Controller parameters will be read each cycle under mutex
    // (to avoid stale params if updated at runtime)
    
    while(1)
    {
        // Wait for signal from Sensors task (with timeout in ticks)
        OSTaskSemPend(MS_TO_TICKS(CONTROL_TIMEOUT_MS),  // Timeout = 90ms < T_ISR (100ms)
                     OS_OPT_PEND_BLOCKING,
                     &ts,
                     &err);
        
        // Check for timeout (deadline miss)
        if (err == OS_ERR_TIMEOUT)
        {
            // Set deadline miss event flag
            OSFlagPost(&EventFlagGroup,
                      (OS_FLAGS)DEADLINE_MISS_FLAG,
                      OS_OPT_POST_FLAG_SET,
                      &err);
            continue;  // Skip this cycle
        }
        
        // Check event flags: ACC_ON AND SafeToActuate (non-blocking check)
        flags = OSFlagAccept(&EventFlagGroup,
                            (OS_FLAGS)(ACC_ON_FLAG | SAFE_TO_ACTUATE_FLAG),
                            OS_OPT_PEND_FLAG_SET_ALL | OS_OPT_PEND_NON_BLOCKING,
                            &err);
        
        if (err != OS_ERR_NONE || 
            (flags & (ACC_ON_FLAG | SAFE_TO_ACTUATE_FLAG)) != 
            (ACC_ON_FLAG | SAFE_TO_ACTUATE_FLAG))
        {
            // ACC not enabled or not safe to actuate
            // Skip control calculation, wait for next cycle
            continue;
        }
        
        // Read Phase: Acquire mutex and copy data with fresh-data check
        OSMutexPend(&ParamMutex,
                   0,
                   OS_OPT_PEND_BLOCKING,
                   &ts,
                   &err);
        
        // Fresh-data guarantee: read seq₁ → copy → read seq₂
        // Cache controller gains safely (read under mutex to avoid stale params)
        seq1 = Parameters.seq;
        Xn = Parameters.Xn;
        Vn = Parameters.Vn;
        Vn1 = Parameters.Vn1;
        Vn2 = Parameters.Vn2;
        Vset = Parameters.Vset;
        Xset = Parameters.Xset;
        Vcruise = Parameters.Vcruise;
        K1 = Parameters.K1;      // Cache gains safely
        K2 = Parameters.K2;
        K3 = Parameters.K3;
        deltaV = Parameters.deltaV;
        seq2 = Parameters.seq;
        
        OSMutexPost(&ParamMutex,
                   OS_OPT_POST_NONE,
                   &err);
        
        // Fresh-Data Check: Accept only if seq₁ == seq₂ and even
        if (seq1 != seq2 || (seq1 & 1) != 0)
        {
            // Data was partially updated, skip this cycle
            // Optional: Could reuse last good sample here (store snapshot)
            continue;
        }
        
        // Compute Phase: Calculate dM(n) using control algorithm (outside mutex)
        // Control Algorithm Implementation
        
        if (Xn >= Xset)
        {
            // Equation 1: Vset = Vcruise
            Vset = Vcruise;
            
            // Equation 2: Calculate errors
            e_n = Vset - Vn;
            e_n1 = Vset - Vn1;
            e_n2 = Vset - Vn2;
            
            // Equation 3: Calculate manipulated variable
            dM_n = K1 * e_n + K2 * e_n1 + K3 * e_n2;
        }
        else  // Xn < Xset
        {
            // Equation 4: Vset = Vset - deltaV
            Vset = Vset - deltaV;
            
            // Equation 2: Calculate errors
            e_n = Vset - Vn;
            e_n1 = Vset - Vn1;
            e_n2 = Vset - Vn2;
            
            // Equation 3: Calculate manipulated variable
            dM_n = K1 * e_n + K2 * e_n1 + K3 * e_n2;
        }
        
        // Output Phase: Store dM(n) in parameter memory block
        OSMutexPend(&ParamMutex,
                   0,
                   OS_OPT_PEND_BLOCKING,
                   &ts,
                   &err);
        
        Parameters.dMn = dM_n;
        Parameters.Vset = Vset;  // Update Vset for next cycle
        
        OSMutexPost(&ParamMutex,
                   OS_OPT_POST_NONE,
                   &err);
        
        // Post to Actuator task via message queue with flow control
        // Wait for available queue slot (flow control)
        OSSemPend(&FlowControlSemaphore,
                 0,
                 OS_OPT_PEND_BLOCKING,
                 &ts,
                 &err);
        
        // Allocate message buffer from memory partition
        msg_ptr = (float*)OSMemGet(&MessagePartition,
                                  &err);
        
        if (err == OS_ERR_NONE)
        {
            *msg_ptr = dM_n;  // Store dM(n) value
            
            // Post message to queue (with size parameter)
            OSQPost(&ControlActuatorQueue,
                   (void*)msg_ptr,
                   sizeof(float),
                   OS_OPT_POST_FIFO,
                   &err);
            
            // Optional: Log soft error counter if post fails
            if (err != OS_ERR_NONE)
            {
                // Error posting to queue, release semaphore and buffer
                OSSemPost(&FlowControlSemaphore,
                         OS_OPT_POST_NONE,
                         &err);
                OSMemPut(&MessagePartition,
                        (void*)msg_ptr,
                        &err);
            }
            else
            {
                // Control task cycle completed successfully
                control_beat = true;  // Set heartbeat flag
            }
        }
        else
        {
            // Memory allocation failed, release semaphore
            OSSemPost(&FlowControlSemaphore,
                     OS_OPT_POST_NONE,
                     &err);
            // Optional: Log soft error counter
        }
    }
}

// Actuator Task - Pseudo-Code
void Actuator_Task(void *p_arg)
{
    OS_ERR err;
    CPU_TS ts;
    OS_MSG_SIZE msg_size;
    float *dM_ptr;
    OS_FLAGS flags;
    
    while(1)
    {
        // Wait for control command from Control task (with size parameter)
        dM_ptr = (float*)OSQPend(&ControlActuatorQueue,
                                 0,
                                 OS_OPT_PEND_BLOCKING,
                                 &msg_size,
                                 &ts,
                                 &err);
        
        // Validate message received successfully
        if (err != OS_ERR_NONE)
        {
            // Error receiving message, continue to next cycle
            continue;
        }
        
        // Post to flow control semaphore (signal availability)
        OSSemPost(&FlowControlSemaphore,
                 OS_OPT_POST_NONE,
                 &err);
        
        // Check event flags: ACC_ON AND SafeToActuate (non-blocking check)
        flags = OSFlagAccept(&EventFlagGroup,
                            (OS_FLAGS)(ACC_ON_FLAG | SAFE_TO_ACTUATE_FLAG),
                            OS_OPT_PEND_FLAG_SET_ALL | OS_OPT_PEND_NON_BLOCKING,
                            &err);
        
        if (err == OS_ERR_NONE && 
            (flags & (ACC_ON_FLAG | SAFE_TO_ACTUATE_FLAG)) == 
            (ACC_ON_FLAG | SAFE_TO_ACTUATE_FLAG))
        {
            // Apply control value to actuators
            Apply_Throttle_Brake(*dM_ptr);
        }
        else
        {
            // ACC not enabled or not safe, explicitly neutralize output
            Apply_Throttle_Brake(0.0f);  // Neutral output (no acceleration/braking)
        }
        
        // Return message buffer to memory partition
        OSMemPut(&MessagePartition,
                (void*)dM_ptr,
                &err);
        
        // Actuator task cycle completed successfully
        actuator_beat = true;  // Set heartbeat flag
    }
}

// Display Task - Pseudo-Code
void Display_Task(void *p_arg)
{
    OS_ERR err;
    CPU_TS ts;
    float Xn, Vn;
    uint8_t seq1, seq2;
    uint8_t ACC_status;
    
    while(1)
    {
        // Wait 2 seconds (periodic delay)
        OSTimeDlyHMSM(0, 0, 2, 0,
                     OS_OPT_TIME_HMSM_NON_STRICT,
                     &err);
        
        // Read parameter memory block with fresh-data guarantee
        OSMutexPend(&ParamMutex,
                   0,
                   OS_OPT_PEND_BLOCKING,
                   &ts,
                   &err);
        
        // Fresh-data guarantee: read seq₁ → copy → read seq₂
        seq1 = Parameters.seq;
        Xn = Parameters.Xn;
        Vn = Parameters.Vn;
        ACC_status = Parameters.ACC01;
        seq2 = Parameters.seq;
        
        OSMutexPost(&ParamMutex,
                   OS_OPT_POST_NONE,
                   &err);
        
        // Fresh-Data Check: Accept only if seq₁ == seq₂ and even
        if (seq1 == seq2 && (seq1 & 1) == 0)
        {
            // Display on LCD
            LCD_Display_Distance(Xn);
            LCD_Display_Speed(Vn);
            LCD_Display_ACC_Status(ACC_status);
        }
        // Else: skip display update (data was partially updated)
        // Optional: Could reuse last good sample here
    }
}

// Setup Task - Pseudo-Code
void Setup_Task(void *p_arg)
{
    OS_ERR err;
    CPU_TS ts;
    OS_FLAGS flags;
    
    // Initialize parameter memory block defaults
    OSMutexPend(&ParamMutex,
               0,
               OS_OPT_PEND_BLOCKING,
               &ts,
               &err);
    
    Parameters.seq = 0;
    Parameters.ACC01 = 0;  // ACC OFF
    Parameters.dMn = 0.0f;
    Parameters.Vset = Parameters.Vcruise;
    Parameters.deltaV = 5.0f;  // Initialize deltaV (example: 5 km/h reduction)
    
    OSMutexPost(&ParamMutex,
               OS_OPT_POST_NONE,
               &err);
    
    // Set event flags: ACC_OFF
    OSFlagPost(&EventFlagGroup,
              (OS_FLAGS)ACC_OFF_FLAG,
              OS_OPT_POST_FLAG_SET,
              &err);
    
    while(1)
    {
        // Monitor event flags (ACC_ON, ACC_OFF, DeadlineMiss, FaultDetected)
        // Note: Setup_Task uses OSFlagPend (blocking) since it's intentionally event-driven
        flags = OSFlagPend(&EventFlagGroup,
                          (OS_FLAGS)(ACC_ON_FLAG | ACC_OFF_FLAG | 
                                    DEADLINE_MISS_FLAG | FAULT_DETECTED_FLAG),
                         0,
                         OS_OPT_PEND_FLAG_SET_ANY,
                         &ts,
                         &err);
        
        if (flags & ACC_ON_FLAG)
        {
            // ACC turned ON
            // Clear DeadlineMiss flag
            OSFlagPost(&EventFlagGroup,
                      (OS_FLAGS)DEADLINE_MISS_FLAG,
                      OS_OPT_POST_FLAG_CLR,
                      &err);
            
            // Reset timer semaphore credits (prevent runaway credits after long OFF period)
            OSSemSet(&TimerSemaphore,
                    0,
                    &err);
            
            // Initialize parameter memory block
            OSMutexPend(&ParamMutex,
                       0,
                       OS_OPT_PEND_BLOCKING,
                       &ts,
                       &err);
            
            Parameters.dMn = 0.0f;
            Parameters.Vset = Parameters.Vcruise;
            
            OSMutexPost(&ParamMutex,
                       OS_OPT_POST_NONE,
                       &err);
            
            // Enable timer interrupt
            Hardware_Timer_Enable();
        }
        else if ((flags & ACC_OFF_FLAG) || 
                 (flags & DEADLINE_MISS_FLAG) || 
                 (flags & FAULT_DETECTED_FLAG))
        {
            // ACC turned OFF or fault detected
            // Disable timer interrupt
            Hardware_Timer_Disable();
            
            // Clear message queue (drain pending messages)
            // Keep flow-control semaphore in sync
            OS_MSG_SIZE msg_size;
            void *p;
            while (1)
            {
                p = OSQPend(&ControlActuatorQueue,
                           0u,
                           OS_OPT_PEND_NON_BLOCKING,
                           &msg_size,
                           &ts,
                           &err);
                
                if (p == NULL || err == OS_ERR_PEND_WOULD_BLOCK)
                {
                    // Queue is empty
                    break;
                }
                
                // Return buffer to memory partition
                OSMemPut(&MessagePartition,
                        p,
                        &err);
                
                // Post to flow control semaphore (keep in sync)
                OSSemPost(&FlowControlSemaphore,
                         OS_OPT_POST_NONE,
                         &err);
            }
            
            // Set dM = 0
            OSMutexPend(&ParamMutex,
                       0,
                       OS_OPT_PEND_BLOCKING,
                       &ts,
                       &err);
            
            Parameters.dMn = 0.0f;
            
            OSMutexPost(&ParamMutex,
                       OS_OPT_POST_NONE,
                       &err);
            
            // Set event flags: ACC_OFF
            OSFlagPost(&EventFlagGroup,
                      (OS_FLAGS)ACC_OFF_FLAG,
                      OS_OPT_POST_FLAG_SET,
                      &err);
        }
    }
}

// Watchdog Timer Callback
void Watchdog_Timer_Callback(void *p_arg)
{
    OS_ERR err;
    
    // Check heartbeat flags: both Control and Actuator must complete each cycle
    if (!(control_beat && actuator_beat))
    {
        // Deadline miss detected - one or both tasks didn't complete
        OSFlagPost(&EventFlagGroup,
                  (OS_FLAGS)DEADLINE_MISS_FLAG,
                  OS_OPT_POST_FLAG_SET,
                  &err);
    }
    
    // Reset heartbeat flags for next period
    control_beat = false;
    actuator_beat = false;
    
    // Note: Timer is OS_OPT_TMR_PERIODIC, so no need to restart
    // Timer automatically restarts after callback completes
}

