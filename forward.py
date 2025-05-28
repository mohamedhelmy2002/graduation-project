import math
import time

# PID parameters
Kp = 1.0
Ki = 0.1
Kd = 0.05

# Motion limits
max_speed = 1.0       # m/s
max_acc = 0.5         # m/s²

# Initial state
x, y = 0.0, 0.0
x_target, y_target = 5.0, 5.0

# PID memory
integral = 0.0
last_error = 0.0
last_time = time.time()

# Velocity state
v_current = 0.0

def get_distance_error(x, y, x_target, y_target):
    dx = x_target - x
    dy = y_target - y
    distance = math.sqrt(dx**2 + dy**2)
    return distance, dx, dy

def clamp(value, min_value, max_value):
    return max(min(value, max_value), min_value)

while True:
    # 1. Get current position
    error, dx, dy = get_distance_error(x, y, x_target, y_target)

    # 2. Time update
    current_time = time.time()
    dt = current_time - last_time
    if dt == 0:
        continue

    # 3. PID output (ideal speed)
    integral += error * dt
    derivative = (error - last_error) / dt
    v_pid = Kp * error + Ki * integral + Kd * derivative

    # 4. Limit PID output to max speed
    v_pid = clamp(v_pid, 0.0, max_speed)

    # 5. Smooth ramping using acceleration limit
    v_change = clamp(v_pid - v_current, -max_acc * dt, max_acc * dt)
    v_current += v_change

    # 6. Update position
    direction = math.atan2(dy, dx)
    x += v_current * dt * math.cos(direction)
    y += v_current * dt * math.sin(direction)

    # 7. Output
    print(f"Error: {error:.2f} m | Target v: {v_pid:.2f} | Smooth v: {v_current:.2f} | Pos: ({x:.2f}, {y:.2f})")

    # 8. Stop when close enough
    if error < 0.05:
        print("Target reached.")
        break

    last_error = error
    last_time = current_time
    time.sleep(0.05)
