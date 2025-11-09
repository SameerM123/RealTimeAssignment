#ifndef ACC_TYPES_H
#define ACC_TYPES_H

#include "os.h"
#include "acc_params.h"
#include "acc_config.h"

// Forward declarations
extern ACC_Parameters_t Parameters;

// Kernel Objects
extern OS_SEM TimerSemaphore;
extern OS_SEM FlowControlSemaphore;
extern OS_MUTEX ParamMutex;
extern OS_Q ControlActuatorQueue;
extern OS_FLAG_GRP EventFlagGroup;
extern OS_MEM MessagePartition;

// Watchdog Timer
extern OS_TMR WatchdogTimer;

// Task Control Blocks
extern OS_TCB SetupTCB;
extern OS_TCB SensorsTCB;
extern OS_TCB ControlTCB;
extern OS_TCB ActuatorTCB;
extern OS_TCB DisplayTCB;

// Task Stacks
extern CPU_STK SetupStk[STK_SIZE_SETUP];
extern CPU_STK SensorsStk[STK_SIZE_SENSORS];
extern CPU_STK ControlStk[STK_SIZE_CONTROL];
extern CPU_STK ActuatorStk[STK_SIZE_ACTUATOR];
extern CPU_STK DisplayStk[STK_SIZE_DISPLAY];

// Message Buffer Backing Store
extern float MessageBufferArray[3];

// Watchdog Heartbeat Flags
extern volatile bool control_beat;
extern volatile bool actuator_beat;

#endif // ACC_TYPES_H

