#For Odrroid board. If not, don't use this code.

import odroid_wiringpi as wpi
import time
import serial  # Import the serial library
import urllib.request  # Import urllib.request for making HTTP requests

wpi.wiringPiSetup()
wpi.pinMode(7, 0)
wpi.pinMode(8, 1)

# Configure the serial connection to the Arduino
arduino_port = '/dev/ttyS1'  # Replace with the correct port for your Arduino
arduino_baudrate = 115200

ser = serial.Serial(arduino_port, arduino_baudrate)

# Define the URL you want to request
url = 'https://api.thingspeak.com/update?api_key=LRDUV4C8FZKRHNWC&field1=0' 

last_time = time.time()
while True:
    # Read a string from the serial port
    data = ser.readline().decode().strip()
    print(f'Receive data : {data}')
    time.sleep(1)
    current_time = time.time()

    if current_time - last_time >= 5:
        last_time = time.time()
        # print('Hi')
        response = urllib.request.urlopen(url + str(data))
        response.read()
        response.close()
        print(f"Sent URL request with count {data} to Thingspeak")






    

