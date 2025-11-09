# Requirement 1: Challenge #2 - Hills [Final Submission Version]

## Selected Challenge
I have selected **Challenge #2: Hills** from the ACC reference document. This challenge addresses the limitation where ACC systems cannot adjust their sensor field of view vertically, causing detection failures on hilly terrain.

---

## Problem Analysis and Solution

### How might your ACC system respond?
When approaching hilly roads, the ACC system's forward-facing radar/lidar sensor has a limited vertical field of view (±15-20 degrees). As a vehicle ahead goes over the crest of a hill, the ACC loses detection and incorrectly interprets this as "no vehicle ahead," causing unexpected acceleration. This creates a safety hazard if another vehicle is present but undetected in the hill's blind spot.

### How should you respond?
The driver should immediately disengage ACC and manually maintain a safe speed. However, this reactive approach may not provide sufficient warning time in critical situations.

### Proposed Solution
I propose a **multi-sensor fusion approach with predictive hill detection**:

1. **Enhanced Sensor Fusion**: Integrate GPS + digital map data to pre-identify upcoming hills, combine with existing radar/lidar, and add camera-based detection for extended field of view.

2. **Predictive Hill Mode**: Before reaching a hill, the system enters "Hill Mode" using GPS/map data, reducing target speed by 10-15% as a safety margin and alerting the driver.

3. **Fail-Safe Behavior**: When sensor confidence drops, automatically reduce speed, increase following distance, and maintain current speed rather than accelerating if detection is lost.

---

## Real-Time Related Issues

### Timing Constraints
- **Control Cycle Deadline**: Hill detection and sensor fusion must complete within the < 100ms control cycle deadline. This requires efficient algorithms and potentially parallel processing.
- **Predictive Detection**: GPS/map-based hill detection must occur several seconds in advance, requiring a separate predictive task with lower frequency but higher look-ahead capability.

### Task Scheduling and Priorities
- **Highest Priority**: Control task (hard real-time, < 100ms deadline)
- **High Priority**: Hill detection task (periodic, ~500ms) and sensor fusion task (synchronized with control cycle)
- **Medium Priority**: Display/alert task (soft real-time, ~2 seconds)

### Synchronization Requirements
- Sensor data from multiple sources (radar, GPS, camera) must be time-stamped and synchronized to a common time base before fusion
- Map data lookups must be asynchronous to avoid blocking the control task
- Mode transitions require atomic notification to all tasks using semaphores or message queues

### Fault Tolerance
- **GPS Failure**: Fall back to sensor-only mode with enhanced conservative behavior
- **Map Data Unavailability**: Rely on sensor-based hill detection using signal strength variations
- **Sensor Degradation**: Gracefully degrade to safer operating mode rather than failing catastrophically

### Real-Time System Design Implications
The system requires preemptive scheduling, mutex-protected shared data structures, minimal interrupt jitter for consistent control cycles, and efficient memory management for sensor data buffers. These mechanisms ensure safety-critical deadlines are met while maintaining system reliability in challenging terrain conditions.

---

## Summary
The hills challenge exposes critical real-time requirements: hard deadlines for control decisions, careful task prioritization, synchronization of multiple data sources, and fault tolerance. The proposed solution addresses detection limitations through predictive awareness and sensor fusion while ensuring all processing meets real-time constraints essential for safety-critical automotive systems.

