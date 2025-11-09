# ENGG4420 Assignment Summary: Adaptive Cruise Control (ACC) System Design

## Assignment Details
- **Course**: ENGG4420 Real Time Systems Design
- **Instructor**: Petros Spachos
- **Weight**: 10% of final grade
- **Date Issued**: October 8, 2025
- **Due Date**: November 14, 2025 (Friday), 11:59 pm online
- **Format**: Typed PDF with diagrams and uC/OS-III code

## Important Notes
- **Independent work required** - no consultations with other students
- **No questions** about assignment solutions allowed to instructor or TAs
- Solutions must be typed with legible schematics

---

## Assignment Requirements Breakdown

### Requirement 1 [1 mark]
**Task**: Study the ACC reference document (MCDW-Deeper-Learning-ACC.pdf)

**Deliverable**: 
- Select **one of the 4 challenges** presented:
  1. **Challenge #1: Curving Roads** - ACC may not detect vehicles around curves
  2. **Challenge #2: Hills** - ACC struggles on hilly roads with limited field of view
  3. **Challenge #3: Merging Traffic** - ACC may not detect vehicles merging into lane
  4. **Challenge #4: Slow and Heavy Traffic** - ACC systems often disengage below 25 mph

- Provide a **short description** with solution addressing **real-time related issues**

---

### Requirement 2 [2 marks]
**Task**: Design safety and fault tolerance features

**Deliverable**:
- Study fault tolerance reading material (engg4420_FaultTolerance...)
- Research reliability and safety topics online
- **Discuss improvements** (hardware and software) for safety and reliability
- **Present a general block diagram** showing safety modules
- **Briefly explain** the role of each safety module

**Key Concepts from Reading**:
- Fail-safe states
- Safety-critical systems
- Fault tolerance techniques:
  - Hardware: BIST, TMR, Static Pairing
  - Software: N-version programming, Recovery blocks, Checkpointing/Roll-back

---

### Requirement 3 [3 marks]
**Task**: Design functional task diagram with synchronization and communication

**Deliverable**:
- **Identify task types**: hard, soft, cyclic, event-driven
- **Create functional task diagram** showing:
  - Main tasks
  - Data objects
  - Synchronization structures (semaphores, flags)
  - Communication structures (message queues, shared memory)

**Required Tasks** (minimum):
1. **Setup** - Monitors ACC_on_off switch, performs setup when ON
2. **ISR (IRQ_sensors)** - Timer interrupt routine (< 100ms), posts semaphore/flag
3. **Sensors** - Data acquisition task, reads distance (X(n)), speed (V(n)), other sensors
4. **Control** - Calculates manipulated variable dM(n) using control algorithm
5. **Actuator** - Sends control values to ACC actuators
6. **Display** - Updates LCD every 2 seconds with distance, speed, ACC status

**System Flow**:
1. Setup task monitors ACC_on_off switch
2. Timer interrupt triggers ISR → posts semaphore/flag
3. Sensors task pends on semaphore → reads sensors → stores X(n), V(n) in parameter memory block (with mutex protection) → signals control task
4. Control task calculates dM(n) → communicates to actuator task
5. Actuator task sends control value to actuators
6. Display task updates LCD every 2 seconds

**Reference**: Week 5 lecture notes on task synchronization and inter-task communication

---

### Requirement 4 [4 marks]
**Task**: Implement uC/OS-III code

**Deliverable**:
- **Detailed implementation** of the **Control task**
- **Pseudo-code** for all other tasks and main program
- Show uC/OS-III function calls (OSInit(), OSStart(), object creation, task creation, etc.)
- **Choose appropriate task priorities** and justify choices

**Control Task Requirements**:
- Wait for new control cycle (nth cycle) with new data
- Calculate manipulated variable **dM(n)** using equations (1)-(4)
- Store dM(n) in parameter memory block
- Post to Actuator task

**Control Algorithm**:

**If X(n) ≥ Xset:**
1. Calculate: Vset = Vcruise (Equation 1)
2. Calculate errors: e(n) = Vset - V(n), e(n-1) = Vset - V(n-1), e(n-2) = Vset - V(n-2) (Equation 2)
3. Calculate: dM(n) = K1×e(n) + K2×e(n-1) + K3×e(n-2) (Equation 3)

**If X(n) < Xset:**
1. Calculate: Vset = Vset - deltaV (Equation 4)
2. Calculate errors: e(n) = Vset - V(n), e(n-1) = Vset - V(n-1), e(n-2) = Vset - V(n-2) (Equation 2)
3. Calculate: dM(n) = K1×e(n) + K2×e(n-1) + K3×e(n-2) (Equation 3)

**Parameter Memory Block Structure**:
- ACC01: ACC-on-off flag
- K1, K2, K3: Controller parameters
- Vcruise: Set cruise speed
- Vset: Current cycle speed reference
- Xset: Minimum safe distance
- Xn: Current distance (nth cycle)
- Vn: Current speed (nth cycle)
- Vn1: Speed at cycle (n-1)
- Vn2: Speed at cycle (n-2)
- dMn: Manipulated variable

**uC/OS-III Functions to Use**:
- Semaphores: OSSemCreate(), OSSemPend(), OSSemPost()
- Mutex: OSMutexCreate(), OSMutexPend(), OSMutexPost()
- Message Queues: OSQCreate(), OSQPend(), OSQPost()
- Tasks: OSTaskCreate()

---

## Key System Constraints
- Timer interrupt frequency: **< 100ms**
- Display update: **Every 2 seconds**
- Floating point variables required for calculations
- Mutex protection needed for parameter memory block access

---

## Next Steps
1. Review all extracted PDF texts in `extracted_texts/` directory
2. Study Week 5 lecture notes on synchronization and message passing
3. Research ACC systems and fault tolerance techniques
4. Design the system architecture
5. Implement the solution


