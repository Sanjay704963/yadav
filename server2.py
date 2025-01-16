from flask import Flask, request
from grove.display.jhd1802 import JHD1802
from seeed_dht import DHT
import requests
import time

# Initialize Flask app
app = Flask(__name__)

# Initialize the LCD
lcd = JHD1802()

# Initialize the DHT sensor on port D5 (pin 5)
dht_sensor = DHT('11', 5)  # '11' refers to the DHT11 sensor

# ThingSpeak Configuration
THINGSPEAK_CHANNEL_ID = "2807590"
THINGSPEAK_WRITE_API = "AJANTEXYOGWP01R2"
THINGSPEAK_URL = f"https://api.thingspeak.com/update"

@app.route('/update', methods=['POST'])
def update_display():
    # Get data from the Arduino POST request
    data = request.get_json()
    if not data:
        return "No data received", 400

    mq9_value = data.get('mq9_value', 'N/A')
    mq9_voltage = data.get('mq9_voltage', 'N/A')
    alert = data.get('alert', 'N/A')

    # Read temperature and humidity from DHT11
    hum, temp = dht_sensor.read()

    # Update the LCD display
    lcd.clear()
    lcd.setCursor(0, 0)
    lcd.write(f"Temp: {temp:.1f}C Hum: {hum:.1f}%")
    lcd.setCursor(1, 0)
    lcd.write(f"MQ9: {mq9_value} Alert: {alert}")

    # Send data to ThingSpeak
    payload = {
        "api_key": THINGSPEAK_WRITE_API,
        "field1": temp,
        "field2": hum,
        "field3": mq9_value,
        "field4": mq9_voltage,
        "field5": alert
    }
    try:
        response = requests.get(THINGSPEAK_URL, params=payload)
        if response.status_code == 200:
            print("Data sent to ThingSpeak successfully!")
        else:
            print(f"Failed to send data to ThingSpeak. HTTP Status: {response.status_code}")
    except Exception as e:
        print(f"Error sending data to ThingSpeak: {e}")

    return "Data updated on LCD and sent to ThingSpeak", 200

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
