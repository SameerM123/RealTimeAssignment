# Requirement 2 Plan - Improvements Incorporated Summary

## ✅ All Improvements Successfully Integrated

### A) Hardware Safety & Fault-Tolerance

#### ✅ A1: Sensor Redundancy + Plausibility Checks
- **Added**: Explicit plausibility window with measurable thresholds
  - Headway bounds: [X_min, X_max] meters
  - Rate-of-change limits: Δv/Δt physical limits
  - Cross-validation tolerance window
- **Added**: k consecutive failure counter (k=3) → fail-safe flag
- **Citation**: Fault Tolerance Reading, Page 25 (safety-reliability coupling, fail-safe states)

#### ✅ A2: BIST + Automatic Reconfiguration
- **Added**: Explicit action on failure (isolate failed unit, switch in redundant)
- **Added**: Short-term fault masking concept
- **Citation**: Fault Tolerance Reading, Page 27

#### ✅ A3: Hardware Watchdog + OS One-Shot Timer Guard
- **Added**: OS one-shot timer guard (dual-layer watchdog)
- **Added**: Deadline guard logic (≤ 100ms from Req 3)
- **Added**: Deadline-miss event → Safety State Machine
- **Citation**: Week 4 Lecture Notes (one-shot timers for watchdogs/safeguards)

#### ✅ A4: Static Pairing/TMR (Optional)
- **Added**: Static pairing behavior (pair compares outputs, isolate on discrepancy)
- **Added**: Interface monitor caveat
- **Citation**: Fault Tolerance Reading, Pages 33, 27

---

### B) Software Safety & Fault-Tolerance

#### ✅ B1: Recovery Blocks with Explicit Acceptance Test
- **Added**: Concrete acceptance test criteria:
  - Output bounded: dM(n) ∈ [dM_min, dM_max]
  - Monotonic headway response validation
  - Rate-of-change limits: |dM(n) - dM(n-1)| < threshold
- **Citation**: Fault Tolerance Reading, Page 36

#### ✅ B2: Checkpointing + Roll-Back
- **Added**: Explicit checkpoint contents (Vset, Xset, e(n), e(n-1), e(n-2), dM(n), sensor readings)
- **Added**: State-check after meaningful progress
- **Added**: Roll-back → Degraded state transition
- **Citation**: Fault Tolerance Reading, Page 38

#### ✅ B3: N-Version Programming (Narrow, with Warning)
- **Added**: Limited scope (one or two critical computations only)
- **Added**: Statistical correlation of failures warning
- **Added**: Use sparingly, don't rely alone
- **Citation**: Fault Tolerance Reading, Page 35

#### ✅ B4: Fault-Tolerant Scheduling with Ghost Copies
- **Added**: Reserve capacity concept
- **Added**: Ghost copies embedded in schedule
- **Added**: Simplified but acceptable backup versions
- **Added**: Conditions C1 and C2 reference
- **Citation**: Fault Tolerance Reading, Pages 41-47

---

### C) Real-Time Communication & Synchronization

#### ✅ C1: Bounded Asynchronous Messaging
- **Added**: Message queues / task message queues for Sensor→Control→Actuator
- **Added**: Flow control (queue + counting semaphore)
- **Added**: Fixed-size memory partitions for deterministic alloc/free
- **Citation**: Week 5 Lecture Notes (Inter-Task Communication, flow control, keeping data in scope)

#### ✅ C2: Event Flags for Fault Broadcasting
- **Added**: Consolidated fault/status signaling to Safety State Machine
- **Added**: Bits for: sensor fault, deadline miss, checksum failure, plausibility violation
- **Added**: Status events (non-blocking) and transient events (blocking)
- **Citation**: Week 5 Lecture Notes (event flags for status and transient events)

#### ✅ C3: Mutex-Protected Shared Safety State
- **Added**: Guard shared state (Safety State Machine state, parameter memory block)
- **Citation**: Week 4 Lecture Notes (mutexes for resource sharing)

---

### D) Measurable Safety Thresholds

#### ✅ D1: Deadline Guard (Hard Timing)
- **Added**: If Control hasn't "petted" OS timer by t = Di (≤ 100ms) → deadline-miss event
- **Added**: Deadline miss → Safety State Machine → Degraded/Fail-Safe
- **Citation**: Assignment requirement (< 100ms control cycle)

#### ✅ D2: Sensor Plausibility Thresholds
- **Added**: Explicit rate-of-change and physical-bounds checks
- **Added**: Headway cannot shrink faster than X m/s given speeds
- **Added**: On violation: drop trust, switch to backup, notify Safety State Machine
- **Citation**: Fault Tolerance Reading, Page 25 (fail-safe concept)

---

## Block Diagram Annotations Added

✅ **OS one-shot timer watchdog** (under "Watchdog Monitor")
✅ **Event-flag lines** feeding Safety State Machine
✅ **Queues + counting semaphores** on Sensor→Control, Control→Actuator paths
✅ **Memory partition annotation** on queue-passed buffers
✅ **Plausibility check thresholds** in Sensor Health Monitor
✅ **Deadline guard timing** (< 100ms) in Watchdog Monitor

---

## Rubric Alignment

### Requirement 2 asks for:
- ✅ **(i) Discussion of HW/SW improvements** using Fault Tolerance reading
- ✅ **(ii) Specific features** to make ACC safe/reliable (fault tolerant)
- ✅ **(iii) General block diagram** with safety modules and brief roles

### All Items Covered:
- ✅ Hardware improvements (4 items) with reading citations
- ✅ Software improvements (4 items) with reading citations
- ✅ Real-time communication (3 items) with Week 4/5 citations
- ✅ Measurable thresholds (2 items) with assignment/reading citations
- ✅ Block diagram annotations (6 items) as specified

---

## Plan Status: ✅ READY FOR APPROVAL

All improvements have been incorporated into the plan. The plan now includes:
- Explicit thresholds and measurable criteria
- Course material references (Week 4, Week 5)
- Reading material citations with page numbers
- Rubric-aligned structure
- Complete block diagram annotations

**Next Step**: Await approval, then proceed with detailed solution write-up.

