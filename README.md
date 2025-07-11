# MinoHome

Known Bugs:
- ESP32-C3 might crash after a while and send any data whatsoever anymore. Reason for this is unknown due to no debugging using JTAG, lack of time.

---

This project aims to get a grade in the Master's CompSci subject "Communication Systems" at the FHV Vorarlberg.  
The goal is to:
- Set up a Home Assistant alongside Mosquitto as a MQTT Message Broker locally using Docker (and Docker Compose)
- Develop an MQTT Client on an ESP32-C3 that sends payloads to Mosquitto with sensor data coming from
  - Temperature
  - Humidity
  - CO2
  - Formaldehyde
  - TVOC
  - Air Quality
  - Ambient Light
  - Potentiometer Voltage
- Have the MQTT Payloads be interpreted and displayed on Home Assistant

## Hardware Equipment

The equipment used were:

- Breadboard
- ESP32-C3-DevKit-M1
- Male-to-Male Jumper Cable
- USB Micro Cable
- B10K Potentiometer
- BH1750 Sensor (Ambient Light)
- DHT20 (Temperature & Humidity sensor)
- TVOC Gas Sensor from WaveShare (www.waveshare.com/wiki/TVOC_Sensor)
- Optional: USB JTAG Cable for JTAG Debugging, since UART is being used by TVOC Sensor

## Software Requirements

* Use VSCode, and download the ESP-IDF Extension
* Set up ESP-IDF Extension and download ESP 5.4.0
* Download Docker Desktop (for Home Assistant, Mosquitto, and Docker Compose)

## How to Setup Home Assistant & Mosquitto

* Go to a terminal, go to the root folder of this project, and write
```
docker-compose -f ./compose.yml up -d
```

This will start Home Assistant and Mosquitto. Go to localhost:8123 on your browser and create an account.  
  
Go to "Settings" -> "Devices & services". On the bottom right is a blue button called "+ ADD INTEGRATION". Search for "MQTT", click on it and select "MQTT" again (NOT "MQTT JSON" or anything else).  You will be prompted to add information now.:
- Broker: "mosquitto"
- Port: 1883
- Username: leave empty
- Password leave empty

Click OK, your sensors should now be visible on Home Assistant's custom dashboard called "MQTT Sensors".

## How to send data to Mosquitto

Obviously the MQTT Container & Home Assistant Container should be running!  
Once the project is open and all requirements set up over the ESP-IDF Extension, open up the menuconfig of the ESP-IDF extension by pressing F1 within VSCode, and then type:
```
>ESP-IDF: SDK Configuration Editor (Menuconfig)
```
Search for
```
WiFi and MQTT Settings
```
and type in the relevant information:

- WiFi SSID: Your WiFi SSID (make sure it's the same network as the your PC)
- WiFi Password: No explanation needed
- MQTT Broker URI: Format is "mqtt://Your-PC-IP-Address"
- MQTT Broker Port: 1883
- MQTT Broker Username: leave empty
- MQTT Broker Password: leave empty

Make sure that you click "Save"!  
  
Set the COM Port, and the ESP chip (ESP-C3).  
You're now ready to go to Build, Flash, and Monitor the Firmware. Make sure that if you Monitor, that you use USB JTAG cables, since UART is used by the TVOC Gas Sensor, making it impossible to Flash & Monitor over UART. You can also just temporarily unplug the TX and RX cables from the Gas Sensor and plug it back in once you start Monitoring.
