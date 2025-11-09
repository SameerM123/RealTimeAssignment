# Requirement 3: Functional Task Diagram with Synchronization and Communication

## Introduction

This document presents the functional task diagram for the Adaptive Cruise Control (ACC) system, showing task types, data objects, synchronization structures, and communication mechanisms. The design ensures hard real-time constraints (< 100ms control cycle) are met while maintaining system safety and reliability.

**Reference**: Week 5 lecture notes on task synchronization and inter-task communication

---

## Task Identification and Classification

### Required Tasks

#### 1. **Setup Task**
- **Type**: Event-Driven (responds to ACC_on_off switch state change)
- **Criticality**: Soft (non-critical, can tolerate delays)
- **Priority**: Low

#### 2. **ISR (IRQ_sensors) - Timer Interrupt Service Routine**
- **Type**: Cyclic (periodic timer interrupt)
- **Criticality**: Hard (must execute within interrupt context)
- **Period**: T_ISR = 50ms
- **Priority**: Highest (interrupt level)

#### 3. **Sensors Task**
- **Type**: Cyclic (periodic, triggered by timer ISR)
- **Criticality**: Hard (critical for control loop stability)
- **Period**: T_Sensors = 50ms
- **Priority**: High (highest among hard tasks: Sensors > Control > Actuator)

#### 4. **Control Task**
- **Type**: Cyclic (periodic, triggered by Sensors task)
- **Criticality**: Hard (critical for control loop stability)
- **Period**: T_Control = 50ms
- **Priority**: High (middle priority among hard tasks: Sensors > Control > Actuator)

#### 5. **Actuator Task**
- **Type**: Cyclic (periodic, triggered by Control task)
- **Criticality**: Hard (critical for control loop stability)
- **Period**: T_Actuator = 50ms
- **Priority**: High (lowest priority among hard tasks: Sensors > Control > Actuator)

#### 6. **Display Task**
- **Type**: Cyclic (periodic, independent timing)
- **Criticality**: Soft (non-critical, can tolerate delays)
- **Period**: T_Display = 2000ms
- **Priority**: Low

---

## Functional Task Diagram

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    ACC SYSTEM FUNCTIONAL TASK DIAGRAM                       │
│                    (Synchronization & Communication)                        │
└─────────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────────┐
│                           HARDWARE LAYER                                    │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌──────────────┐         ┌──────────────┐         ┌──────────────┐      │
│  │   Hardware   │         │   Distance   │         │    Speed     │      │
│  │    Timer     │         │   Sensors    │         │   Sensors    │      │
│  │  (50ms ISR)  │         │  (Radar/     │         │  (Encoder/   │      │
│  │              │         │   Lidar)     │         │   GPS)       │      │
│  └──────┬───────┘         └──────┬───────┘         └──────┬───────┘      │
│         │                        │                         │              │
│         │ Timer Interrupt        │                         │              │
│         │                        │                         │              │
│         ▼                        │                         │              │
│  ┌──────────────┐               │                         │              │
│  │ ISR          │               │                         │              │
│  │(IRQ_sensors) │               │                         │              │
│  │              │               │                         │              │
│  │ [Posts Timer │               │                         │              │
│  │  Semaphore]  │               │                         │              │
│  └──────┬───────┘               │                         │              │
│         │                       │                         │              │
│         │ OSSemPost()           │                         │              │
│         │                       │                         │              │
└─────────┼───────────────────────┼─────────────────────────┼──────────────┘
          │                       │                         │
          │                       │                         │
┌─────────▼───────────────────────▼─────────────────────────▼──────────────┐
│                        SOFTWARE LAYER                                       │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐  │
│  │                    SYNCHRONIZATION STRUCTURES                        │  │
│  ├─────────────────────────────────────────────────────────────────────┤  │
│  │                                                                     │  │
│  │  ┌──────────────────┐                                              │  │
│  │  │ Timer Semaphore  │  [ISR→Sensors (counting, credit tracking)]  │  │
│  │  │  (Counting)      │                                              │  │
│  │  └────────┬─────────┘                                              │  │
│  │           │                                                         │  │
│  │           │ OSSemPend()                                             │  │
│  │           │                                                         │  │
│  │           ▼                                                         │  │
│  │  ┌──────────────────┐                                              │  │
│  │  │  Sensors Task    │  [Priority: High (Sensors > Control)]       │  │
│  │  │  (Cyclic, Hard)  │  [Period: 50ms]                             │  │
│  │  │                  │                                              │  │
│  │  │  1. Read sensors │                                              │  │
│  │  │  2. seq++        │                                              │  │
│  │  │  3. Write to     │                                              │  │
│  │  │     Param Block  │                                              │  │
│  │  │  4. seq++        │                                              │  │
│  │  │  5. OSTaskSemPost│                                              │  │
│  │  │     (Control)    │                                              │  │
│  │  └──────┬───────────┘                                              │  │
│  │         │                                                           │  │
│  │         │ OSTaskSemPost()                                           │  │
│  │         │                                                           │  │
│  │         ▼                                                           │  │
│  │  ┌──────────────────┐                                              │  │
│  │  │ Control Task     │  [Priority: High (Control > Actuator)]     │  │
│  │  │ Semaphore        │  [Task Semaphore: Sensors→Control]          │  │
│  │  │ (Built-in)       │  [Unilateral rendez-vous]                   │  │
│  │  └────────┬─────────┘                                              │  │
│  │           │                                                         │  │
│  │           │ OSTaskSemPend(timeout=45ms)                             │  │
│  │           │                                                         │  │
│  │           ▼                                                         │  │
│  │  ┌──────────────────┐                                              │  │
│  │  │  Control Task    │  [Priority: High (middle)]                   │  │
│  │  │  (Cyclic, Hard)  │  [Period: 50ms]                             │  │
│  │  │                  │                                              │  │
│  │  │  1. Check flags: │                                              │  │
│  │  │     ACC_ON AND   │                                              │  │
│  │  │     SafeToActuate│                                              │  │
│  │  │  2. Read seq₁    │                                              │  │
│  │  │  3. Copy data    │                                              │  │
│  │  │  4. Read seq₂    │                                              │  │
│  │  │  5. Verify seq₁==│                                              │  │
│  │  │     seq₂ & even  │                                              │  │
│  │  │  6. Compute dM(n)│                                              │  │
│  │  │  7. Write dM(n) │                                              │  │
│  │  │  8. OSSemPend    │                                              │  │
│  │  │     (flow ctrl)  │                                              │  │
│  │  │  9. OSQPost      │                                              │  │
│  │  └──────┬───────────┘                                              │  │
│  │         │                                                           │  │
│  │         │ OSQPost()                                                 │  │
│  │         │                                                           │  │
│  └─────────┼───────────────────────────────────────────────────────────┘  │
│            │                                                               │
│  ┌─────────▼───────────────────────────────────────────────────────────┐  │
│  │                    COMMUNICATION STRUCTURES                         │  │
│  ├─────────────────────────────────────────────────────────────────────┤  │
│  │                                                                     │  │
│  │  ┌──────────────────────────────────────────────────────────────┐    │  │
│  │  │ Control→Actuator Message Queue                             │    │  │
│  │  │ [N=3, bounded + counting semaphore for flow control]      │    │  │
│  │  │                                                             │    │  │
│  │  │ Flow Control Semaphore (initialized to N=3)                │    │  │
│  │  │                                                             │    │  │
│  │  │ Producer (Control):                                        │    │  │
│  │  │   OSSemPend(flow_sem) → OSQPost(msg)                      │    │  │
│  │  │                                                             │    │  │
│  │  │ Consumer (Actuator):                                       │    │  │
│  │  │   OSQPend(queue) → OSSemPost(flow_sem)                    │    │  │
│  │  └──────────────┬─────────────────────────────────────────────┘    │  │
│  │                 │                                                   │  │
│  │                 │ OSQPend()                                        │  │
│  │                 │                                                   │  │
│  │                 ▼                                                   │  │
│  │  ┌──────────────────┐                                              │  │
│  │  │  Actuator Task   │  [Priority: High (lowest among hard)]      │  │
│  │  │  (Cyclic, Hard)  │  [Period: 50ms]                             │  │
│  │  │                  │                                              │  │
│  │  │  1. Check flags: │                                              │  │
│  │  │     ACC_ON AND   │                                              │  │
│  │  │     SafeToActuate│                                              │  │
│  │  │  2. Receive dM(n)│                                              │  │
│  │  │  3. OSSemPost    │                                              │  │
│  │  │     (flow ctrl)  │                                              │  │
│  │  │  4. Apply to     │                                              │  │
│  │  │     actuators    │                                              │  │
│  │  └──────────────────┘                                              │  │
│  └─────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐  │
│  │                    SHARED DATA OBJECTS                                │  │
│  ├─────────────────────────────────────────────────────────────────────┤  │
│  │                                                                     │  │
│  │  ┌──────────────────────────────────────────────────────────────┐    │  │
│  │  │ Parameter Memory Block                                      │    │  │
│  │  │ [Parameter Block (short, copy-in/out)] 🔒                   │    │  │
│  │  │                                                             │    │  │
│  │  │ Protected by Mutex (OSMutexPend/OSMutexPost)               │    │  │
│  │  │                                                             │    │  │
│  │  │ Contents:                                                   │    │  │
│  │  │   - seq: Sequence counter (fresh-data guarantee)            │    │  │
│  │  │   - K1, K2, K3: Controller parameters                       │    │  │
│  │  │   - Vcruise, Vset, Xset                                     │    │  │
│  │  │   - Xn, Vn, Vn1, Vn2: Sensor data                           │    │  │
│  │  │   - dMn: Manipulated variable                               │    │  │
│  │  │                                                             │    │  │
│  │  │ Mutex Hygiene:                                              │    │  │
│  │  │   - All tasks acquire only this mutex                       │    │  │
│  │  │   - ≤ (TBD ms) copy operations                             │    │  │
│  │  │   - No nested locks                                         │    │  │
│  │  │   - Compute outside mutex                                   │    │  │
│  │  └────────────────────────────────────────────────────────────┘    │  │
│  │                                                                     │  │
│  │  ┌──────────────────────────────────────────────────────────────┐    │  │
│  │  │ Event Flag Group                                             │    │  │
│  │  │ [Multi-event status: ACC_ON, ACC_OFF, DeadlineMiss,         │    │  │
│  │  │  SafeToActuate, FaultDetected]                              │    │  │
│  │  │                                                             │    │  │
│  │  │ Bit Assignments:                                            │    │  │
│  │  │   Bit #0: ACC_ON                                            │    │  │
│  │  │   Bit #1: ACC_OFF                                           │    │  │
│  │  │   Bit #2: DeadlineMiss                                     │    │  │
│  │  │   Bit #3: SafeToActuate                                    │    │  │
│  │  │   Bit #4: FaultDetected                                    │    │  │
│  │  │                                                             │    │  │
│  │  │ Semantics:                                                  │    │  │
│  │  │   - ACC Enable: ACC_ON AND SafeToActuate (SET_ALL)        │    │  │
│  │  │   - Fault Path: DeadlineMiss OR FaultDetected (SET_ANY)   │    │  │
│  │  └────────────────────────────────────────────────────────────┘    │  │
│  └─────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐  │
│  │                    SOFT REAL-TIME TASKS                              │  │
│  ├─────────────────────────────────────────────────────────────────────┤  │
│  │                                                                     │  │
│  │  ┌──────────────────┐                                              │  │
│  │  │  Display Task    │  [Priority: Low]                            │  │
│  │  │  (Cyclic, Soft)  │  [Period: 2000ms]                           │  │
│  │  │                  │                                              │  │
│  │  │  1. OSTimeDlyHMSM│                                              │  │
│  │  │     (2 seconds)  │                                              │  │
│  │  │  2. Read seq₁    │                                              │  │
│  │  │  3. Copy data    │                                              │  │
│  │  │  4. Read seq₂    │                                              │  │
│  │  │  5. Verify seq₁==│                                              │  │
│  │  │     seq₂ & even  │                                              │  │
│  │  │  6. Display LCD  │                                              │  │
│  │  └──────────────────┘                                              │  │
│  │                                                                     │  │
│  │  ┌──────────────────┐                                              │  │
│  │  │  Setup Task      │  [Priority: Low]                            │  │
│  │  │  (Event-Driven,  │                                              │  │
│  │  │   Soft)          │                                              │  │
│  │  │                  │                                              │  │
│  │  │  1. Monitor event│                                              │  │
│  │  │     flags        │                                              │  │
│  │  │  2. On ACC_ON:   │                                              │  │
│  │  │     - Clear      │                                              │  │
│  │  │       DeadlineMiss│                                             │  │
│  │  │     - Init params│                                              │  │
│  │  │     - Enable     │                                              │  │
│  │  │       timer      │                                              │  │
│  │  │  3. On fault:    │                                              │  │
│  │  │     - Clear queue│                                              │  │
│  │  │     - Zero dM    │                                              │  │
│  │  │     - Disable    │                                              │  │
│  │  │       timer      │                                              │  │
│  │  └──────────────────┘                                              │  │
│  └─────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
│  ┌─────────────────────────────────────────────────────────────────────┐  │
│  │                    WATCHDOG TIMER                                    │  │
│  ├─────────────────────────────────────────────────────────────────────┤  │
│  │                                                                     │  │
│  │  One-Shot Timer Callback (non-blocking)                            │  │
│  │  - Checks if Control/Actuator completed in last cycle              │  │
│  │  - On deadline miss → sets event flag (DeadlineMiss)                │  │
│  │  - Setup task responds → safe-OFF transition                        │  │
│  └─────────────────────────────────────────────────────────────────────┘  │
│                                                                             │
└─────────────────────────────────────────────────────────────────────────────┘

Legend:
  → Data Flow
  → Synchronization Signal (Semaphore Post/Pend)
  → Communication (Message Queue Post/Pend)
  🔒 Mutex Protection
  [ ] Annotations and Labels
```

---

## Timing Constraints

### Hard Real-Time Constraints

- **OS Tick**: 10ms (concrete timing pick)
- **Control Cycle Period**: T_cycle = 50ms (≤ 100ms requirement)
- **ISR Period**: T_ISR = 50ms (multiple of OS tick: 10ms × 5)
- **Sensors Period**: T_Sensors = 50ms
- **Control Period**: T_Control = 50ms
- **Actuator Period**: T_Actuator = 50ms

### Control Loop Timing Budget (≤ 50ms)

**Timing Constraint**: ISR_latency + C_sensors + C_control + C_actuator + (enqueue+dequeue) ≤ 50ms

Where (placeholders to be filled after WCETs are measured):
- ISR_latency: Timer interrupt processing time (TBD ms)
- C_sensors: Worst-case execution time of Sensors task (TBD ms)
- C_control: Worst-case execution time of Control task (TBD ms)
- C_actuator: Worst-case execution time of Actuator task (TBD ms)
- (enqueue+dequeue): Message queue post/pend operations overhead (TBD ms)

### Soft Real-Time Constraints

- **Display Update Period**: T_Display = 2000ms (2 seconds, locked to OS tick via OSTimeDlyHMSM())

---

## Priority Assignment

### Explicit Priority Ordering (Minimize Latency)

**Sensors > Control > Actuator** (all "High" priority, but strictly ordered with distinct fixed priorities)

**Rationale**: 
- Guarantees: read completes before compute; compute completes before apply—within the same 50ms frame
- Minimizes latency by ensuring Sensors task (data acquisition) completes before Control task (computation) starts
- Ensures Control task completes before Actuator task (actuation) starts

### Priority Levels (High to Low)

1. **ISR (IRQ_sensors)**: Highest (interrupt level)
2. **Sensors Task**: High (highest among hard tasks)
3. **Control Task**: High (middle priority among hard tasks)
4. **Actuator Task**: High (lowest priority among hard tasks)
5. **Display Task**: Low (soft real-time)
6. **Setup Task**: Low (non-critical initialization)

---

## Synchronization Structures

### 1. Timer Semaphore (ISR → Sensors)
- **Type**: Counting semaphore
- **Function**: OSSemCreate(), OSSemPend(), OSSemPost()
- **Purpose**: Synchronize ISR → Sensors task (timing synchronization)
- **Label**: "ISR→Sensors (counting, credit tracking)"
- **Usage**: ISR posts semaphore each timer interrupt; Sensors task pends (blocks until ISR posts)

### 2. Control Task Semaphore (Sensors → Control)
- **Type**: Built-in task semaphore
- **Function**: OSTaskSemPost(), OSTaskSemPend()
- **Purpose**: Synchronize Sensors → Control task (task-to-task synchronization)
- **Label**: "Sensors→Control (task semaphore, unilateral rendez-vous)"
- **Usage**: Sensors posts to Control's built-in semaphore; Control pends with timeout = 45ms

### 3. Mutex for Parameter Memory Block
- **Type**: Mutex (mutual exclusion)
- **Function**: OSMutexCreate(), OSMutexPend(), OSMutexPost()
- **Purpose**: Protect shared parameter memory block from concurrent access
- **Label**: "Parameter Block (short, copy-in/out)" 🔒
- **Mutex Hygiene**: All tasks acquire only the parameter-block mutex, for ≤ (TBD ms) copy; no nested locks; compute outside mutex

### 4. Event Flag Group (Multi-Event Status)
- **Type**: Event flags (OS_FLAG_GRP)
- **Function**: OSFlagCreate(), OSFlagPost(), OSFlagPend()
- **Purpose**: Represent multi-event status (modes, faults, ACC state)
- **Bit Assignments**:
  - Bit #0: ACC_ON
  - Bit #1: ACC_OFF
  - Bit #2: DeadlineMiss
  - Bit #3: SafeToActuate
  - Bit #4: FaultDetected
- **Semantics**:
  - **ACC Enable Path**: Control/Actuator must see ACC_ON AND SafeToActuate (OSFlagPend with SET_ALL)
  - **Fault Path**: DeadlineMiss OR FaultDetected ⇒ Setup drives safe-OFF (OSFlagPend with SET_ANY)

---

## Communication Structures

### 1. Control → Actuator Message Queue
- **Type**: Message queue
- **Function**: OSQCreate(), OSQPost(), OSQPend()
- **Purpose**: Asynchronous communication from Control to Actuator
- **Queue Size**: N = 3 messages (explicit sizing)
- **Label**: "N=3, bounded + counting semaphore for flow control"
- **Flow Control**:
  - Flow control semaphore initialized to N = 3
  - Producer (Control): OSSemPend(flow_sem) → OSQPost(msg)
  - Consumer (Actuator): OSQPend(queue) → OSSemPost(flow_sem)
- **ISR Discipline**: ISR never touches the queue; only Control/Actuator tasks do

### 2. Parameter Memory Block (Shared Memory)
- **Type**: Shared memory (mutex-protected)
- **Contents**: seq counter, controller parameters, sensor data, dM(n)
- **Fresh-Data Guarantee**:
  - **Sensors**: seq++ → write all fields → seq++ (both increments under mutex)
  - **Control/Display**: read seq₁ → copy all fields → read seq₂ and accept only if seq₁ == seq₂ and even

---

## Startup & Teardown Defaults

### On Power-Up or ACC_OFF
- Set dM = 0, Vset = Vcruise
- Event flags: ACC_OFF set
- Timer disabled

### When ACC_ON Asserted
- Clear DeadlineMiss flag
- Initialize Parameter Memory Block with default values (dM = 0, Vset = Vcruise)
- Enable timer interrupt
- Create/start other tasks

### On Fault (DeadlineMiss OR FaultDetected)
- Disable timer interrupt
- Stop/suspend other tasks
- Clear message queue (drain any pending messages)
- Set dM = 0 in Parameter Memory Block
- Set event flags: ACC_OFF
- Transition ACC to safe OFF state

---

## Summary

This functional task diagram implements a hard real-time ACC system with:
- **6 tasks** (Setup, ISR, Sensors, Control, Actuator, Display)
- **Explicit priority ordering** (Sensors > Control > Actuator) to minimize latency
- **Fresh-data guarantee** using sequence counter to prevent torn reads
- **Bounded asynchronous communication** with flow control (N=3 queue)
- **Event flags** for multi-event status (ACC_ON, faults, safe-to-actuate)
- **Mutex-protected shared memory** with short critical sections
- **50ms control cycle** meeting the < 100ms requirement
- **Watchdog timer** for deadline miss detection

All synchronization and communication structures are aligned with Week 5 lecture notes and ensure predictable, real-time behavior.

