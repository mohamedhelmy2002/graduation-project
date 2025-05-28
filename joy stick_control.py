import time
import threading
import queue
import pygame

gampad_data = queue.Queue()

# First thread
def task():
    while True: 
        # Wait for the gamepad data from the second thread
        data = gampad_data.get()  # Block until new data is available
        print(data)  # Print the received data
        #time.sleep(.5)

# Second thread (gamepad control)
def gampad_control():
    # Setup
    pygame.init()
    joystick = pygame.joystick.Joystick(0)
    joystick.init()

    BASE_SPEED = 500
    speed_levels = [0.2,.35, 0.5,.65, 0.75,.85, 1.0]
    speed_index = 0  # Start at 100%
    DEADZONE = 0.1
    last_button_state = False

    while True: 
        pygame.event.pump()
        steering = joystick.get_axis(0)  # Left/Right
        speed = -joystick.get_axis(1)    # Forward/Backward
        if abs(speed) < DEADZONE: speed = 0
        if abs(steering) < DEADZONE: steering = 0 
        # Handle speed level change
        button_pressed = joystick.get_button(0)
        if button_pressed and not last_button_state:
            speed_index = (speed_index + 1) % len(speed_levels)
        last_button_state = button_pressed

        level = speed_levels[speed_index]
        max_speed = BASE_SPEED * level

        # Scaled values to send to microcontroller
        speed_to_send = int(speed * max_speed)
        steering_to_send = int(steering * max_speed)
        # Left and right motor speeds for reference
        left = speed + steering
        right = speed - steering
        max_val = max(abs(left), abs(right))
        if max_val > 1:
            left /= max_val
            right /= max_val
        left_speed, right_speed =int(left * max_speed), int(right * max_speed)
        #print(f"Level: {int(level*100)}% | Speed: {speed_to_send} | Steering: {steering_to_send} | L: {left_speed} | R: {right_speed}")
        gampad_data.put([[steering_to_send,speed_to_send],[level,round(level*max_speed)],[right_speed,left_speed]])
        time.sleep(0.05)  # Add a small delay to reduce CPU usage

# Start the threads
task_thread = threading.Thread(target=task)
gampad_thread = threading.Thread(target=gampad_control)

task_thread.start()
gampad_thread.start()

# Wait for threads to finish (this won't happen because they run infinitely)
task_thread.join()
gampad_thread.join()
