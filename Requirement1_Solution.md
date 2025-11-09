# Requirement 1 Solution: Challenge #2 - Hills

## Selected Challenge
**Challenge #2: Hills** - ACC systems struggle on hilly roads due to limited sensor field of view.

---

## Problem Analysis

### How might your ACC system respond?
When approaching or traversing hilly terrain, the ACC system's forward-facing radar/lidar sensor has a limited vertical field of view (typically ±15-20 degrees). As a result, when a vehicle (Car A) goes over the crest of a hill ahead, the ACC system may lose detection and incorrectly interpret this as "no vehicle ahead." This causes the system to unexpectedly accelerate to reach the set cruise speed, creating a dangerous situation if another vehicle (Car B) is actually present but undetected in the hill's blind spot.

### How should you respond?
The driver should immediately disengage ACC and manually maintain a safe speed until the road becomes less hilly. However, this reactive approach places the burden on the driver and may not provide sufficient warning time in critical situations.

---

## Proposed Solution

To address the hills challenge, I propose a **multi-sensor fusion approach with predictive hill detection**:

### 1. Enhanced Sensor Fusion
The system should integrate multiple sensor modalities:
- **Primary sensor**: Forward-facing radar/lidar (existing ACC sensor) for direct vehicle detection
- **GPS + Digital Map Integration**: Pre-identify upcoming hills and elevation changes using GPS coordinates and high-definition map data
- **Camera-based Detection**: Wide-angle camera system to provide additional visual confirmation and extended field of view
- **Vehicle-to-Vehicle (V2V) Communication**: If available, receive position and speed data from vehicles ahead that are beyond line-of-sight

### 2. Predictive Hill Mode
Before reaching a hill, the system should:
- Detect approaching hills using GPS and map data
- Preemptively enter "Hill Mode" which activates conservative control parameters
- Reduce target speed by a safety margin (e.g., 10-15% below set cruise speed)
- Increase sensor sensitivity and scanning frequency
- Alert the driver that ACC is operating in hill mode

### 3. Fail-Safe Behavior
When sensor confidence drops below a threshold (indicating potential detection failure):
- Automatically reduce speed to a safe level
- Increase following distance if a vehicle is detected
- Provide visual and audible alerts to the driver
- If detection is completely lost on a hill, maintain current speed rather than accelerating

---

## Real-Time Related Issues

### Timing Constraints and Deadlines
1. **Control Cycle Deadline**: The ACC control loop operates with a hard deadline of < 100ms. Hill detection and sensor fusion processing must complete within this cycle to ensure timely control decisions. This requires efficient algorithms and potentially parallel processing of sensor data.

2. **Predictive Detection Deadline**: GPS/map-based hill detection must occur well in advance (several seconds before reaching the hill) to allow time for mode transition and speed adjustment. This requires a separate predictive task with lower frequency but higher look-ahead capability.

3. **Sensor Fusion Latency**: Combining data from multiple sensors (radar, GPS, camera) introduces processing latency. The fusion algorithm must be optimized to complete within the control cycle deadline while maintaining accuracy.

### Task Scheduling and Priorities
The system requires multiple tasks with different priorities:
- **Highest Priority**: Control task (hard real-time, < 100ms deadline) - calculates acceleration/deceleration commands
- **High Priority**: Hill detection task (periodic, ~500ms) - processes GPS/map data to identify upcoming hills
- **High Priority**: Sensor fusion task (periodic, synchronized with control cycle) - combines sensor data
- **Medium Priority**: Display/alert task (soft real-time, ~2 seconds) - updates driver interface and warnings

### Synchronization Requirements
1. **Sensor Data Synchronization**: Multiple sensor readings must be time-stamped and synchronized to a common time base before fusion. This requires careful timestamp management and potentially buffering to align asynchronous sensor data.

2. **Map Data Updates**: GPS coordinates must be matched with map data, requiring periodic map lookups. This should be done asynchronously to avoid blocking the control task.

3. **Mode Transition Synchronization**: When entering/exiting hill mode, all tasks must be notified atomically to ensure consistent system state. This requires inter-task communication mechanisms (semaphores or message queues).

### Fault Tolerance Considerations
1. **GPS Failure**: If GPS becomes unavailable, the system should fall back to sensor-only mode with enhanced conservative behavior (reduced speed, increased following distance).

2. **Map Data Unavailability**: If map data cannot be accessed, the system should rely on sensor-based hill detection (e.g., analyzing sensor signal strength variations that may indicate terrain changes).

3. **Sensor Degradation**: If primary sensor confidence drops, the system should gracefully degrade to a safer operating mode rather than failing catastrophically.

### Real-Time System Design Implications
- **Preemptive Scheduling**: Higher priority tasks (control, hill detection) must be able to preempt lower priority tasks to meet deadlines
- **Resource Sharing**: Shared data structures (sensor readings, system state) require mutex protection to prevent race conditions
- **Interrupt Handling**: Timer interrupts for control cycle must have minimal jitter to maintain consistent 100ms period
- **Memory Management**: Sensor data buffers must be managed efficiently to avoid memory leaks or fragmentation over long operation periods

---

## Conclusion

The hills challenge exposes critical real-time system design requirements for safety-critical automotive systems. The proposed solution addresses detection limitations through sensor fusion and predictive awareness, while ensuring all processing meets hard real-time deadlines. The system must balance computational complexity with timing constraints, requiring careful task scheduling, synchronization, and fault tolerance mechanisms to maintain safety in challenging terrain conditions.

