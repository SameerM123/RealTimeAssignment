#include "os.h"
#include "acc_types.h"
#include "acc_config.h"
#include "acc_hardware.h"

// Forward declarations for task functions
void Setup_Task(void *p_arg);
void Sensors_Task(void *p_arg);
void Control_Task(void *p_arg);
void Actuator_Task(void *p_arg);
void Display_Task(void *p_arg);
void IRQ_sensors_ISR(void);
void Watchdog_Timer_Callback(void *p_arg);

int main(void)
{
    OS_ERR err;
    
    // 1. Initialize hardware (CPU, peripherals, timer)
    Hardware_Init();
    
    // 2. Initialize uC/OS-III kernel
    OSInit(&err);
    
    // 3. Create kernel objects (before tasks)
    //    - Timer Semaphore
    OSSemCreate(&TimerSemaphore, "Timer Sem", 0, &err);
    
    //    - Flow Control Semaphore
    OSSemCreate(&FlowControlSemaphore, "Flow Ctrl Sem", 3, &err);
    
    //    - Parameter Block Mutex
    OSMutexCreate(&ParamMutex, "Param Mutex", &err);
    
    //    - Control→Actuator Message Queue
    OSQCreate(&ControlActuatorQueue, "Ctrl Act Queue", 3, &err);
    
    //    - Event Flag Group
    OSFlagCreate(&EventFlagGroup, "ACC Event Flags", (OS_FLAGS)0, &err);
    
    //    - Memory Partition (for message buffers)
    // Backing store: static float MessageBufferArray[3];
    OSMemCreate(&MessagePartition, "Msg Partition", 
                (void*)MessageBufferArray, 3, sizeof(float), &err);
    
    // 4. Initialize Parameter Memory Block (set defaults before using)
    Parameters.seq = 0;
    Parameters.ACC01 = 0;  // ACC OFF
    Parameters.K1 = 1.0f;  // Example controller gains (set before use)
    Parameters.K2 = 0.5f;
    Parameters.K3 = 0.25f;
    Parameters.Vcruise = 100.0f;  // Example: 100 km/h cruise speed
    Parameters.Xset = 50.0f;  // Example: 50m minimum safe distance
    Parameters.deltaV = 5.0f;  // Example: 5 km/h reduction
    Parameters.Vset = Parameters.Vcruise;  // Initialize Vset after Vcruise is set
    Parameters.dMn = 0.0f;
    Parameters.Xn = 0.0f;
    Parameters.Vn = 0.0f;
    Parameters.Vn1 = 0.0f;
    Parameters.Vn2 = 0.0f;
    
    // 5. Create tasks (after objects are created)
    //    - Setup Task
    OSTaskCreate(&SetupTCB, "Setup", Setup_Task, 0,
                 PRIO_SETUP, &SetupStk[0], STK_SIZE_SETUP/10,
                 STK_SIZE_SETUP, 0, 0, 0,
                 OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR, &err);
    
    //    - Sensors Task (FP task - save FP context)
    OSTaskCreate(&SensorsTCB, "Sensors", Sensors_Task, 0,
                 PRIO_SENSORS, &SensorsStk[0], STK_SIZE_SENSORS/10,
                 STK_SIZE_SENSORS, 0, 0, 0,
                 OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR | OS_OPT_TASK_SAVE_FP, &err);
    
    //    - Control Task (FP task - save FP context)
    OSTaskCreate(&ControlTCB, "Control", Control_Task, 0,
                 PRIO_CONTROL, &ControlStk[0], STK_SIZE_CONTROL/10,
                 STK_SIZE_CONTROL, 0, 0, 0,
                 OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR | OS_OPT_TASK_SAVE_FP, &err);
    
    //    - Actuator Task (FP task - save FP context)
    OSTaskCreate(&ActuatorTCB, "Actuator", Actuator_Task, 0,
                 PRIO_ACTUATOR, &ActuatorStk[0], STK_SIZE_ACTUATOR/10,
                 STK_SIZE_ACTUATOR, 0, 0, 0,
                 OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR | OS_OPT_TASK_SAVE_FP, &err);
    
    //    - Display Task
    OSTaskCreate(&DisplayTCB, "Display", Display_Task, 0,
                 PRIO_DISPLAY, &DisplayStk[0], STK_SIZE_DISPLAY/10,
                 STK_SIZE_DISPLAY, 0, 0, 0,
                 OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR, &err);
    
    // 6. Create and start watchdog timer (after tasks are ready)
    OSTmrCreate(&WatchdogTimer, "WD Timer", 0,
                MS_TO_TICKS(50), OS_OPT_TMR_PERIODIC,
                Watchdog_Timer_Callback, NULL, &err);
    
    OSTmrStart(&WatchdogTimer, &err);
    
    // 7. Start multitasking
    OSStart(&err);
    
    // Should never reach here
    while(1);
}

