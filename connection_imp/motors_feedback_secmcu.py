import requests
import re
import math

## 201.253
ESP_IP = "http://192.168.201.253:8080"  # Replace with your ESP8266's IP address
### receice ########
def receive_data():
    global feedback_data
    try:
        response = requests.get(ESP_IP, timeout=2)
        if response.status_code == 200:
            feedback_data=response.text
            #print("ESP Response:", feedback_data)
        else:
            print("Failed with status:", response.status_code)
    except requests.exceptions.RequestException as e:
        print("Error:", e)

#### 
def count_to_distance(count):
    wheel_diameter_cm = 16.5
    steps_per_revolution = 90  # from 0 to 89
    circumference_cm = math.pi * wheel_diameter_cm
    distance_per_step = circumference_cm / steps_per_revolution

    # Final distance in cm
    distance_cm = count * distance_per_step
    return distance_cm


if __name__ == "__main__":
    while True:
        receive_data()
        match = re.search(r"c1:\s*(-?\d+)\s*\|\s*.*c2:\s*(-?\d+)", feedback_data)
        counter_1 = int(match.group(1))
        counter_2 = int(match.group(2))
        
        rightwheel=count_to_distance(counter_1)
        leftwheel=count_to_distance(counter_2)
        print("rightwheel:", rightwheel,end=" ")
        print("leftwheel:", leftwheel)
        #time.sleep(.05)  # fetch every 1 second
