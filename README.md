# Environmental Quality & Hazard Detection System using Arduino Mega 2560 with sensors (ESP8266 ESP01 wifi module)

## notes
```bash
sudo ufw allow 8000 # port 8000
```

## **Components and Power Requirements**

### **Devices**

1. **4 MQ Sensors**
2. **DHT22**
3. **DSM501A**
4. **DS3231**
5. **BH1750FVI**
6. **801S Vibration Sensor**
7. **ESP8266 ESP-01**
8. **Arduino Mega 2560**
9. **SH1106G OLED Display (128x64)**
10. **Push Buttons (x4)**
11. **LEDs (RGB and single color)**
12. **Buzzer**

### **Power Requirements**

| **Device** | **Voltage** | **Current** | **Total Current** |
| --- | --- | --- | --- |
| **4 MQ Sensors** | 5V | ~300mA each | **1.2A** |
| **DHT22** | 5V | ~1.5mA | **~2mA** |
| **DSM501A** | 5V | ~100mA | **~100mA** |
| **DS3231 RTC** | 3.3V/5V | ~0.1mA | **~0.1mA** |
| **BH1750FVI** | 3.3V/5V | ~0.2mA | **~0.2mA** |
| **801S Vibration Sensor** | 5V | ~15mA | **~15mA** |
| **ESP8266 ESP-01** | 3.3V | ~200mA–250mA | **~250mA** |
| **Arduino Mega 2560** | 5V | ~50mA (base) | **~50mA** |
| **OLED Display** | 3.3V/5V | ~20mA | **~20mA** |
| **LEDs** | 5V | ~20mA per LED | **~60mA** (for 3 LEDs) |
| **Buzzer** | 5V | ~30mA | **~30mA** |

**Total Current Requirement**: ~1.73A

Adding a **20% safety margin**, the required power supply is **5V 2A** (minimum).

---

## **Power Supply Setup**

- Use a **5V 3A regulated power supply** to power the Arduino Mega 2560 and sensors.
- Use a **5V-to-3.3V voltage regulator** for devices like the **ESP8266**, **BH1750**, and **DS3231**.

---

## **Wiring Guide**

### **1. ESP8266 ESP-01 Wiring**

The ESP8266 operates on **3.3V**, and its current draw (200-250mA) exceeds the Mega's 3.3V regulator capacity. Use a **dedicated 5V-to-3.3V regulator** (e.g., **AMS1117-3.3**):

- **Regulator Input (Vin)**: Connect to the 5V rail.
- **Regulator Output (Vout)**: Connect to ESP8266 VCC.
- **GND**: Common ground with the Arduino.

**Connections**:

| ESP-01 Pin | Connect To |
| ----------- | ----------- |
| VCC | 3.3V (via regulator) |
| GND | Common ground |
| TX | Mega RX1 (pin 19, via resistor divider) |
| RX | Mega TX1 (pin 18) |
| EN (CH_PD) | 3.3V (pull-up with ~10kΩ) |
| RST | Leave unconnected or use as needed |

**Voltage Divider** for RX Pin:

- ESP8266 RX is **3.3V logic**, while the Mega uses **5V logic**. Add a **voltage divider (1kΩ and 2kΩ)** to step down 5V TX signals to 3.3V.

---

### **2. Arduino Mega 2560**

- Power the Arduino Mega 2560 via the **5V pin** or **barrel jack**:
    - **Barrel Jack**: Use a 9V–12V adapter.
    - **5V Pin**: Connect directly to the 5V rail of the power supply.

---

### **3. Sensors**

### **OLED Display (SH1106G)**

- **VCC**: Connect to 3.3V or 5V.
- **GND**: Common ground.
- **SCL**: Mega SCL (pin 21).
- **SDA**: Mega SDA (pin 20).

### **Buttons (x4)**

- Connect one leg of each button to a digital pin (50, 51, 52, 53).
- Connect the other leg to GND.

### **LEDs**

- **RGB LED**: Connect R, G, and B pins to PWM-capable digital pins (e.g., 6, 7, 42).
- **Alarm/Server LEDs**: Connect to digital pins (e.g., 44, 48).
- Use a current-limiting resistor (e.g., 220Ω) for each LED.

### **Buzzer**

- Connect the positive leg to a digital pin (e.g., 5).
- Connect the negative leg to GND.

### **MQ Sensors (4x)**

- Connect all MQ sensors' VCC pins to the **5V rail**.
- Add **100µF electrolytic capacitors** across each sensor's VCC and GND pins for stability.

### **DHT22, DSM501A, 801S**

- Connect their VCC pins to the **5V rail**.

### **BH1750FVI and DS3231**

- Power these sensors via the **3.3V regulator**.

---

### **4. Power Distribution**

- Use a **breadboard** or **PCB with power rails** to distribute 5V and 3.3V:
    - 5V: Power for MQ sensors, DSM501A, DHT22, 801S, and Arduino Mega.
    - 3.3V: Power for ESP8266, BH1750FVI, and DS3231.

---

## **Safety Measures**

1. **Capacitors**:
    - Place **100µF electrolytic capacitors** near high-power devices (e.g., MQ sensors, ESP8266) to smooth voltage spikes.
    - Add **0.1µF ceramic capacitors** for filtering.
2. **Voltage Regulation**:
    - Use a **dedicated AMS1117-3.3 regulator** for 3.3V devices to avoid overloading the Arduino Mega's 3.3V regulator.
3. **Grounding**:
    - Ensure all devices share a **common ground** to prevent communication issues.
4. **Current Limit Protection**:
    - Add a **2A fuse** or resettable polyfuse on the 5V line for overcurrent protection.
5. **Resistor Divider for ESP RX**:
    - Prevent damage to the ESP8266 by using a **voltage divider** on its RX pin to step down 5V signals from the Mega.

---

## **Final Testing and Verification**

1. **Voltage Measurement**:
    - Measure voltage at each device's VCC pin to ensure it's stable (5V or 3.3V as required).
2. **Current Draw**:
    - Use a multimeter in series to verify the total current draw does not exceed 2A.
3. **ESP8266 Debugging**:
    - Check ESP8266 boot messages via the Serial Monitor (use 115200 baud).
