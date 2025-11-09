# Requirement 2: Safety and Fault Tolerance for ACC System

## Introduction

The Adaptive Cruise Control (ACC) system is a safety-critical automotive application where failures can result in severe damage or accidents. As stated in the fault tolerance reading material, "in real-time systems safety and reliability are coupled together" [Fault Tolerance Reading, Page 25]. For safety-critical systems like ACC, the absence of fail-safe states implies that safety can only be ensured through increased reliability. This requirement addresses how to improve the ACC system through hardware and software fault tolerance techniques to make it safe and reliable.

---

## Hardware Improvements

### 1. Sensor Redundancy + Plausibility Checks

**Implementation**: The ACC system employs redundant sensors for critical measurements with explicit plausibility validation to detect and prevent sensor failures from causing unsafe behavior.

**Primary + Backup Sensor Configuration**:
- **Distance Sensor**: Primary radar + Backup lidar
- **Speed Sensor**: Primary wheel encoder + Backup GPS-based speed

**Plausibility Window with Explicit Thresholds**:
- **Headway Bounds**: Distance readings X(n) must be within [X_min, X_max] meters (e.g., 0-200m for typical ACC operation)
- **Rate-of-Change Limits**: The rate of change Δv/Δt cannot exceed physical limits. Specifically, headway cannot shrink faster than X m/s given the vehicle's own speed and the lead vehicle's speed. This prevents unrealistic sudden distance changes that could indicate sensor failure.
- **Cross-Validation**: Primary and backup sensors must agree within a tolerance window (e.g., |Primary - Backup| < 5% of reading or 2 meters, whichever is larger)

**Fail-Safe Transition**: If plausibility checks fail for k consecutive cycles (k=3), the Sensor Health Monitor sets a 'fail-safe' flag for the Safety State Machine. This ensures that transient sensor glitches don't trigger false alarms, while persistent failures are caught promptly.

**Rationale**: As stated in the fault tolerance reading, "if no damage can result when a system enters a fail-safe state just before it fails, then through careful transition to fail-safe state upon a failure, it is possible to turn an extremely unreliable and unsafe system into a safe system" [Fault Tolerance Reading, Page 25]. The plausibility checks ensure that sensor failures are detected before they can cause unsafe control actions.

---

### 2. Built-In Self Test (BIST) + Automatic Reconfiguration

**Implementation**: The system periodically performs self-tests of its components and automatically reconfigures itself when failures are detected.

**Periodic BIST Checks**:
- Sensor health monitoring (signal quality, calibration verification)
- Actuator response verification (throttle/brake response tests)
- Processor self-diagnostics (memory tests, CPU integrity checks)
- Communication interface validation

**Explicit Action on Failure**:
Upon detection of a failure during BIST:
1. **Isolate Failed Unit**: Switch out the faulty component from the active system
2. **Switch In Redundant Component**: Activate the backup component (short-term fault masking)
3. **Automatic Reconfiguration**: System continues operation without manual intervention

**Example**: If the primary radar sensor fails BIST, the system automatically switches to the backup lidar sensor and notifies the Safety State Machine of the redundancy loss, transitioning to Degraded mode.

**Rationale**: As described in the reading material, "in BIST, the system periodically performs self tests of its components. Upon detection of a failure, the system automatically reconfigures itself by switching out of the faulty component and switching in one of the redundant good components" [Fault Tolerance Reading, Page 27]. This provides short-term fault masking suitable for automotive systems where components can be repaired/replaced.

---

### 3. Hardware Watchdog + OS One-Shot Timer Guard

**Implementation**: A dual-layer watchdog system ensures system responsiveness and detects deadline violations.

**Hardware Watchdog Timer**:
- Independent hardware timer that must be reset by the control task within each control cycle
- If not reset within the deadline → hardware-level fail-safe activation

**OS One-Shot Timer Guard** [Week 4 Lecture Notes]:
- One-shot timer created/restarted each control cycle (synchronized with timer interrupt)
- Timer set to deadline Di (≤ 100ms as per assignment requirement)
- Control task must "pet" (reset) the OS timer before expiry by calling a timer reset function

**Deadline Guard Logic**:
- **Hard Timing Constraint**: If Control task hasn't completed and reset the OS timer by t = Di (≤ 100ms), a deadline-miss event is raised
- **State Transition**: Deadline miss → Safety State Machine → Degraded/Fail-Safe state
- **Fail-Safe Activation**: System maintains current speed, prevents acceleration, alerts driver

**Rationale**: One-shot timers are suitable for watchdogs and safeguards [Week 4 Lecture Notes]. The dual-layer approach (hardware + OS) provides redundancy in deadline monitoring, ensuring that even if one watchdog fails, the other can still detect system unresponsiveness.

---

### 4. Redundant Processing Units (Optional - High-End Systems)

**Implementation**: For high-end ACC systems requiring maximum reliability, processor redundancy can be implemented using Static Pairing or Triple Modular Redundancy (TMR).

**Static Pairing Approach** [Fault Tolerance Reading, Page 33]:
- Processors hardwired in pairs
- Each pair runs identical software using identical inputs
- Output comparison: If outputs are identical → pair is functional
- If non-identical outputs detected → at least one processor is faulty
- **Isolation on Discrepancy**: The processor detecting the discrepancy switches off its interface to the rest of the system, isolating the pair

**Caveats**:
- Problems if the interface fails
- Problems if both processors fail identically around the same time
- Interface monitor can be introduced to test interface and monitor each other

**TMR Alternative** [Fault Tolerance Reading, Page 27]:
- Three redundant copies of critical components run concurrently
- Majority voting on outputs masks single component failures

**Rationale**: For automotive systems, static pairing provides a balance between fault tolerance and cost. The reading notes that "the pair runs identical software using identical inputs, and compares the output of each task" [Fault Tolerance Reading, Page 33]. This approach is suitable for systems where complete processor failure is rare but must be tolerated.

---

## Software Improvements

### 1. Recovery Blocks with Explicit Acceptance Test

**Implementation**: The control task uses multiple algorithm implementations (try blocks) with concrete acceptance tests to ensure correct control calculations.

**Try Blocks**:
- **Primary Algorithm**: Standard PID-based control algorithm (equations 1-4 from assignment)
- **Backup Algorithm**: Simplified but different algorithm (e.g., simpler proportional control with conservative gains)

**Explicit Acceptance Test** (concrete criteria applied to both try blocks):
1. **Output Bounded**: dM(n) must be within [dM_min, dM_max] range (e.g., -50% to +30% of current speed)
2. **Monotonic Headway Response**: If X(n) < X(n-1) (distance decreasing), then dM(n) should indicate deceleration (negative or reduced positive value)
3. **Rate-of-Change Limits**: |dM(n) - dM(n-1)| < threshold (e.g., 10% change per cycle maximum) to prevent abrupt control changes

**Fallback Logic**:
- Try primary algorithm → apply acceptance test
- If acceptance test fails → try backup algorithm → apply same acceptance test
- If backup also fails → trigger fail-safe mode (maintain current speed, alert driver)

**Common Test**: The same acceptance test is applied to all try blocks, ensuring consistent validation criteria.

**Rationale**: As described in the reading material, "in the recovery block scheme, the redundant components are called 'try blocks'. Each try block computes the same end result as the others but is intentionally written using a different algorithm compared to the other try blocks" [Fault Tolerance Reading, Page 36]. The acceptance test ensures that only valid control outputs are used, preventing unsafe control actions.

---

### 2. Checkpointing + Roll-Back for Parameter/State Protection

**Implementation**: The system periodically saves its state to stable storage, allowing roll-back recovery if corruption is detected.

**Checkpoint Creation**:
At the end of each control cycle, after a state-check succeeds, the system persists critical state to stable storage:
- **Control Parameters**: Vset, Xset (current speed reference and safe distance)
- **Error History**: Last 3 error values: e(n), e(n-1), e(n-2)
- **Actuator Command**: dM(n) (manipulated variable)
- **Sensor Readings**: X(n), V(n), V(n-1), V(n-2) (current and historical)

**State Check**:
After meaningful progress in computation (end of control cycle), the system validates:
- Parameter values are within expected ranges
- State consistency (e.g., Vset reasonable given Vcruise)
- Data integrity (checksums valid)

**Roll-Back Procedure**:
- On detection of corruption (checksum failure, out-of-range values, inconsistency):
  1. Restore system state from last checkpoint
  2. Drop to Degraded state (reduced functionality but safe)
  3. Initiate fresh computation from checkpointed state
  4. Alert driver of system degradation

**Rationale**: As stated in the reading, "in this scheme as the computation proceeds, the system state is tested each time after some meaningful progress in computation is made. Immediately after a state-check succeeds, the state of the system is backed up on a stable storage. If the next test does not succeed the system can be made to roll back to the last check-pointed state" [Fault Tolerance Reading, Page 38]. This prevents error propagation and allows recovery from transient faults.

---

### 3. N-Version Programming (Narrow Application, with Warning)

**Implementation**: Limited application of N-version programming to one or two critical computations only.

**Limited Scope**:
- Applied only to the control algorithm calculation (dM(n))
- N=2 versions for cost-effectiveness (primary + backup)

**Parallel Execution**:
- Two different versions of the control algorithm run concurrently
- Each version receives identical inputs (sensor data, parameters)
- Results are compared at runtime

**Voting Mechanism**:
- If both versions agree (within tolerance) → use the result
- If versions disagree → trigger fail-safe mode (don't use either result)

**Warning and Limitation**:
The reading material cautions that "the scheme is not very successful in achieving fault-tolerance and the problem can be attributed to 'statistical correlation of failure'—which means that even with independent teams developing different version the versions tend to fail for identical reasons" [Fault Tolerance Reading, Page 35]. Programmers commit errors in parts they perceive as difficult, and what is difficult to one team is difficult to all teams. Therefore:
- **Use Sparingly**: Don't rely on N-version programming alone
- **Combine with Other Techniques**: Use alongside recovery blocks and checkpointing
- **Focus on Simple Computations**: Apply only to well-understood calculations

**Rationale**: N-version programming adapts TMR for software, but has limitations. It should be used as one component of a multi-layered fault tolerance strategy, not as the sole mechanism.

---

### 4. Fault-Tolerant Scheduling with Ghost Copies

**Implementation**: Reserve capacity in the schedule for backup task execution (ghost copies) that can be activated if primary tasks fail.

**Reserve Capacity**:
- Ensure sufficient reserve capacity in the schedule to accommodate ghost copies
- Reserve capacity must be sufficient to continue meeting critical-task deadlines despite failures

**Ghost Copies**:
- Simplified, acceptable backup versions of critical tasks (e.g., Control task)
- Ghost copies are embedded into the schedule alongside primary copies
- Ghosts can be alternative copies that produce poorer but still acceptable results

**Activation**:
- If primary task fails (deadline miss, execution error, processor failure) → activate ghost copy
- Ghost copy executes at the time specified in the ghost schedule
- Primary tasks may be shifted to make room for activated ghosts, but all deadlines must still be met

**Scheduling Conditions** [Fault Tolerance Reading, Page 45]:
- **Condition C1**: Two or more copies (primary or ghost) of the same version must not be scheduled on the same processor
- **Condition C2**: Ghost copies may overlap in schedule only if no other processor carries a copy of those tasks, and primary copies may overlap ghosts only if sufficient slack time exists

**Rationale**: As described in the reading, "use additional ghost copies of tasks, which are embedded into the schedule and activated whenever a processor carrying one of their corresponding primary or previously-activated ghost copies fails" [Fault Tolerance Reading, Page 41]. This ensures that "the fault-tolerant schedule must ensure that, after some time for reacting to the failure(s), the system can still execute nc(i) copies of each version of task Ti, despite the failure of up to nsust processors" [Fault Tolerance Reading, Page 43].

---

## Real-Time Communication and Synchronization

### Bounded Asynchronous Messaging [Week 5]

**Message Queues for Inter-Task Communication**:
- **Sensor → Control**: Message queue for sensor data (X(n), V(n), etc.)
- **Control → Actuator**: Message queue for control commands (dM(n))
- **Asynchronous Operation**: Producers (Sensors task) don't block consumers (Control task), ensuring timing predictability

**Flow Control** [Week 5]:
- **Queue + Counting Semaphore**: Prevents queue overflow when consumer is preempted
- Counting semaphore tracks available queue slots
- Producer waits on semaphore before posting (prevents overflow)
- Consumer posts to semaphore after retrieving message (signals availability)

**Memory Management** [Week 5]:
- **Fixed-Size Memory Partitions**: Deterministic allocation/free for queue message buffers
- Prevents memory fragmentation and ensures predictable timing
- Messages remain "in scope" until consumer completes processing
- Memory partitions allocated from OS memory partition manager

**Rationale**: As discussed in Week 5 lecture notes, "task-to-task communication can involve data transfers such that one task produces data while the other consumes it. Due to time differences in producing and processing times it is possible for the producer to overflow the message queue" [Week 5]. Flow control using queues and counting semaphores solves this problem.

---

### Event Flags for Fault Broadcasting [Week 5]

**Consolidated Fault Signaling**:
- Event flag group used to signal multiple fault conditions to Safety State Machine
- **Bit Assignments**:
  - Bit #0: Sensor fault detected
  - Bit #1: Deadline miss detected
  - Bit #2: Checksum failure (data corruption)
  - Bit #3: Plausibility violation (k consecutive failures)
  - Bit #4: Control algorithm acceptance test failure
  - Bit #5: BIST failure detected

**Usage Patterns** [Week 5]:
- **Status Events**: Safety State Machine monitors flags using non-blocking wait calls (OSFlagPend with timeout=0)
- **Transient Events**: Error Detection module waits for any fault flag using blocking wait (OSFlagPend with OS_OPT_PEND_FLAG_SET_ANY)

**Broadcasting**:
- Multiple safety modules can set different flag bits simultaneously
- Safety State Machine can wait for "any" flag (OS_OPT_PEND_FLAG_SET_ANY) or "all" flags (OS_OPT_PEND_FLAG_SET_ALL) depending on fault severity

**Rationale**: Event flags are used "when a task needs to synchronize with the occurrence of multiple events" [Week 5]. They are suitable for "status events" (monitored non-blocking) and "transient events" (blocking wait) [Week 5, Page 233-234].

---

### Mutex-Protected Shared Safety State [Week 4]

**Shared Resources Requiring Protection**:
- **Safety State Machine State**: Current state (Normal/Degraded/Fail-Safe/Emergency) accessed by multiple tasks
- **Parameter Memory Block**: Shared between Sensors, Control, and Actuator tasks
- **Checkpoint Storage**: Accessed by Error Detection and Recovery modules

**Mutex Implementation**:
- Mutex created for each shared resource (OSMutexCreate)
- Tasks must acquire mutex before accessing shared state (OSMutexPend)
- Mutex released after access complete (OSMutexPost)
- Prevents race conditions and data corruption

**Priority Inheritance**:
- uC/OS-III mutexes support priority inheritance protocol
- Prevents unbounded priority inversion when high-priority tasks wait for mutexes held by low-priority tasks

**Rationale**: As discussed in Week 4, mutexes provide mutual exclusion to prevent corruption of shared data structures. "In a real-time system where semaphores are used for mutual exclusion and synchronization, we must have at least 3 main states: RUN, READY, WAIT" [Week 4]. Mutexes ensure exclusive access to critical sections.

---

## Safety Modules Block Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│              ACC SYSTEM ARCHITECTURE                               │
│         (with Safety & Fault Tolerance Modules)                    │
└─────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────┐
│                        HARDWARE LAYER                               │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐        │
│  │   Primary    │    │    Backup    │    │    Primary    │        │
│  │   Distance   │    │   Distance   │    │     Speed    │        │
│  │   Sensor     │    │   Sensor     │    │    Sensor    │        │
│  │  (Radar)     │    │   (Lidar)    │    │  (Encoder)   │        │
│  └──────┬───────┘    └──────┬───────┘    └──────┬───────┘        │
│         │                   │                    │                 │
│         └───────────────────┴────────────────────┘                 │
│                            │                                       │
│                  ┌─────────▼──────────┐                           │
│                  │  Sensor Health     │                           │
│                  │  Monitor Module    │                           │
│                  │  [Plausibility    │                           │
│                  │   Checks: Headway │                           │
│                  │   bounds, Δv/Δt   │                           │
│                  │   limits, k=3     │                           │
│                  │   consecutive     │                           │
│                  │   failures]       │                           │
│                  └─────────┬──────────┘                           │
│                            │                                       │
│         ┌──────────────────┼──────────────────┐                   │
│         │                  │                  │                   │
│  ┌──────▼──────┐   ┌──────▼──────┐   ┌──────▼──────┐           │
│  │  Hardware    │   │  OS One-Shot │   │  Redundancy │           │
│  │  Watchdog    │   │  Timer Guard │   │  Manager    │           │
│  │  Timer       │   │  [Deadline   │   │  [BIST      │           │
│  │              │   │   Guard:     │   │   Switchover│           │
│  │              │   │   ≤100ms,    │   │   Logic]    │           │
│  │              │   │   Deadline   │   │             │           │
│  │              │   │   Miss →     │   │             │           │
│  │              │   │   Fail-Safe] │   │             │           │
│  └──────────────┘   └──────────────┘   └──────────────┘           │
│                                                                     │
└────────────────────────────┬───────────────────────────────────────┘
                             │
┌────────────────────────────▼───────────────────────────────────────┐
│                        SOFTWARE LAYER                               │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐ │
│  │              APPLICATION TASKS                               │ │
│  │                                                              │ │
│  │  ┌────────┐    ┌────────┐    ┌────────┐    ┌────────┐       │ │
│  │  │ Setup  │    │Sensors │    │Control │    │Actuator│       │ │
│  │  │ Task   │    │ Task   │    │ Task   │    │ Task   │       │ │
│  │  └────────┘    └───┬────┘    └───┬────┘    └───┬────┘       │ │
│  │                   │              │              │            │ │
│  │    ┌──────────────▼──────────────▼──────────────▼─────┐    │ │
│  │    │  Message Queues + Counting Semaphores            │    │ │
│  │    │  [Flow Control: Sensor→Control, Control→Actuator]│    │ │
│  │    │  [Memory Partitions for Message Buffers]         │    │ │
│  │    └───────────────────────────────────────────────────┘    │ │
│  └─────────────────────────────────────────────────────────────┘ │
│                             │                                      │
│  ┌──────────────────────────▼──────────────────────────────────┐  │
│  │         SAFETY & FAULT TOLERANCE MODULES                    │  │
│  ├──────────────────────────────────────────────────────────────┤  │
│  │                                                              │  │
│  │  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │  │
│  │  │ Data         │  │ Error        │  │ Fail-Safe    │      │  │
│  │  │ Integrity    │  │ Detection &   │  │ Controller   │      │  │
│  │  │ Checker      │  │ Recovery     │  │              │      │  │
│  │  │ [Checksums,  │  │ [Recovery    │  │ [Maintain    │      │  │
│  │  │  Range       │  │  Blocks,     │  │  Speed,      │      │  │
│  │  │  Checks]     │  │  Checkpoint/  │  │  Disengage]  │      │  │
│  │  │              │  │  Roll-Back]   │  │              │      │  │
│  │  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘      │  │
│  │         │                  │                  │              │  │
│  │         └──────────────────┴──────────────────┘              │  │
│  │                            │                                  │  │
│  │                  ┌──────────▼──────────┐                      │  │
│  │                  │  Safety State       │                      │  │
│  │                  │  Machine            │                      │  │
│  │                  │  [Normal → Degraded │                      │  │
│  │                  │   → Fail-Safe →     │                      │  │
│  │                  │   Emergency]        │                      │  │
│  │                  │  [Receives Event    │                      │  │
│  │                  │   Flags: sensor     │                      │  │
│  │                  │   fault, deadline   │                      │  │
│  │                  │   miss, checksum    │                      │  │
│  │                  │   failure,          │                      │  │
│  │                  │   plausibility      │                      │  │
│  │                  │   violation]       │                      │  │
│  │                  └─────────────────────┘                      │  │
│  └──────────────────────────────────────────────────────────────┘  │
│                             │                                      │
│  ┌──────────────────────────▼──────────────────────────────────┐  │
│  │         REAL-TIME KERNEL (uC/OS-III)                        │  │
│  │  [Task Scheduling, Mutexes, Semaphores, Message Queues,    │  │
│  │   Event Flags, Memory Partitions, One-Shot Timers]          │  │
│  └──────────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────────┘

Legend:
  → Data Flow
  → Event Flag Signals (fault notifications)
  → Mutex-Protected Access
  [ ] Module annotations and key features
```

---

## Safety Modules and Their Roles

### Module 1: Sensor Health Monitor

**Role**: Continuously monitors sensor health, validates data plausibility, and triggers fail-safe transitions when sensor reliability degrades.

**Key Functions**:
- **Plausibility Checks**: Validates sensor readings against explicit thresholds:
  - Headway bounds: X(n) ∈ [X_min, X_max] meters
  - Rate-of-change limits: |ΔX/Δt| < max_rate (headway cannot shrink faster than physically possible)
  - Cross-validation: Primary vs backup sensors must agree within tolerance
- **Consecutive Failure Tracking**: Counts k consecutive plausibility failures (k=3 threshold)
- **Fail-Safe Trigger**: If k ≥ 3 consecutive failures → sets fail-safe flag → Safety State Machine transitions to Fail-Safe state
- **Sensor Confidence Levels**: Provides confidence scores to other modules for decision-making

**Integration**: Validates sensor data before Sensors task stores it in parameter memory block. Routes out-of-window readings to fail-safe transition rather than allowing them to propagate to control calculations.

---

### Module 2: Watchdog Monitor

**Role**: Ensures system responsiveness through dual-layer monitoring (hardware + OS-level) and detects deadline violations.

**Key Functions**:
- **Hardware Watchdog**: Independent hardware timer that must be reset by control task each cycle
- **OS One-Shot Timer Guard** [Week 4]: One-shot timer restarted each control cycle, set to deadline Di (≤ 100ms)
- **Deadline Guard (Hard Timing)**: Monitors if Control task completes by deadline
  - If Control hasn't "petted" OS timer by t = Di → deadline miss detected
  - Raises deadline-miss event → Safety State Machine
  - Triggers transition to Degraded/Fail-Safe state
- **System Health Status**: Provides continuous monitoring of system responsiveness

**Integration**: Monitors Control task execution time. Deadline miss events posted via event flags to Safety State Machine. Ensures hard real-time constraints are met.

---

### Module 3: Fail-Safe Controller

**Role**: Implements fail-safe behavior when faults are detected, ensuring the system transitions to a safe state.

**Key Functions**:
- **Fault Severity Assessment**: Evaluates fault signals from all safety modules
- **Fail-Safe Commands**:
  - Maintain current speed (no acceleration) for moderate faults
  - Gradual speed reduction (not abrupt braking) for more severe faults
  - Immediate ACC disengagement for critical faults
- **Driver Alert Activation**: Visual and audible alerts when fail-safe mode activates
- **State Transition**: Coordinates with Safety State Machine for proper state transitions

**Integration**: Receives fault signals via event flags. Overrides Actuator task commands when faults detected. Implements the fail-safe state concept from the reading material [Fault Tolerance Reading, Page 25].

---

### Module 4: Data Integrity Checker

**Role**: Validates data integrity and prevents corruption from propagating through the system.

**Key Functions**:
- **Checksums**: CRC checksums on critical data structures (parameter memory block, checkpoint data)
- **Range Validation**: Ensures all values within expected bounds (Vset, Xset, dM(n), etc.)
- **Consistency Checks**: Validates logical consistency (e.g., speed vs distance rate of change)
- **Memory Corruption Detection**: Detects memory corruption before data is used

**Integration**: Validates parameter memory block before Control task reads it. Validates checkpoint data before roll-back operations. Posts checksum failure events via event flags to Safety State Machine.

---

### Module 5: Redundancy Manager

**Role**: Manages redundant components (sensors, processors) and coordinates switchover operations.

**Key Functions**:
- **Component Health Assessment**: Monitors health of primary and backup components
- **BIST Coordination**: Coordinates Built-In Self Test operations
- **Switchover Logic**: 
  - Isolates failed unit (switches out faulty component)
  - Switches in redundant component (short-term fault masking)
  - Ensures seamless transition without service interruption
- **Backup Component Testing**: Periodically tests backup components to ensure readiness

**Integration**: Works with Sensor Health Monitor for sensor redundancy. Coordinates with BIST for automatic reconfiguration [Fault Tolerance Reading, Page 27]. Notifies Safety State Machine of redundancy loss (transition to Degraded mode).

---

### Module 6: Error Detection and Recovery

**Role**: Detects errors in task execution and initiates recovery procedures.

**Key Functions**:
- **Recovery Block Acceptance Tests**: Implements explicit acceptance test criteria:
  - Output bounded: dM(n) ∈ [dM_min, dM_max]
  - Monotonic headway response validation
  - Rate-of-change limits: |dM(n) - dM(n-1)| < threshold
- **Checkpoint Management**: 
  - Creates checkpoints at end of each control cycle (after state-check succeeds)
  - Persists: Vset, Xset, e(n), e(n-1), e(n-2), dM(n), sensor readings
- **Roll-Back Procedure**: On corruption detection → restores from checkpoint → transitions to Degraded state
- **Event Flag Posting**: Posts fault flags (sensor fault, deadline miss, checksum failure) to Safety State Machine

**Integration**: Monitors Control task output and performs acceptance tests. Manages checkpoint/roll-back operations. Coordinates with Safety State Machine for error recovery and state transitions.

---

### Module 7: Safety State Machine

**Role**: Central coordinator that manages system safety states and coordinates all safety modules.

**Key Functions**:
- **State Management**: Manages four safety states:
  - **NORMAL**: All systems operational, full ACC functionality
  - **DEGRADED**: Some redundancy lost, reduced functionality but still safe
    - Trigger: Single sensor failure, deadline miss, or data corruption (after roll-back)
  - **FAIL-SAFE**: Critical fault detected, minimal safe operation
    - Trigger: k consecutive plausibility failures, multiple sensor failures, or control algorithm failure
    - Action: Maintain current speed, no acceleration, alert driver
  - **EMERGENCY**: Multiple failures, immediate ACC disengagement
    - Trigger: Complete system failure or multiple critical faults
    - Action: Immediate ACC disengagement, driver must take control

- **Event Flag Processing**: Receives consolidated fault signals via event flags:
  - Sensor fault (bit #0)
  - Deadline miss (bit #1)
  - Checksum failure (bit #2)
  - Plausibility violation (bit #3)
  - Control algorithm failure (bit #4)
  - BIST failure (bit #5)

- **State Transition Logic**: Implements measurable triggers:
  - Deadline miss → Degraded/Fail-Safe
  - Sensor implausible → redundancy switch + Degraded/Fail-Safe
  - Multiple faults → Emergency

**Integration**: Central hub receiving event flags from all safety modules. Coordinates state transitions. Controls Fail-Safe Controller activation. Manages mutex-protected shared state.

---

## Fail-Safe Behavior and State Transitions

**Fail-Safe State Concept** [Fault Tolerance Reading, Page 25]:
A fail-safe state is "a state which if entered when the system fails, no damage would result." For the ACC system, the fail-safe state is:
- Maintain current speed (no acceleration)
- Prevent unsafe deceleration (gradual reduction only)
- Alert driver to take control
- Disengage ACC if driver doesn't respond

**State Transition Examples**:
1. **Normal → Degraded**: Single sensor failure detected → switch to backup sensor → continue operation with reduced confidence
2. **Degraded → Fail-Safe**: k consecutive plausibility failures → maintain speed, alert driver
3. **Any State → Emergency**: Multiple critical failures → immediate ACC disengagement

**Rationale**: "If no damage can result when a system enters a fail-safe state just before it fails, then through careful transition to fail-safe state upon a failure, it is possible to turn an extremely unreliable and unsafe system into a safe system" [Fault Tolerance Reading, Page 25].

---

## Summary

This fault tolerance design incorporates multiple layers of hardware and software redundancy, explicit safety thresholds, and real-time communication mechanisms aligned with course materials. The system ensures that safety-critical ACC operations can tolerate component failures while maintaining hard real-time constraints (< 100ms control cycle). All safety modules work together through the Safety State Machine to provide graceful degradation and fail-safe behavior, ensuring that the system transitions to safe states when faults are detected, preventing damage and maintaining driver safety.

