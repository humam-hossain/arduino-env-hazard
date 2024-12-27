# Environmental Quality & Hazard Detection System using Arduino Mega 2560 with sensors (ESP8266 ESP01 wifi module)

## notes
```bash
sudo ufw allow 8000 # port 8000
```

Components and Power Requirements

Devices

1. 4 MQ Sensors


2. DHT22


3. DSM501A


4. DS3231


5. BH1750FVI


6. 801S Vibration Sensor


7. ESP8266 ESP-01


8. Arduino Mega 2560



Power Requirements

Total Current Requirement: ~1.62A
Adding a 20% safety margin, the required power supply is 5V 2A (minimum).


---

Power Supply Setup

Use a 5V 3A regulated power supply to power the Arduino Mega 2560 and sensors.

Use a 5V-to-3.3V voltage regulator for devices like the ESP8266, BH1750, and DS3231.



---

Wiring Guide

1. ESP8266 ESP-01 Wiring

The ESP8266 operates on 3.3V, and its current draw (200-250mA) exceeds the Mega’s 3.3V regulator capacity. Use a dedicated 5V-to-3.3V regulator (e.g., AMS1117-3.3):

Regulator Input (Vin): Connect to the 5V rail.

Regulator Output (Vout): Connect to ESP8266 VCC.

GND: Common ground with the Arduino.


Connections: | ESP-01 Pin | Connect To                  | |------------|-----------------------------| | VCC        | 3.3V (via regulator)        | | GND        | Common ground               | | TX         | Mega RX1 (pin 19, via resistor divider) | | RX         | Mega TX1 (pin 18)           | | EN (CH_PD) | 3.3V (pull-up with ~10kΩ)   | | RST        | Leave unconnected or use as needed |

Voltage Divider for RX Pin:

ESP8266 RX is 3.3V logic, while the Mega uses 5V logic. Add a voltage divider (1kΩ and 2kΩ) to step down 5V TX signals to 3.3V.



---

2. Arduino Mega 2560

Power the Arduino Mega 2560 via the 5V pin or barrel jack:

Barrel Jack: Use a 9V–12V adapter.

5V Pin: Connect directly to the 5V rail of the power supply.




---

3. Sensors

MQ Sensors (4x)

Connect all MQ sensors’ VCC pins to the 5V rail.

Add 100µF electrolytic capacitors across each sensor's VCC and GND pins for stability.


DHT22, DSM501A, 801S

Connect their VCC pins to the 5V rail.


BH1750FVI and DS3231

Power these sensors via the 3.3V regulator.



---

4. Power Distribution

Use a breadboard or PCB with power rails to distribute 5V and 3.3V:

5V: Power for MQ sensors, DSM501A, DHT22, 801S, and Arduino Mega.

3.3V: Power for ESP8266, BH1750FVI, and DS3231.




---

Safety Measures

1. Capacitors:

Place 100µF electrolytic capacitors near high-power devices (e.g., MQ sensors, ESP8266) to smooth voltage spikes.

Add 0.1µF ceramic capacitors for filtering.



2. Voltage Regulation:

Use a dedicated AMS1117-3.3 regulator for 3.3V devices to avoid overloading the Arduino Mega's 3.3V regulator.



3. Grounding:

Ensure all devices share a common ground to prevent communication issues.



4. Current Limit Protection:

Add a 2A fuse or resettable polyfuse on the 5V line for overcurrent protection.



5. Resistor Divider for ESP RX:

Prevent damage to the ESP8266 by using a voltage divider on its RX pin to step down 5V signals from the Mega.





---

Final Testing and Verification

1. Voltage Measurement:

Measure voltage at each device’s VCC pin to ensure it’s stable (5V or 3.3V as required).



2. Current Draw:

Use a multimeter in series to verify the total current draw does not exceed 2A.



3. ESP8266 Debugging:

Check ESP8266 boot messages via the Serial Monitor (use 115200 baud).





---

Would you like a schematic diagram or further clarification on any part?

