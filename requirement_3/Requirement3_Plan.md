# Requirement 3 Plan: Functional Task Diagram with Synchronization and Communication

## Overview

Requirement 3 requires designing a functional task diagram for the ACC system that shows:
1. **Task types** (hard, soft, cyclic, event-driven)
2. **Main tasks** and their relationships
3. **Data objects** (shared memory, parameter memory block)
4. **Synchronization structures** (semaphores, event flags)
5. **Communication structures** (message queues, shared memory)

**Reference**: Week 5 lecture notes on task synchronization and inter-task communication

---

## Task Identification and Classification

### Required Tasks (Minimum 6 Tasks)

#### 1. **Setup Task**
- **Type**: **Event-Driven** (responds to ACC_on_off switch state change)
- **Criticality**: **Soft** (non-critical, can tolerate delays)
- **Function**: 
  - Monitors ACC_on_off switch (hardware input)
  - **Event Flag Usage**: Reads ACC_on_off status from event flag group (bit #0: ACC_ON, bit #1: ACC_OFF)
  - Performs initialization when ACC is turned ON
  - Sets up parameter memory block with initial values
  - Enables/disables other tasks based on switch state
  - Transitions ACC to safe OFF state if watchdog timer detects deadline miss
- **Activation**: Triggered by switch state change (interrupt posts event flag) or watchdog event flag
- **Priority**: Low (non-critical initialization)

#### 2. **ISR (IRQ_sensors) - Timer Interrupt Service Routine**
- **Type**: **Cyclic** (periodic timer interrupt)
- **Criticality**: **Hard** (must execute within interrupt context, < 100ms period)
- **Function**:
  - Timer interrupt fires every 50ms (concrete timing pick)
  - Posts semaphore to signal Sensors task (OSSemPost)
  - Minimal processing (just signal posting)
  - **ISR Discipline**: ISR never touches message queues; only posts semaphores for signaling
- **Activation**: Hardware timer interrupt
- **Period**: T_ISR = 50ms (concrete timing pick, ≤ 100ms requirement)
- **OS Tick**: 10ms (concrete timing pick)
- **Period Alignment**: T_ISR = 50ms is multiple of OS tick (10ms × 5)
- **Priority**: Highest (interrupt level)

#### 3. **Sensors Task**
- **Type**: **Cyclic** (periodic, triggered by timer ISR)
- **Criticality**: **Hard** (critical for control loop stability)
- **Function**:
  - Waits for semaphore from ISR (OSSemPend on Timer Semaphore)
  - Reads distance sensor: X(n)
  - Reads speed sensors: V(n), V(n-1), V(n-2)
  - Reads other sensors (if needed)
  - **Fresh-Data Guarantee**: Acquire mutex → seq++ → write all fields (X(n), V(n), V(n-1), V(n-2)) → seq++ → release mutex (both increments under mutex)
  - Signals Control task (OSTaskSemPost() to Control task's built-in semaphore)
- **Activation**: Triggered by ISR semaphore (periodic w.r.t. OS tick, no drift)
- **Period**: T_Sensors = T_ISR = 50ms (concrete timing pick, synchronized with timer)
- **Deadline**: Must complete before next timer interrupt (50ms)
- **Priority**: High (highest among hard tasks, distinct fixed priority: Sensors > Control > Actuator)

#### 4. **Control Task**
- **Type**: **Cyclic** (periodic, triggered by Sensors task)
- **Criticality**: **Hard** (critical for control loop stability)
- **Function**:
  - Waits for signal from Sensors task (OSTaskSemPend() with timeout = 45ms < T_ISR)
  - **Timeout Handling**: If timeout occurs → set watchdog/event flag bit (Deadline Miss) → skip control output that cycle
  - **Event Flag Check**: Must see ACC_ON AND SafeToActuate flags before producing output
  - **Controller Ordering** (minimize sample-to-actuate delay):
    1. **Read Phase**: Acquire mutex → read seq₁ → copy sensor data (X(n), V(n), V(n-1), V(n-2), Vset, Xset) to local variables → read seq₂ → release mutex immediately
    2. **Fresh-Data Check**: Accept data only if seq₁ == seq₂ and even (ensures no partial updates, no torn reads)
    3. **Compute Phase**: Calculate dM(n) using control algorithm (outside mutex):
       - If X(n) ≥ Xset: Vset = Vcruise, calculate errors, dM(n) = K1×e(n) + K2×e(n-1) + K3×e(n-2)
       - If X(n) < Xset: Vset = Vset - deltaV, calculate errors, dM(n) = K1×e(n) + K2×e(n-1) + K3×e(n-2)
    4. **Output Phase**: Acquire mutex → write dM(n) to Parameter Memory Block → release mutex immediately
  - Sends dM(n) to Actuator task (via message queue with flow control)
- **Activation**: Triggered by Sensors task completion (periodic w.r.t. OS tick, no drift)
- **Period**: T_Control = T_Sensors = 50ms (concrete timing pick, same as sensor period)
- **Deadline**: Must complete within 50ms control cycle
- **Priority**: High (middle priority among hard tasks, distinct fixed priority: Sensors > Control > Actuator)

#### 5. **Actuator Task**
- **Type**: **Cyclic** (periodic, triggered by Control task)
- **Criticality**: **Hard** (critical for control loop stability)
- **Function**:
  - Waits for control command from Control task (OSQPend on message queue)
  - **Event Flag Check**: Must see ACC_ON AND SafeToActuate flags before applying output
  - Receives dM(n) value from message queue
  - Posts to flow control semaphore (OSSemPost) after retrieving message
  - Sends control values to ACC actuators (throttle/brake)
  - Updates actuator hardware
- **Activation**: Triggered by Control task message (periodic w.r.t. OS tick, no drift)
- **Period**: T_Actuator = T_Control = 50ms (concrete timing pick, same as control period)
- **Deadline**: Must complete within 50ms control cycle
- **Priority**: High (lowest priority among hard tasks, distinct fixed priority: Sensors > Control > Actuator)

#### 6. **Display Task**
- **Type**: **Cyclic** (periodic, independent timing)
- **Criticality**: **Soft** (non-critical, can tolerate delays)
- **Function**:
  - Updates LCD display every 2 seconds
  - **Periodic Execution**: Uses OSTimeDlyHMSM() or OSTimeDly() for absolute/periodic delay (spacing doesn't depend on CPU load)
  - **Fresh-Data Guarantee**: Acquire mutex → read seq₁ → copy data (X(n), V(n), ACC status) to local variables → read seq₂ → release mutex immediately
  - **Fresh-Data Check**: Accept data only if seq₁ == seq₂ and even (ensures no partial updates, no torn reads)
  - Formats and displays information on LCD (computation outside mutex)
- **Activation**: Periodic delay API (OSTimeDlyHMSM(0, 0, 2, 0) for 2 seconds)
- **Period**: T_Display = 2000ms (2 seconds, locked to OS tick)
- **Deadline**: Soft (can tolerate occasional delays)
- **Priority**: Low (non-critical, runs when CPU available)

---

## Task Type Summary

| Task | Type | Criticality | Period/Activation | Priority |
|------|------|------------|-------------------|----------|
| Setup | Event-Driven | Soft | On switch change | Low |
| ISR (IRQ_sensors) | Cyclic | Hard | Timer interrupt (50ms) | Highest (ISR) |
| Sensors | Cyclic | Hard | Semaphore from ISR (50ms) | High (Sensors > Control > Actuator) |
| Control | Cyclic | Hard | Task semaphore from Sensors (50ms) | High (Sensors > Control > Actuator) |
| Actuator | Cyclic | Hard | Message from Control (50ms) | High (Sensors > Control > Actuator) |
| Display | Cyclic | Soft | Periodic timer (2000ms) | Low |

---

## Data Objects

### 1. **Parameter Memory Block** (Shared Memory)
- **Type**: Shared memory structure (mutex-protected)
- **Contents**:
  - **seq**: Sequence counter (for fresh-data guarantee, prevents torn reads)
  - K1, K2, K3: Controller parameters
  - Vcruise: Set cruise speed
  - Vset: Current cycle speed reference
  - Xset: Minimum safe distance
  - Xn: Current distance (nth cycle)
  - Vn: Current speed (nth cycle)
  - Vn1: Speed at cycle (n-1)
  - Vn2: Speed at cycle (n-2)
  - dMn: Manipulated variable
- **Access**: Protected by mutex (OSMutexPend/OSMutexPost)
- **Mutex Discipline**: **Short critical sections only** - Copy data in/out quickly, perform computation outside mutex
- **Priority Inversion Prevention**: Use mutex (not semaphore) to avoid unbounded priority inversion
- **Fresh-Data Guarantee (No Torn Reads)**:
  - **Sensors Task**: seq++ → write all fields (X(n), V(n), V(n-1), V(n-2)) → seq++ (both increments under mutex)
  - **Control/Display Tasks**: read seq₁ → copy all fields → read seq₂ and accept only if seq₁ == seq₂ and even (ensures no partial updates)
  - This ensures tasks never compute on partially updated samples without increasing mutex hold time
- **Accessed by**: Sensors, Control, Actuator, Display tasks
- **Note**: ACC_on_off status moved to event flag group (see Event Flags section)

### 2. **Message Buffers** (Fixed-Size Memory Partitions)
- **Type**: Fixed-size memory partitions for message queues
- **Purpose**: Store messages passed between tasks
- **Usage**: 
  - Control → Actuator: dM(n) value
  - (Optional) Sensors → Control: sensor data packet
- **Allocation**: Pre-allocated from OS memory partition manager
- **Rationale**: Deterministic allocation/free, prevents memory fragmentation [Week 5]

---

## Synchronization Structures

### 1. **Timer Semaphore** (Counting Semaphore)
- **Type**: Counting semaphore
- **Function**: OSSemCreate(), OSSemPend(), OSSemPost()
- **Purpose**: Synchronize ISR → Sensors task (timing synchronization)
- **Usage**:
  - ISR posts semaphore each timer interrupt
  - Sensors task pends on semaphore (blocks until ISR posts)
  - Credit tracking: accumulates if Sensors task is busy (prevents lost interrupts)
- **Label**: "ISR→Sensors (counting, credit tracking)"
- **Rationale**: Unilateral rendez-vous between ISR and task [Week 5, Page 223]

### 2. **Control Task Semaphore** (Task Semaphore)
- **Type**: Built-in task semaphore (μC/OS-III provides each task with its own semaphore)
- **Function**: OSTaskSemPost(), OSTaskSemPend()
- **Purpose**: Synchronize Sensors → Control task (task-to-task synchronization)
- **Usage**:
  - Sensors task posts to Control task's built-in semaphore (OSTaskSemPost()) after storing data in parameter memory block
  - Control task pends on its own semaphore (OSTaskSemPend()) - waits for new sensor data
- **Label**: "Sensors→Control (task semaphore, unilateral rendez-vous)"
- **Rationale**: Task semaphores keep rendez-vous tidy and avoid managing extra kernel objects [Week 5, Page 228-229]. Built-in semaphore cannot be disabled at compile time.

### 3. **Mutex for Parameter Memory Block**
- **Type**: Mutex (mutual exclusion)
- **Function**: OSMutexCreate(), OSMutexPend(), OSMutexPost()
- **Purpose**: Protect shared parameter memory block from concurrent access
- **Usage**:
  - Tasks acquire mutex before reading/writing parameter memory block
  - **Critical Section Discipline**: Keep lock sections tiny - copy data in/out quickly, perform computation outside mutex
  - Tasks release mutex immediately after copy operation
  - Prevents race conditions and data corruption
- **Priority Inversion Prevention**: Mutex supports priority inheritance protocol (unlike semaphores)
- **Deadlock Avoidance**: All tasks acquire resources in the same order; use time-bounded pends to prevent indefinite blocking [Week 4]
- **Mutex Hygiene**: All tasks acquire only the parameter-block mutex, for ≤ (TBD ms) copy; no nested locks; compute outside mutex. This covers deadlock/jitter concerns.
- **Label**: "Parameter Block (short, copy-in/out)" with lock icon
- **Rationale**: **Mutex for sharing** - Use mutexes to protect shared resources [Week 4]. Semaphores from ISRs are for signaling, not sharing.

### 4. **Event Flag Group** (Multi-Event Status)
- **Type**: Event flags (OS_FLAG_GRP)
- **Function**: OSFlagCreate(), OSFlagPost(), OSFlagPend()
- **Purpose**: Represent multi-event status (modes, faults, ACC state)
- **Bit Assignments**:
  - Bit #0: ACC_ON (ACC system enabled)
  - Bit #1: ACC_OFF (ACC system disabled)
  - Bit #2: Deadline Miss Detected (watchdog timer flag)
  - Bit #3: SafeToActuate (all checks passed)
  - Bit #4: FaultDetected (from safety modules, if integrated)
- **Event Flag Semantics**:
  - **ACC Enable Path**: Control/Actuator must see ACC_ON AND SafeToActuate before producing/applying an output (OSFlagPend with OS_OPT_PEND_FLAG_SET_ALL)
  - **Fault Path**: DeadlineMiss OR FaultDetected ⇒ Setup drives safe-OFF (clear queue, zero dM, disable timer). Setup task uses OSFlagPend with OS_OPT_PEND_FLAG_SET_ANY to detect either fault condition.
- **Usage**:
  - Setup task monitors ACC_ON/ACC_OFF flags and fault flags (OSFlagPend with OS_OPT_PEND_FLAG_SET_ANY)
  - Watchdog timer sets Deadline Miss flag
  - Control/Actuator tasks check ACC_ON AND SafeToActuate flags before output (OSFlagPend with OS_OPT_PEND_FLAG_SET_ALL)
- **Rationale**: Event flags preferred for multi-event status (ANY/ALL combinations) [Week 5, Page 233-234]

---

## Communication Structures

### 1. **Control → Actuator Message Queue**
- **Type**: Message queue
- **Function**: OSQCreate(), OSQPost(), OSQPend()
- **Purpose**: Asynchronous communication from Control to Actuator
- **Message Content**: dM(n) value (floating point)
- **Message Buffer**: Fixed-size memory partition
- **Queue Size**: N = 3 messages (explicit sizing)
- **Flow Control Implementation** (Explicit):
  - **Initialization**: Flow control counting semaphore initialized to N = 3 (OSSemCreate with initial count = 3)
  - **Producer (Control)**: OSSemPend on flow control semaphore → OSQPost message → (semaphore count decrements)
  - **Consumer (Actuator)**: OSQPend on message queue → retrieve message → OSSemPost on flow control semaphore → (semaphore count increments, signals availability)
  - Prevents burst of control messages from overflowing queue if Actuator is busy/preempted
- **ISR Discipline**: ISR never touches the queue; only Control/Actuator tasks do—keeps ISR strictly signal-only.
- **Label**: "Control→Actuator Queue (N=3, bounded + counting semaphore for flow control)"
- **Rationale**: Asynchronous communication, producer doesn't block consumer [Week 5]. Flow control prevents queue overflow [Week 5]. Explicit sizing (N=3) allows 2-3 bursts to accumulate if Actuator is briefly preempted.

### 2. **Parameter Memory Block** (Shared Memory)
- **Type**: Shared memory (mutex-protected)
- **Purpose**: Shared data structure for all tasks
- **Access Pattern**: 
  - Sensors task: Writes X(n), V(n), V(n-1), V(n-2)
  - Control task: Reads sensor data, writes dM(n)
  - Actuator task: Reads dM(n) (optional, can use message queue instead)
  - Display task: Reads X(n), V(n)
  - Setup task: Writes initial values
- **Protection**: Mutex ensures exclusive access
- **Note**: ACC_on_off status moved to event flag group (see Event Flags section)

### 3. **Optional: Sensors → Control Message Queue**
- **Type**: Message queue (alternative to semaphore + shared memory)
- **Purpose**: Pass sensor data packet from Sensors to Control
- **Message Content**: X(n), V(n), V(n-1), V(n-2) packet
- **Rationale**: Bounded asynchronous messaging [Week 5]

---

## Functional Task Diagram Structure

### Diagram Components:

1. **Tasks** (circles or rectangles):
   - Setup Task
   - ISR (IRQ_sensors)
   - Sensors Task
   - Control Task
   - Actuator Task
   - Display Task

2. **Data Objects** (rectangles or cylinders):
   - Parameter Memory Block (with mutex symbol)
   - Message Buffers (memory partitions)

3. **Synchronization Structures** (flags/semaphores):
   - Timer Semaphore (ISR → Sensors) - Label: "ISR→Sensors (counting, credit tracking)"
   - Control Task Semaphore (Sensors → Control) - Label: "Sensors→Control (task semaphore, unilateral rendez-vous)"
   - Mutex (Parameter Memory Block) - Label: "Parameter Block (short, copy-in/out)" with lock icon
   - Event Flag Group (multi-event status: ACC_ON, ACC_OFF, Deadline Miss, Safe to Actuate)

4. **Communication Structures** (queues):
   - Control → Actuator Message Queue - Label: "N=3, bounded + counting semaphore for flow control"
   - Flow Control Counting Semaphore (initialized to N=3, prevents queue overflow)
   - (Optional) Sensors → Control Message Queue

5. **Arrows/Connections**:
   - Data flow arrows
   - Synchronization arrows (semaphore posts/pends)
   - Communication arrows (message queue posts/pends)

---

## System Flow Description

### Control Loop Flow (< 100ms cycle):

1. **Timer Interrupt** (< 100ms period):
   - Hardware timer fires → ISR (IRQ_sensors) executes
   - ISR posts Timer Semaphore
   - ISR returns

2. **Sensors Task Activation**:
   - Sensors task pends on Timer Semaphore (blocks until ISR posts)
   - Sensors task wakes up, reads sensors:
     - Distance: X(n)
     - Speed: V(n), V(n-1), V(n-2)
   - Sensors task acquires Mutex
   - Sensors task writes X(n), V(n), V(n-1), V(n-2) to Parameter Memory Block
   - Sensors task releases Mutex
   - Sensors task posts to Control task's built-in semaphore (OSTaskSemPost())

3. **Control Task Activation**:
   - Control task pends on its own built-in semaphore (OSTaskSemPend())
   - Control task wakes up, acquires Mutex
   - Control task reads sensor data from Parameter Memory Block
   - Control task calculates dM(n) using control algorithm
   - Control task writes dM(n) to Parameter Memory Block
   - Control task releases Mutex
   - Control task posts dM(n) to Control → Actuator Message Queue

4. **Actuator Task Activation**:
   - Actuator task pends on Control → Actuator Message Queue
   - Actuator task receives dM(n) value
   - Actuator task sends control command to actuators (throttle/brake)
   - Control cycle complete

### Display Task Flow (Independent, 2-second period):

1. **Display Task Activation** (every 2 seconds):
   - Display task waits for 2-second timer (OSTimeDly or semaphore)
   - Display task acquires Mutex
   - Display task reads X(n), V(n), ACC status from Parameter Memory Block
   - Display task releases Mutex
   - Display task formats and displays on LCD

### Setup Task Flow (Event-Driven):

1. **Startup & Teardown Defaults**:
   - **On Power-Up or ACC_OFF**: Set dM = 0, Vset = Vcruise, event flags: ACC_OFF set; timer disabled
   - **When ACC_ON Asserted**: Clear DeadlineMiss flag, then enable timer

2. **Setup Task Activation** (on ACC_on_off switch change or watchdog event):
   - Setup task monitors event flag group (OSFlagPend with OS_OPT_PEND_FLAG_SET_ANY)
   - **Switch State Change**: Hardware interrupt posts event flag (bit #0: ACC_ON or bit #1: ACC_OFF)
   - **Watchdog Deadline Miss**: Watchdog timer posts event flag (bit #2: Deadline Miss)
   - **Fault Path Logic**: DeadlineMiss OR FaultDetected ⇒ Setup drives safe-OFF (clear queue, zero dM, disable timer)
   - If ACC_ON flag set:
     - Clear DeadlineMiss flag
     - Initialize Parameter Memory Block with default values (dM = 0, Vset = Vcruise)
     - Enable timer interrupt
     - Create/start other tasks
   - If ACC_OFF flag set OR DeadlineMiss flag set OR FaultDetected flag set:
     - Disable timer interrupt
     - Stop/suspend other tasks
     - Clear message queue (drain any pending messages)
     - Set dM = 0 in Parameter Memory Block
     - Set event flags: ACC_OFF
     - Transition ACC to safe OFF state

---

## Priority Assignment Rationale

### Priority Levels (High to Low):

1. **ISR (IRQ_sensors)**: Highest (interrupt level, cannot be preempted)
2. **Sensors Task**: High (highest among hard tasks, distinct fixed priority)
3. **Control Task**: High (middle priority among hard tasks, distinct fixed priority)
4. **Actuator Task**: High (lowest priority among hard tasks, distinct fixed priority)
5. **Display Task**: Low (soft real-time, can tolerate delays)
6. **Setup Task**: Low (non-critical initialization)

### Explicit Priority Ordering (Minimize Latency):

**Sensors > Control > Actuator** (all "High" priority, but strictly ordered with distinct fixed priorities)

**Rationale**: 
- This guarantees: read completes before compute; compute completes before apply—within the same 50ms frame
- Minimizes latency by ensuring Sensors task (data acquisition) completes before Control task (computation) starts
- Ensures Control task completes before Actuator task (actuation) starts
- All hard real-time tasks need high priority to meet 50ms deadline
- Soft real-time tasks (Display, Setup) can run at lower priority
- ISR runs at interrupt level (highest priority)

---

## Timing Constraints

### Hard Real-Time Constraints:

- **OS Tick**: 10ms (concrete timing pick)
- **Control Cycle Period**: T_cycle = 50ms (concrete timing pick, ≤ 100ms requirement)
- **ISR Period**: T_ISR = 50ms (concrete timing pick, multiple of OS tick: 10ms × 5)
- **Sensors Period**: T_Sensors = 50ms (concrete timing pick, synchronized with T_ISR)
- **Control Period**: T_Control = 50ms (concrete timing pick, synchronized with T_Sensors)
- **Actuator Period**: T_Actuator = 50ms (concrete timing pick, synchronized with T_Control)
- **Period Alignment**: All hard tasks (ISR, Sensors, Control, Actuator) have periods = 50ms, multiple of OS tick (10ms), no drift
- **Sensors Task Deadline**: Must complete before next timer interrupt (50ms)
- **Control Task Deadline**: Must complete within 50ms from Sensors completion
- **Actuator Task Deadline**: Must complete within 50ms from Control completion

### Control Loop Timing Budget (≤ 50ms):

**Timing Constraint**: ISR_latency + C_sensors + C_control + C_actuator + (enqueue+dequeue) ≤ 50ms

Where (placeholders to be filled after WCETs are measured):
- ISR_latency: Timer interrupt processing time (TBD ms)
- C_sensors: Worst-case execution time of Sensors task (TBD ms)
- C_control: Worst-case execution time of Control task (TBD ms)
- C_actuator: Worst-case execution time of Actuator task (TBD ms)
- (enqueue+dequeue): Message queue post/pend operations overhead (TBD ms)

This ties Requirement 3 to the control-loop constraints from the control lectures (Week 2, Week 8).

### Soft Real-Time Constraints:

- **Display Update Period**: T_Display = 2000ms (2 seconds, locked to OS tick via OSTimeDlyHMSM())
- **Display Task Deadline**: Soft (can tolerate occasional delays)

---

## Safety/Watchdog Timer

### Lightweight Safety/Watchdog Timer

**Purpose**: Detect deadline misses without adding hard dependencies to control loop

**Implementation**:
- **One-Shot Timer Callback**: OS timer callback (non-blocking) that runs periodically
- **Deadline Monitoring**: 
  - Timer callback checks if Control/Actuator tasks completed in last cycle
  - If deadline miss detected → sets event flag (bit #2: Deadline Miss)
- **Setup Task Response**: 
  - Setup task monitors deadline miss event flag
  - On deadline miss → transitions ACC to safe OFF state
- **Non-Blocking**: Callback does not block control loop tasks
- **Rationale**: Adds fault-tolerance without touching < 100ms control loop [Requirement 2 integration]

---

## Schedulability Analysis

### Rate Monotonic Scheduling (RMS) Sanity Check

**Once WCETs (Worst-Case Execution Times) are known**:

1. **Utilization Bound**: Verify total CPU utilization ≤ RMS bound
   - For n tasks: U ≤ n(2^(1/n) - 1)
   - For 3 hard tasks: U ≤ 3(2^(1/3) - 1) ≈ 0.78 (78%)

2. **Utilization Calculation**:
   - U = Σ(Ci / Ti) for all hard tasks
   - Where Ci = worst-case execution time, Ti = period

3. **Fixed-Priority Ordering**: Current priority assignment follows rate-based priority (higher rate = higher priority)

4. **Latency Measurement**: Measure worst-case response times to verify deadlines are met

**Rationale**: Week 8 notes explicitly point to rate-based priority assignment & bounds for schedulability verification

---

## Implementation Notes

### uC/OS-III Functions to Use:

**Semaphores**:
- OSSemCreate() - Create counting semaphore
- OSSemPend() - Wait for semaphore (blocking)
- OSSemPost() - Post semaphore (signal)

**Mutexes**:
- OSMutexCreate() - Create mutex
- OSMutexPend() - Acquire mutex (blocking, with priority inheritance)
- OSMutexPost() - Release mutex

**Message Queues**:
- OSQCreate() - Create message queue
- OSQPost() - Post message to queue
- OSQPend() - Wait for message from queue (blocking)

**Event Flags**:
- OSFlagCreate() - Create event flag group
- OSFlagPost() - Post event flag (set bit)
- OSFlagPend() - Wait for event flag (ANY or ALL)

**Task Semaphores**:
- OSTaskSemPost() - Post to a task's built-in semaphore
- OSTaskSemPend() - Pend on task's own semaphore

**Tasks**:
- OSTaskCreate() - Create task
- OSTimeDlyHMSM() - Delay task execution (hours, minutes, seconds, milliseconds) - for Display task periodic execution
- OSTimeDly() - Delay task execution (ticks)

**Timers**:
- OSTmrCreate() - Create one-shot/periodic timer
- OSTmrStart() - Start timer
- OSTmrCallback() - Timer callback function (non-blocking)

**Memory Partitions**:
- OSMemCreate() - Create memory partition
- OSMemGet() - Allocate memory block
- OSMemPut() - Free memory block

---

## Diagram Legend

- **Circle/Rectangle**: Task
- **Flag**: Semaphore (for timing synchronization)
- **Event Flag Group**: Multi-bit event flags (for status/modes/faults)
- **Lock**: Mutex (for shared resource protection)
- **Queue**: Message Queue (with flow control counting semaphore)
- **Cylinder**: Shared Memory / Data Object
- **Arrow**: Data flow or synchronization signal
- **Dashed Arrow**: Optional communication path
- **Timer**: One-shot/periodic timer (watchdog)

## Diagram Annotations (Required Labels)

1. **Timer Semaphore**: "ISR→Sensors (counting, credit tracking)"
2. **Control Task Semaphore**: "Sensors→Control (task semaphore, unilateral rendez-vous)"
3. **Control → Actuator Queue**: "N=3, bounded + counting semaphore for flow control"
4. **Parameter Memory Block**: "Parameter Block (short, copy-in/out)" with lock icon
5. **Event Flag Group**: Show bit assignments (ACC_ON, ACC_OFF, Deadline Miss, Safe to Actuate)
6. **Watchdog Timer**: One-shot timer callback for deadline miss detection

---

## Next Steps

1. Create detailed ASCII/text diagram showing all tasks, data objects, synchronization, and communication structures
2. Label all components clearly
3. Show data flow directions
4. Indicate task types and priorities
5. Include timing constraints annotations
6. Reference Week 5 lecture notes for synchronization/communication patterns

---

## Summary of Improvements Incorporated

✅ **All hard tasks periodic w.r.t. OS tick** (no drift, periods are multiples of tick)
✅ **Display task uses OSTimeDlyHMSM()** (periodic/absolute delay, spacing doesn't depend on CPU load)
✅ **Task semaphores for task↔task sync** (Sensors→Control uses OSTaskSemPost/Pend, avoids extra kernel objects)
✅ **Semaphore for signaling, mutex for sharing** (confirmed discipline: semaphores from ISRs for signaling, mutexes for shared resources)
✅ **Queue flow-control explicitly implemented** (Control→Actuator queue N=3 + counting semaphore)
✅ **Queue explicitly sized** (N=3 messages, counting semaphore linked to capacity)
✅ **Control loop timing budget** (ISR latency + C_sensors + C_control + C_actuator + queue/dequeue ≤ 100ms)
✅ **Deadlock hygiene** (all tasks acquire resources in same order; time-bounded pends)
✅ **Event flags for multi-event status** (ACC_on_off, fault states, deadline miss)
✅ **Mutex discipline** (short critical sections, copy-in/out, compute outside)
✅ **Controller ordering** (read → compute → output to minimize sample-to-actuate delay)
✅ **Lightweight safety/WD timer** (one-shot timer callback for deadline miss detection)
✅ **Schedulability sanity check** (RMS bound verification once WCETs known)
✅ **Diagram clarifications** (all required labels and annotations specified)

---

## References

- **Week 2 Lecture Notes**: Controller ordering, computational delay management
- **Week 4 Lecture Notes**: Mutexes, priority inheritance, one-shot timers
- **Week 5 Lecture Notes**: Task synchronization and inter-task communication, event flags, flow control
- **Week 8 Lecture Notes**: Task types (hard, soft, periodic, aperiodic, event-driven), RMS scheduling
- **Assignment Document**: Required tasks and system flow
- **Requirement 2 Solution**: Safety modules integration, watchdog timer

