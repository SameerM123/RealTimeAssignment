#include "acc_types.h"
#include "acc_config.h"
#include "acc_params.h"
#include <stdbool.h>

// Kernel Objects
OS_SEM TimerSemaphore;
OS_SEM FlowControlSemaphore;
OS_MUTEX ParamMutex;
OS_Q ControlActuatorQueue;
OS_FLAG_GRP EventFlagGroup;
OS_MEM MessagePartition;

// Watchdog Timer
OS_TMR WatchdogTimer;

// Task Control Blocks
OS_TCB SetupTCB;
OS_TCB SensorsTCB;
OS_TCB ControlTCB;
OS_TCB ActuatorTCB;
OS_TCB DisplayTCB;

// Task Stacks
CPU_STK SetupStk[STK_SIZE_SETUP];
CPU_STK SensorsStk[STK_SIZE_SENSORS];
CPU_STK ControlStk[STK_SIZE_CONTROL];
CPU_STK ActuatorStk[STK_SIZE_ACTUATOR];
CPU_STK DisplayStk[STK_SIZE_DISPLAY];

// Message Buffer Backing Store
float MessageBufferArray[3];

// Watchdog Heartbeat Flags
volatile bool control_beat = false;
volatile bool actuator_beat = false;

// Parameter Memory Block Instance
ACC_Parameters_t Parameters;

