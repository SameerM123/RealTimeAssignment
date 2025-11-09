# Requirement 1: Challenge #2 - Hills - Planning Document

## Challenge Overview
**Challenge #2: Hills** from MCDW-Deeper-Learning-ACC.pdf

### Problem Description
- ACC systems look straight ahead and cannot adjust their field of view up or down hills
- On hilly roads, the system's field of view may not detect vehicles that are:
  - Over the crest of a hill ahead
  - In a dip/valley below
- **Consequence**: ACC may not detect Car B (vehicle ahead) and could unexpectedly accelerate

### Expert's Analysis
- **How might ACC respond?**: When Car A goes over the hill, ACC system may not detect it and unexpectedly accelerate
- **How should driver respond?**: Disengage ACC and maintain safe speed manually until road is less hilly

---

## Plan for Requirement 1 Solution

### Step 1: Problem Analysis
1. **Technical Root Cause**
   - Limited sensor field of view (radar/lidar typically ±15-20 degrees vertical)
   - Line-of-sight obstruction by hill geometry
   - Sensor cannot "look over" or "look under" hills

2. **Real-Time Implications**
   - Detection failure = safety-critical timing violation
   - Unexpected acceleration = potential collision risk
   - Need for rapid detection and response (< 100ms typical ACC cycle time)

### Step 2: Solution Components

#### A. Enhanced Sensor Fusion
- Combine multiple sensor types:
  - **Primary**: Forward-facing radar/lidar (current ACC sensor)
  - **Secondary**: GPS + map data (predictive awareness of hills)
  - **Tertiary**: Vehicle-to-Vehicle (V2V) communication (if available)
  - **Additional**: Camera-based object detection (wider field of view)

#### B. Predictive Hill Detection
- Use GPS and digital map data to:
  - Identify upcoming hills before reaching them
  - Pre-adjust sensor parameters
  - Prepare control system for reduced visibility scenarios

#### C. Real-Time Safety Mechanisms
- **Hill Mode Detection**: Detect when approaching/on hills
- **Conservative Speed Control**: Reduce speed proactively on hills
- **Fail-Safe Behavior**: If sensor confidence drops, engage safer mode
- **Driver Alert System**: Warn driver when entering hill scenarios

### Step 3: Real-Time Related Issues to Address

1. **Timing Constraints**
   - Sensor fusion processing time
   - Map data lookup latency
   - Decision-making deadline (< 100ms control cycle)

2. **Task Priorities**
   - Hill detection task (high priority)
   - Sensor fusion task (high priority)
   - Control task (critical priority)
   - Display/alert task (medium priority)

3. **Deadline Requirements**
   - Hard deadline: Must detect hill before vehicle reaches it
   - Soft deadline: Sensor fusion should complete within control cycle
   - Safety deadline: Fail-safe activation if detection fails

4. **Synchronization Needs**
   - Coordinate GPS data with sensor data
   - Synchronize map updates with sensor readings
   - Ensure control task receives timely hill status

5. **Fault Tolerance**
   - Handle GPS failure gracefully
   - Handle map data unavailability
   - Fallback to sensor-only mode with conservative behavior

---

## Solution Structure for Write-up

### 1. Introduction (2-3 sentences)
- State chosen challenge: Challenge #2 - Hills
- Brief problem description

### 2. Problem Analysis (3-4 sentences)
- Explain why ACC fails on hills
- Technical limitations
- Safety implications

### 3. Proposed Solution (4-5 sentences)
- Multi-sensor fusion approach
- Predictive hill detection using GPS/maps
- Conservative control strategy
- Driver alert system

### 4. Real-Time Related Issues (4-5 sentences)
- Timing constraints and deadlines
- Task priorities and scheduling
- Synchronization requirements
- Fault tolerance considerations

### 5. Conclusion (1-2 sentences)
- Summary of how solution addresses challenge
- Real-time system design considerations

---

## Key Points to Emphasize

✅ **Real-Time Aspects**:
- Control cycle timing (< 100ms)
- Hard deadlines for safety-critical decisions
- Task scheduling and priorities
- Synchronization between multiple data sources

✅ **Safety Considerations**:
- Fail-safe behavior when detection uncertain
- Conservative speed control on hills
- Driver notification system

✅ **Technical Solutions**:
- Sensor fusion (not just single sensor)
- Predictive approach (GPS + maps)
- Multi-modal detection (radar + camera + V2V)

---

## Next Steps
1. Draft the solution write-up following the structure above
2. Ensure real-time issues are clearly addressed
3. Keep it concise (short description as per requirement)
4. Review against assignment rubric

