import serial
import json
import time

# Replace with the ESP32's USB port on Ubuntu (e.g., "/dev/ttyUSB0")
esp32_port = "/dev/ttyUSB0"
baud_rate = 115200

# Open the serial connection
ser = serial.Serial(esp32_port, baud_rate, timeout=1)

def parse_weather_data(data):
    try:
        # Attempt to parse JSON response from ESP32
        weather = json.loads(data)
        print("\nCurrent Weather Data:")
        print(f"Location: {weather['location']['name']}, {weather['location']['country']}")
        temp_c = weather['current']['temp_c']
        print(f"Temperature: {temp_c}°C")
        print(f"Condition: {weather['current']['condition']['text']}")
        print(f"Humidity: {weather['current']['humidity']}%")
        print(f"Wind Speed: {weather['current']['wind_kph']} kph")

        # LED notification based on temperature
        if temp_c < 15:
            print("Cold LED is ON (temperature < 15°C)")
        else:
            print("Warm LED is ON (temperature >= 15°C)")

    except json.JSONDecodeError:
        # If JSON decoding fails, print the raw data (likely an error message)
        print(f"ESP32 Response: {data}")

while True:
    # Prompt for the city name
    location = input("Enter the city name: ")
    
    # Send the city name to the ESP32
    ser.write((location + '\n').encode())

    # Wait for the ESP32 to respond with weather data
    time.sleep(2)  # Wait for ESP32 to process the request

    # Read response from ESP32
    while ser.in_waiting > 0:
        # Read a line from the ESP32
        data = ser.readline().decode('utf-8').strip()
        if data:
            parse_weather_data(data)
    time.sleep(1)

