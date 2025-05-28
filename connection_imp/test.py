import time
import pygame
import requests
import re
import math


# === ESP Setup ===
esp_ip = "http://192.168.201.178"
current_speed = 0  # Global current speed
# === Function to send values to ESP ===
def send_to_esp(speed, steer=0):
    global data
    try:
        response = requests.get(f"{esp_ip}/receive", timeout=1)
        data=response.text
        #print("Received from ESP:", response.text)
    except Exception as e:
        print("Error while receiving from ESP:", e)

    try:
        send_value = f"{steer},{speed}"
        send_data = {"value": send_value}
        response = requests.post(f"{esp_ip}/send", data=send_data, timeout=1)
        #print("Sent to ESP:", send_data["value"], end="")
    except Exception as e:
        print("Error while sending to ESP:", e)
###  motor ##
def motor_speeds(steer, speed):
    right_speed = (steer / 2) - speed
    left_speed = (steer / 2) + speed
    """print(f"   SP_R: {right_speed}", end="")
    print(f" SP_L: {left_speed}")"""
    return left_speed, right_speed
### distance###
distance_r=0
distance_l=0
def wheel_dist(rpm, delta_ms, wheel_diameter_cm=16.5):
    wheel_circumference = 3.1416 * wheel_diameter_cm  # cm
    rps = rpm / 60.0  # revolutions per second
    time_sec = delta_ms / 1000.0
    distance = rps * time_sec * wheel_circumference
    return distance
# === Joystick Setup ===
pygame.init()
joystick = pygame.joystick.Joystick(0)
joystick.init()

BASE_SPEED = 250
speed_levels = [0.2, 0.35, 0.5, 0.65, 0.75, 0.85, 1.0]
speed_index = 0
DEADZONE = 0.1
last_button_state = False

# Slope Configuration
ACCEL_STEP = 10       # Smaller = slower acceleration
DECEL_STEP = 50       # Larger = faster deceleration
UPDATE_INTERVAL = 0.05  # 50ms = 20Hz update

# === Main Loop ===
print("Starting control loop. Press Ctrl+C to exit.")
try:
    last_update_time = time.time()

    target_speed = 0
    steer_to_send = 0

    while True:
        pygame.event.pump()

        # === Joystick Input ===
        steering = joystick.get_axis(0)
        speed = joystick.get_axis(1)

        if abs(speed) < DEADZONE:
            speed = 0
        if abs(steering) < DEADZONE:
            steering = 0

        # Speed level button
        button_pressed = joystick.get_button(0)
        if button_pressed and not last_button_state:
            speed_index = (speed_index + 1) % len(speed_levels)
            print(f"Speed level changed to {int(speed_levels[speed_index] * 100)}%")
        last_button_state = button_pressed

        level = speed_levels[speed_index]
        max_speed = BASE_SPEED * level

        target_speed = int(speed * max_speed)
        steer_to_send = int(steering * max_speed)

        # === Update Speed Gradually (Non-blocking) ===
        now = time.time()
        if now - last_update_time >= UPDATE_INTERVAL:
            last_update_time = now
            diff = target_speed - current_speed
            if diff != 0:
                # Check if speeds are in the same direction
                same_direction = (current_speed * target_speed) > 0

                # Choose step based on whether we're accelerating or decelerating
                if same_direction:
                    accelerating = abs(target_speed) > abs(current_speed)
                    step = ACCEL_STEP if accelerating else DECEL_STEP
                else:
                    # If reversing direction, decelerate quickly
                    step = DECEL_STEP

                # Limit step to not overshoot
                step = min(abs(diff), step)
                current_speed += step if diff > 0 else -step
        motor_speeds(steer_to_send, current_speed)
        send_to_esp(current_speed, steer_to_send)
        #### take each value separtly #### 
        # data is feeedback data from motor 
         # Find all key-value pairs
        matches = re.findall(r'(\w+):\s*(-?\d+)', data)
        # Convert to dictionary
        parsed_data = {key: int(value) for key, value in matches}
        # Now you can access each value like this:
        steer = parsed_data['st']
        speed = parsed_data['sp']
        speed_r = parsed_data['sp_r']
        speed_l = parsed_data['sp_l']
        battery = parsed_data['bat_V']
        temperature = parsed_data['temp']
        delta_time = parsed_data['d_t']
        
        ## GETTING DISTACE 
        distance_r = wheel_dist(speed_r, delta_time)+distance_r 
        distance_l = wheel_dist(speed_l, delta_time)+distance_l
        avg_distance = (distance_r - distance_l)/2
        
        if avg_distance >= 10:
            send_to_esp(0, 0)  # stop
        else:
            send_to_esp(-100, 0)  # keep moving  
        print(avg_distance)
        print(f"Distance Right: {distance_r:.2f} cm | Left: {distance_l:.2f} cm")

        #print(data)

            

except KeyboardInterrupt:
    print("Exiting...")
    send_to_esp(0, 0)
