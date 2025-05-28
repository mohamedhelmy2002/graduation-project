import requests
import time

esp_ip = "http://192.168.22.178"  # Replace with your ESP IP

# Global variable to track current speed
current_speed = 0
steer=0
def gradual_speed_update(target_speed, step_value=10, step_time=0.01, steer=0):
    global current_speed

    while current_speed != target_speed:
        # Compute difference
        diff = target_speed - current_speed

        # Choose step direction
        if abs(diff) < step_value:
            current_speed = target_speed  # Final step
        else:
            current_speed += step_value if diff > 0 else -step_value

        send_to_esp(current_speed, steer)
        time.sleep(step_time)

    # Send the final stable value once more after reaching target
    send_to_esp(current_speed, steer)

def send_to_esp(speed, steer=0):
    try:
        # First get feedback (optional)
        response = requests.get(f"{esp_ip}/receive", timeout=1)
        print("Received from ESP:", response.text)
    except Exception as e:
        print("Error while receiving from ESP:", e)

    try:
        # Then send updated values
        send_value = f"{steer}-{speed}"
        send_data = {"value": send_value}
        response = requests.post(f"{esp_ip}/send", data=send_data, timeout=1)
        print("Sent to ESP:", send_data["value"])
    except Exception as e:
        print("Error while sending to ESP:", e)

# Main loop (can dynamically change target_speed)
while True:
    target_speed = 100  # You can try changing this to -200, 0, etc.
    steer=50
    gradual_speed_update(target_speed, step_value=25, step_time=0.1, steer=steer)
    #time.sleep(1)  # Wait before potentially updating again
