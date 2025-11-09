# Requirement 2 Plan: Safety and Fault Tolerance for ACC System

## Assignment Requirements
- **Weight**: 2 marks
- **Task**: Design safety and fault tolerance features (hardware and software)
- **Deliverables**:
  1. Discussion of system improvements (hardware and software)
  2. Specific features for safety and reliability (fault tolerance)
  3. General block diagram with safety modules
  4. Brief explanation of each module's role

---

## Plan Structure

### Phase 1: Analysis of ACC System Critical Components
**Objective**: Identify components that require fault tolerance

**Critical Components to Protect**:
1. **Sensors** (Distance, Speed, Camera, GPS)
   - Single point of failure risk
   - Sensor degradation/failure scenarios
   - Real-time data acquisition critical

2. **Control Task**
   - Safety-critical calculations
   - Hard deadline (< 100ms)
   - Must prevent incorrect acceleration/deceleration

3. **Actuator System**
   - Directly controls vehicle speed
   - Failure could cause unsafe acceleration or brake failure
   - Must have fail-safe mechanisms

4. **Processor/Computing Unit**
   - Single processor failure = complete system failure
   - Need redundancy or graceful degradation

5. **Communication Infrastructure**
   - Inter-task communication (semaphores, queues, shared memory)
   - Data corruption risks
   - Synchronization failures

---

### Phase 2: Hardware Fault Tolerance Design

**Based on Reading Material Techniques**:

#### 2.1 Sensor Redundancy + Plausibility Checks (Hardware)
- **Technique**: Multiple sensor units with explicit plausibility validation
- **Implementation**:
  - **Primary + Backup sensors**: 
    - Distance sensor: Primary radar + Backup lidar
    - Speed sensor: Primary wheel encoder + Backup GPS-based speed
  - **Plausibility Window** (explicit thresholds):
    - **Headway bounds**: Distance readings must be within [X_min, X_max] meters
    - **Rate-of-change limits**: Δv/Δt cannot exceed physical limits (e.g., headway cannot shrink faster than X m/s given own/lead speeds)
    - **Cross-validation**: Primary and backup sensors must agree within tolerance window
  - **Fail-Safe Transition**: If plausibility fails for k consecutive cycles (e.g., k=3) → set 'fail-safe' flag for Safety State Machine
  - **Rationale**: Safety and reliability are coupled; entering fail-safe state prevents damage [Fault Tolerance Reading, Page 25]

#### 2.2 Built-In Self Test (BIST) + Automatic Reconfiguration
- **Technique**: Periodic self-testing with automatic reconfiguration [Fault Tolerance Reading, Page 27]
- **Implementation**:
  - **Periodic BIST checks**: Sensor health monitoring, calibration verification, actuator response tests, processor self-diagnostics
  - **Explicit Action on Failure**: 
    - Isolate failed unit (switch out faulty component)
    - Switch in redundant component (short-term fault masking)
    - Automatic reconfiguration without manual intervention
  - **Rationale**: BIST enables automatic reconfiguration by switching out faulty components and switching in redundant good components [Fault Tolerance Reading, Page 27]

#### 2.3 Hardware Watchdog + OS One-Shot Timer Guard
- **Technique**: Dual-layer watchdog system (hardware + OS-level)
- **Implementation**:
  - **Hardware Watchdog Timer**: Independent hardware timer, must be reset by control task within deadline
  - **OS One-Shot Timer Guard** [Week 4 Notes]:
    - One-shot timer restarted each control cycle
    - Acts as deadline guard: monitors Control task completion
    - **Deadline Guard Logic**: If Control hasn't "petted" the OS timer by t = Di (≤ 100ms period from Req 3), raise a deadline-miss event → Safety State Machine → Degraded/Fail-Safe
    - On expiry: raise "deadline miss" fault to Safety State Machine
  - **Rationale**: One-shot timers are suitable for watchdogs/safeguards [Week 4 Lecture Notes]

#### 2.4 Redundant Processing Units (Optional - for high-end systems)
- **Technique**: Static Pairing or TMR [Fault Tolerance Reading, Pages 33, 27]
- **Implementation**:
  - **Static Pairing**: Processors hardwired in pairs, run identical software with identical inputs
  - **Output Comparison**: Compare outputs of each processor; if non-identical → indication of fault
  - **Isolation on Discrepancy**: Processor detecting discrepancy switches off interface, isolating the pair
  - **Caveat**: Problems if interface fails or both processors fail identically around same time [Fault Tolerance Reading, Page 33]
  - **TMR Alternative**: Triple Modular Redundancy with majority voting for critical components

---

### Phase 3: Software Fault Tolerance Design

**Based on Reading Material Techniques**:

#### 3.1 Recovery Blocks with Explicit Acceptance Test (Software)
- **Technique**: Multiple algorithm implementations with concrete acceptance tests [Fault Tolerance Reading, Page 36]
- **Implementation**:
  - **Try Blocks**: Control task has Primary algorithm + Backup algorithm (intentionally different algorithm)
  - **Explicit Acceptance Test** (concrete criteria):
    - Output bounded: dM(n) within [dM_min, dM_max] range
    - Monotonic headway response: If X(n) < X(n-1), then dM(n) should indicate deceleration
    - Rate-of-change limits: |dM(n) - dM(n-1)| < threshold
  - **Fallback Logic**: If primary fails acceptance test → try backup algorithm
  - **Common Test**: Same acceptance test applied to all try blocks
  - **Rationale**: Recovery blocks use try blocks with acceptance tests; if test fails, next try block is tried [Fault Tolerance Reading, Page 36]

#### 3.2 Checkpointing + Roll-Back for Parameter/State Protection
- **Technique**: Save system state periodically with roll-back capability [Fault Tolerance Reading, Page 38]
- **Implementation**:
  - **Checkpoint Creation**: Persist critical state each cycle after state-check succeeds:
    - Vset, Xset (current speed reference and safe distance)
    - Last 3 error values: e(n), e(n-1), e(n-2)
    - Actuator command: dM(n)
    - Sensor readings: X(n), V(n), V(n-1), V(n-2)
  - **State Check**: Validate state after meaningful progress (end of control cycle)
  - **Roll-Back Procedure**: On detection of corruption → roll back to last checkpoint and drop to Degraded state
  - **Fresh Computation**: After roll-back, initiate fresh computation from checkpointed state
  - **Rationale**: System state backed up on stable storage; if next test fails, roll back to last checkpoint [Fault Tolerance Reading, Page 38]

#### 3.3 N-Version Programming (Narrow Application, with Warning)
- **Technique**: Multiple implementations with voting [Fault Tolerance Reading, Page 35]
- **Implementation**:
  - **Limited Scope**: Apply only to one or two critical computations (e.g., control algorithm calculation)
  - **Parallel Execution**: N different versions (N=2 for cost-effectiveness) run concurrently
  - **Voting**: Results subject to voting; majority result accepted
  - **Warning**: Note the reading's caution on "statistical correlation of failures" - even independent teams tend to fail for identical reasons in complex/least-understood parts
  - **Use Sparingly**: Don't rely on N-version programming alone; combine with other techniques
  - **Rationale**: N-version programming adapts TMR for software, but has limitations due to statistical correlation [Fault Tolerance Reading, Page 35]

#### 3.4 Fault-Tolerant Scheduling with Ghost Copies
- **Technique**: Ghost copies of tasks embedded in schedule [Fault Tolerance Reading, Pages 41-47]
- **Implementation**:
  - **Reserve Capacity**: Ensure sufficient reserve capacity for ghost copies
  - **Ghost Copies**: Simplified, acceptable backup versions of critical tasks (e.g., Control task)
  - **Embedded Schedule**: Ghost copies embedded into schedule alongside primary copies
  - **Activation**: If primary task fails → activate ghost copy (simplified but acceptable)
  - **Deadline Guarantee**: Ghost schedule ensures deadlines continue to be met despite failures
  - **Conditions**: Ghost copies must satisfy conditions C1 and C2 (no same-version copies on same processor, overlapping requirements) [Fault Tolerance Reading, Page 45]
  - **Rationale**: Ghost copies activated when processor carrying primary copy fails; system continues to meet critical-task deadlines [Fault Tolerance Reading, Page 41]

---

### Phase 4: Safety Modules Design

**Safety Modules to Include in Block Diagram**:

#### Module 1: **Sensor Health Monitor**
- **Role**: Continuously monitors sensor health, data validity, and plausibility
- **Functions**:
  - Detects sensor failures (no signal, out-of-range values)
  - **Plausibility Checks** (explicit thresholds):
    - Headway bounds: X(n) within [X_min, X_max]
    - Rate-of-change limits: Δv/Δt within physical limits
    - Cross-validation: Primary vs backup agreement within tolerance
  - **Fail-Safe Trigger**: If plausibility fails for k consecutive cycles (k=3) → set 'fail-safe' flag for Safety State Machine
  - Flags sensor degradation
  - Provides sensor confidence levels
  - Routes out-of-window readings to fail-safe transition

#### Module 2: **Watchdog Monitor**
- **Role**: Ensures system responsiveness with dual-layer monitoring
- **Functions**:
  - **Hardware Watchdog**: Independent hardware timer, must be reset by control task
  - **OS One-Shot Timer Guard** [Week 4]: One-shot timer restarted each control cycle
  - **Deadline Guard**: Monitors if Control task completes by deadline Di (≤ 100ms)
  - **Deadline Miss Detection**: If Control hasn't "petted" OS timer by deadline → raise deadline-miss event
  - **Fail-Safe Trigger**: Deadline miss → Safety State Machine → Degraded/Fail-Safe state
  - Triggers fail-safe if system becomes unresponsive

#### Module 3: **Fail-Safe Controller**
- **Role**: Implements fail-safe behavior when faults detected
- **Functions**:
  - Activates when fault detected
  - Maintains current speed (no acceleration)
  - Gradually reduces speed if unsafe
  - Disengages ACC and alerts driver
  - Implements fail-safe state transition

#### Module 4: **Data Integrity Checker**
- **Role**: Validates data integrity and prevents corruption
- **Functions**:
  - Checksums on critical data structures
  - Validates sensor readings (range checks, rate-of-change limits)
  - Detects communication errors
  - Prevents use of corrupted data

#### Module 5: **Redundancy Manager**
- **Role**: Manages redundant components and switching
- **Functions**:
  - Coordinates primary/backup sensor selection
  - Manages processor redundancy (if implemented)
  - Handles switchover logic
  - Monitors backup component health

#### Module 6: **Error Detection and Recovery**
- **Role**: Detects errors and initiates recovery
- **Functions**:
  - Implements recovery block acceptance tests
  - Manages checkpoint/roll-back operations
  - Coordinates error recovery procedures
  - Logs errors for diagnostics

#### Module 7: **Safety State Machine**
- **Role**: Manages system safety states
- **Functions**:
  - Normal operation state
  - Degraded operation state (reduced functionality)
  - Fail-safe state (minimal safe operation)
  - Emergency shutdown state
  - State transition logic

---

### Phase 5: Block Diagram Structure

**Proposed Block Diagram Layout** (with explicit annotations for rubric):

**Key Annotations Required**:
- OS one-shot timer watchdog (under "Watchdog Monitor")
- Event-flag lines feeding Safety State Machine
- Queues + counting semaphores on Sensor→Control, Control→Actuator paths
- Memory partition annotation on queue-passed buffers
- Plausibility check thresholds
- Deadline guard timing (< 100ms)

```
┌─────────────────────────────────────────────────────────────┐
│                    ACC SYSTEM ARCHITECTURE                  │
│                  (with Safety & Fault Tolerance)            │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│                    HARDWARE LAYER                            │
├─────────────────────────────────────────────────────────────┤
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐     │
│  │ Primary  │  │ Backup   │  │ Primary │  │ Backup   │     │
│  │ Distance │  │ Distance │  │ Speed   │  │ Speed   │     │
│  │ Sensor   │  │ Sensor   │  │ Sensor  │  │ Sensor  │     │
│  └────┬─────┘  └────┬─────┘  └────┬────┘  └────┬────┘     │
│       │             │              │             │          │
│       └─────────────┴──────────────┴─────────────┘          │
│                        │                                      │
│              ┌─────────▼──────────┐                          │
│              │  Sensor Health     │                          │
│              │  Monitor Module    │                          │
│              └─────────┬──────────┘                          │
│                        │                                      │
│  ┌─────────────────────┼─────────────────────┐              │
│  │  ┌──────────────┐   │   ┌──────────────┐ │              │
│  │  │  Watchdog    │   │   │  Redundancy   │ │              │
│  │  │  Timer       │   │   │  Manager      │ │              │
│  │  └──────────────┘   │   └──────────────┘ │              │
└──┼─────────────────────┼─────────────────────┼──────────────┘
   │                     │                     │
┌──▼─────────────────────▼─────────────────────▼──────────────┐
│                    SOFTWARE LAYER                            │
├──────────────────────────────────────────────────────────────┤
│                                                              │
│  ┌────────────────────────────────────────────────────┐    │
│  │         APPLICATION TASKS                         │    │
│  │  ┌──────┐  ┌──────┐  ┌──────┐  ┌──────┐          │    │
│  │  │Setup │  │Sensors│  │Control│  │Actuator│        │    │
│  │  └──────┘  └───┬───┘  └───┬───┘  └───┬───┘          │    │
│  │                │          │          │              │    │
│  │    ┌───────────▼──────────▼──────────▼──────┐     │    │
│  │    │ Message Queues + Counting Semaphores   │     │    │
│  │    │ (Flow Control, Async Communication)    │     │    │
│  │    │ [Memory Partitions for Buffers]        │     │    │
│  │    └────────────────────────────────────────┘     │    │
│  └────────────────────────────────────────────────────┘    │
│                        │                                      │
│  ┌─────────────────────▼──────────────────────────────────┐ │
│  │         SAFETY & FAULT TOLERANCE MODULES                │ │
│  ├─────────────────────────────────────────────────────────┤ │
│  │ ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  │ │
│  │ │ Data         │  │ Error        │  │ Fail-Safe    │  │ │
│  │ │ Integrity    │  │ Detection &   │  │ Controller   │  │ │
│  │ │ Checker      │  │ Recovery     │  │              │  │ │
│  │ └──────────────┘  └──────────────┘  └──────────────┘  │ │
│  │                                                         │ │
│  │ ┌───────────────────────────────────────────────────┐  │ │
│  │ │         Safety State Machine                      │  │ │
│  │ │  (Normal → Degraded → Fail-Safe → Emergency)      │  │ │
│  │ │  [Receives Event Flags: sensor fault, deadline     │  │ │
│  │ │   miss, checksum failure, plausibility violation] │  │ │
│  │ └───────────────────────────────────────────────────┘  │ │
│  └─────────────────────────────────────────────────────────┘ │
│                        │                                      │
│  ┌─────────────────────▼──────────────────────────────────┐ │
│  │              REAL-TIME KERNEL (uC/OS-III)              │ │
│  │  (Task Scheduling, Synchronization, Communication)     │ │
│  └─────────────────────────────────────────────────────────┘ │
└──────────────────────────────────────────────────────────────┘
```

---

### Phase 6: Implementation Details for Each Module

#### 6.1 Sensor Health Monitor
- **Inputs**: All sensor readings (primary + backup)
- **Outputs**: Sensor status, confidence levels, validated sensor data, plausibility flags
- **Algorithms**:
  - **Range checking**: Sensor values within expected bounds [X_min, X_max]
  - **Plausibility Thresholds** (measurable):
    - Headway bounds: X(n) ∈ [X_min, X_max] meters
    - Rate-of-change limits: |ΔX/Δt| < max_rate (headway cannot shrink faster than X m/s given speeds)
    - Cross-validation: |Primary - Backup| < tolerance_window
  - **Consecutive Failure Counter**: Track k consecutive plausibility failures
  - **Fail-Safe Trigger**: If k ≥ 3 consecutive failures → set fail-safe flag → Safety State Machine
  - Signal quality assessment

#### 6.2 Watchdog Monitor
- **Inputs**: Control task completion signals, system clock, OS timer
- **Outputs**: System health status, timeout alerts, deadline-miss events
- **Algorithms**:
  - **Hardware Watchdog**: Timer reset mechanism (must be reset every control cycle)
  - **OS One-Shot Timer Guard** [Week 4]:
    - One-shot timer created/restarted each control cycle
    - Timer set to deadline Di (≤ 100ms from assignment requirement)
    - Control task must "pet" timer before expiry
  - **Deadline Guard (Hard Timing)**:
    - If Control hasn't completed by t = Di → deadline miss detected
    - Raise deadline-miss event → Safety State Machine
    - Transition to Degraded/Fail-Safe state
  - Failure detection (timeout = system failure)

#### 6.3 Fail-Safe Controller
- **Inputs**: Fault signals from all safety modules, current system state
- **Outputs**: Fail-safe commands (maintain speed, reduce speed, disengage ACC)
- **Algorithms**:
  - Fault severity assessment
  - Gradual speed reduction (not abrupt braking)
  - Driver alert activation
  - ACC disengagement logic

#### 6.4 Data Integrity Checker
- **Inputs**: All data structures (parameter memory block, sensor data, control outputs)
- **Outputs**: Data validity flags, checksum verification results
- **Algorithms**:
  - CRC checksums on critical data
  - Range validation
  - Consistency checks (e.g., speed vs distance rate of change)
  - Memory corruption detection

#### 6.5 Redundancy Manager
- **Inputs**: Status from all redundant components
- **Outputs**: Active component selection, switchover commands
- **Algorithms**:
  - Component health assessment
  - Switchover decision logic
  - Seamless transition (no interruption in service)
  - Backup component testing

#### 6.6 Error Detection and Recovery
- **Inputs**: System state, task execution results, acceptance test results
- **Outputs**: Error flags, recovery actions, checkpoint restore commands, event flags
- **Algorithms**:
  - **Recovery Block Acceptance Tests** (explicit criteria):
    - Output bounded: dM(n) ∈ [dM_min, dM_max]
    - Monotonic headway response validation
    - Rate-of-change limits: |dM(n) - dM(n-1)| < threshold
  - **Checkpoint Creation**: End of each control cycle after state-check succeeds
    - Persist: Vset, Xset, e(n), e(n-1), e(n-2), dM(n), sensor readings
  - **Roll-Back Procedure**: On corruption detection → restore from checkpoint → Degraded state
  - **Event Flag Posting**: Post fault flags (sensor fault, deadline miss, checksum failure) to Safety State Machine
  - Error logging

#### 6.7 Safety State Machine
- **Inputs**: Event flags (sensor fault, deadline miss, checksum failure, plausibility violation), all fault signals, system health status
- **Outputs**: Current safety state, state transition commands, fail-safe activation signals
- **States**:
  - **NORMAL**: All systems operational, full ACC functionality
  - **DEGRADED**: Some redundancy lost, reduced functionality but still safe
    - Trigger: Single sensor failure, deadline miss, or data corruption (after roll-back)
  - **FAIL-SAFE**: Critical fault detected, minimal safe operation
    - Trigger: k consecutive plausibility failures, multiple sensor failures, or control algorithm failure
    - Action: Maintain current speed, no acceleration, alert driver
  - **EMERGENCY**: Multiple failures, immediate ACC disengagement
    - Trigger: Complete system failure or multiple critical faults
    - Action: Immediate ACC disengagement, driver must take control
- **State Transitions**: Measurable triggers (deadline miss → Degraded/Fail-Safe; sensor implausible → redundancy switch + Degraded/Fail-Safe)

---

### Phase 7: Real-Time Considerations

**Timing Constraints for Safety Modules**:
- **Sensor Health Monitor**: Must complete within sensor reading period (< 100ms)
- **Watchdog Monitor**: Continuous monitoring, interrupt-driven
- **Fail-Safe Controller**: Hard deadline (< 50ms) for critical faults
- **Data Integrity Checker**: Must complete before data use (< 10ms)
- **Error Detection**: Must not delay control cycle (< 100ms total)

**Task Priorities**:
- **Highest**: Fail-Safe Controller (safety-critical)
- **High**: Watchdog Monitor, Error Detection
- **Medium**: Sensor Health Monitor, Data Integrity Checker
- **Lower**: Redundancy Manager (background operation)

**Synchronization Requirements** [Week 4, Week 5]:
- **Mutex-Protected Shared Safety State** [Week 4]: Guard shared state (Safety State Machine state, parameter memory block) with mutexes to prevent corruption
- **Bounded Asynchronous Messaging** [Week 5]: 
  - Message queues / task message queues for Sensor→Control→Actuator paths
  - Producers don't block consumers (asynchronous communication)
  - Flow control: Queue + counting semaphore to prevent overflows when consumer preempted
  - Fixed-size memory partitions for deterministic alloc/free (keep data "in scope" safely)
- **Event Flags for Fault Broadcasting** [Week 5]:
  - Consolidated fault/status signaling to Safety State Machine
  - Bits for: sensor fault, deadline miss, checksum failure, plausibility violation
  - Used for status events (non-blocking) and transient events (blocking)
- **Semaphores**: For task synchronization (sensor data ready, control cycle complete)

---

### Phase 8: Integration with ACC System

**How Safety Modules Integrate**:
1. **Sensor Health Monitor** → Validates sensor data before use by Sensors task
2. **Data Integrity Checker** → Validates parameter memory block before Control task reads
3. **Error Detection** → Monitors Control task output, performs acceptance test
4. **Fail-Safe Controller** → Overrides Actuator commands if fault detected
5. **Watchdog Monitor** → Monitors entire system, triggers fail-safe if timeout
6. **Safety State Machine** → Coordinates all safety modules, manages system state

---

## Deliverable Structure

### Section 1: Introduction (2-3 sentences)
- ACC is a safety-critical system requiring fault tolerance
- Brief overview of approach

### Section 2: Hardware Improvements (1-1.5 pages)
- Sensor redundancy
- BIST implementation
- Watchdog timer
- Optional: Processor redundancy

### Section 3: Software Improvements (1-1.5 pages)
- Recovery blocks for control algorithm
- Checkpointing and roll-back
- Fault-tolerant scheduling
- Data integrity mechanisms

### Section 4: Safety Modules Block Diagram (1 page)
- Visual diagram showing all modules
- Clear connections and data flow
- Integration with ACC tasks

### Section 5: Module Descriptions (1-2 pages)
- Brief explanation of each module's role
- Key functions
- Real-time considerations

### Section 6: Fail-Safe Behavior (0.5 page)
- What happens when faults detected
- State transitions
- Driver notification

---

## Key Points to Emphasize

✅ **Safety-Critical Nature**: ACC failures can cause accidents
✅ **Fail-Safe States**: System must transition to safe state on failure [Fault Tolerance Reading, Page 25]
✅ **Redundancy**: Multiple layers of redundancy (sensors, algorithms, processors)
✅ **Real-Time Constraints**: All safety mechanisms must meet timing deadlines (< 100ms control cycle)
✅ **Graceful Degradation**: System should degrade gracefully, not fail catastrophically
✅ **Driver Awareness**: Driver must be notified of system status and faults
✅ **Measurable Thresholds**: Explicit plausibility checks, deadline guards, acceptance test criteria
✅ **Course Material Alignment**: References to Week 4 (one-shot timers, mutexes), Week 5 (event flags, message queues, flow control)
✅ **Reading Material Citations**: BIST, TMR, Static Pairing, Recovery Blocks, Checkpointing, N-version programming, Ghost copies

---

## References to Use

1. **Fault Tolerance Reading Material**:
   - BIST, TMR, Static Pairing (hardware)
   - N-version programming, Recovery blocks, Checkpointing (software)
   - Fail-safe states concept

2. **Lecture Materials**:
   - **Week 4**: One-shot timers for watchdogs/safeguards, mutexes for resource sharing
   - **Week 5**: Event flags (status and transient events), message queues, task message queues, flow control (queue + counting semaphore), fixed-size memory partitions
   - **Week 7**: Hard real-time kernel design
   - **Week 8**: Complex real-time applications, safety considerations

3. **Assignment Context**:
   - ACC system requirements from Requirement 1
   - Control task timing constraints (< 100ms)
   - Task structure from Requirement 3 (to be designed)

---

## Rubric Alignment Checklist

**Requirement 2 asks for**:
- ✅ **(i) Discussion of HW/SW improvements** using Fault Tolerance reading
- ✅ **(ii) Specific features** to make ACC safe/reliable (fault tolerant)
- ✅ **(iii) General block diagram** with safety modules and brief roles

**Hardware Improvements** (Items 1-4):
- ✅ Sensor redundancy + plausibility checks [Reading Pages 25, 27]
- ✅ BIST + automatic reconfiguration [Reading Page 27]
- ✅ Hardware watchdog + OS one-shot timer guard [Week 4, Reading]
- ✅ Optional static pairing/TMR [Reading Pages 33, 27]

**Software Improvements** (Items 5-8):
- ✅ Recovery Blocks with explicit acceptance tests [Reading Page 36]
- ✅ Checkpointing + roll-back [Reading Page 38]
- ✅ Limited N-version programming (with warning) [Reading Page 35]
- ✅ Fault-tolerant scheduling with ghost copies [Reading Pages 41-47]

**Real-Time Communication** (Items 9-11):
- ✅ Bounded asynchronous messaging [Week 5]
- ✅ Event flags for fault broadcasting [Week 5]
- ✅ Mutex-protected shared state [Week 4]

**Measurable Safety Thresholds** (Items 12-13):
- ✅ Deadline guard (hard timing < 100ms) [Assignment requirement]
- ✅ Sensor plausibility thresholds (k consecutive failures) [Reading Page 25]

**Block Diagram Annotations**:
- ✅ OS one-shot timer watchdog (under Watchdog Monitor)
- ✅ Event-flag lines feeding Safety State Machine
- ✅ Queues + counting semaphores on Sensor→Control, Control→Actuator paths
- ✅ Memory partition annotation on queue-passed buffers

---

## Next Steps After Plan Approval

1. Create detailed block diagram with all required annotations
2. Write hardware improvements section (with reading citations)
3. Write software improvements section (with reading citations)
4. Describe each safety module in detail (with measurable thresholds)
5. Explain integration and real-time considerations (Week 4/5 references)
6. Review against assignment rubric (2 marks) - ensure all checklist items covered

