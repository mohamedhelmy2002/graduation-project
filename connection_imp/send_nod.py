import requests
import time

esp_ip = "http://192.168.125.113"  # Replace with actual IP from Serial Monitor
send_value = "10-50" ## send steerig then speed 

while True:
    try:
        # GET from ESP /receive
        response = requests.get(f"{esp_ip}/receive")
        print("Received from ESP:", response.text)

        # POST to ESP /send
        send_data = {"value":send_value}
        requests.post(f"{esp_ip}/send", data=send_data)
        print("Sent to ESP:", send_data["value"])

        time.sleep(0.1)  # 100ms delay to be apple to read serial 
    except Exception as e:
        print("Error:", e)
        time.sleep(1)
