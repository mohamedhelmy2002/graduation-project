import time
import pygame
#import requests

# === ESP Setup ===
# own one : "192.168.79.113"
# lab one : "192.168.22.178"
esp_ip = "http://192.168.79.113" # 
current_speed = 0  # Global current speed
"""
# === Function to send values to ESP ===
def send_to_esp(speed, steer=0):
    try:
        response = requests.get(f"{esp_ip}/receive", timeout=1)
        print("Received from ESP:", response.text)
    except Exception as e:
        print("Error while receiving from ESP:", e)

    try:
        send_value = f"{steer}-{speed}"
        send_data = {"value": send_value}
        response = requests.post(f"{esp_ip}/send", data=send_data, timeout=1)
        print("Sent to ESP:", send_data["value"])
        
    except Exception as e:
        print("Error while sending to ESP:", e)
"""
# === Smooth speed update ===
def gradual_speed_update(target_speed, steer, step_value=10, step_time=0.01):
    global current_speed

    while current_speed != target_speed:
        diff = target_speed - current_speed
        if abs(diff) < step_value:
            current_speed = target_speed
        else:
            current_speed += step_value if diff > 0 else -step_value
        print(steer,current_speed)
        #send_to_esp(current_speed, steer)
        time.sleep(step_time)

    #send_to_esp(current_speed, steer)

# === Joystick Setup ===
pygame.init()
joystick = pygame.joystick.Joystick(0)
joystick.init()

BASE_SPEED = 100
speed_levels = [0.2, 0.35, 0.5, 0.65, 0.75, 0.85, 1.0]
speed_index = 0
DEADZONE = 0.1
last_button_state = False

# === Main Loop ===
print("Starting control loop. Press Ctrl+C to exit.")
try:
    while True:
        pygame.event.pump()

        steering = joystick.get_axis(0)*-1
        speed = -joystick.get_axis(1)*-1

        # Deadzone filtering
        if abs(speed) < DEADZONE:
            speed = 0
        if abs(steering) < DEADZONE:
            steering = 0

        # Handle speed level button
        button_pressed = joystick.get_button(0)
        if button_pressed and not last_button_state:
            speed_index = (speed_index + 1) % len(speed_levels)
            print(f"Speed level changed to {int(speed_levels[speed_index] * 100)}%")
        last_button_state = button_pressed

        level = speed_levels[speed_index]
        max_speed = BASE_SPEED * level

        speed_to_send = int(speed * max_speed)
        steer_to_send = int(steering * max_speed)

        gradual_speed_update(speed_to_send, steer_to_send)
        #time.sleep(0.5)

except KeyboardInterrupt:
    print("Exiting...")
    #send_to_esp(0, 0)  # Stop the robot
