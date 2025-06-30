  Hardware Components and Objectives

  The hardware forms the physical layer of the system, responsible for
  interacting with the environment, collecting raw data, and providing
  immediate, on-site feedback.

1. Arduino Mega 2560 (The Central Controller)

   * Objective: To act as the brain of the sensor node. It orchestrates all
     on-site operations, including reading data from sensors, processing it,
     displaying it locally, and transmitting it to the server.
   * Technical Details: The Mega is chosen for its extensive number of I/O
     pins (54 digital, 16 analog), which is essential for connecting the
     large array of sensors and peripherals in this project. It has multiple
     hardware serial ports (Serial1, Serial2, etc.), allowing it to
     communicate with the ESP8266 on Serial1 while reserving the main USB
     serial port for debugging and programming. It operates at 5V, which is
     compatible with most of the selected sensors.
2. ESP8266 ESP-01 (The Communication Module)

   * Objective: To provide Wi-Fi connectivity, acting as a bridge between the
     isolated Arduino environment and the local network where the server
     resides.
   * Technical Details: This module is a self-contained System on a Chip (SoC)
     with an integrated TCP/IP protocol stack. The Arduino communicates with
     it using AT commands over a serial connection (UART). The main.cpp code
     shows it being configured in "station mode" (AT+CWMODE_CUR=1) to connect
     to a Wi-Fi network. It then establishes a TCP connection to the server's
     IP address and port to send data via HTTP POST requests.
3. Gas & Environmental Sensors (The Detectors)

   * MQ-5 (LPG Sensor): Detects Liquefied Petroleum Gas, natural gas, and
     coal gas. Its objective is to warn of potential gas leaks and fire
     hazards.
   * MQ-7 (CO Sensor): Detects Carbon Monoxide, a toxic gas produced by
     incomplete combustion. Its objective is to ensure air safety for
     personnel.
   * MQ-135 (Air Quality Sensor): Detects a wide range of gases like ammonia
     (NH3), benzene, and smoke. Its objective is to provide a general measure
     of air quality.
   * DHT22 (Temperature & Humidity Sensor): Provides ambient temperature and
     relative humidity readings, which are crucial context for other sensor
     readings and for general environmental monitoring.
   * Flame Sensor: An infrared (IR) sensor that detects the specific light
     spectrum emitted by fire. Its objective is to provide a rapid and direct
     fire alert.
   * Technical Details: The MQ sensors are analog sensors whose resistance
     changes based on gas concentration. The MQUnifiedsensor library in the
     firmware abstracts the complex process of reading the analog voltage,
     calculating the sensor's resistance ratio (Rs/R0), and converting it to
     Parts Per Million (PPM) using regression formulas defined during setup.
     The Flame sensor is also read as a simple analog value.
4. DSM501A (Particulate Matter Sensor)

   * Objective: To measure the concentration of airborne particulate matter
     (PM), specifically PM2.5 and PM10, which are significant health hazards.
   * Technical Details: This sensor uses an optical system (an LED and a
     photodetector) to detect particles. The firmware measures the duration
     of low pulses (pulseIn) from the sensor's output pins over a fixed
     sample time (15000 ms). These pulse durations are then used in formulas
     (calc_c_ugm3) to calculate the particle concentration in micrograms per
     cubic meter (µg/m³).
5. Local Feedback Peripherals (The On-site Interface)

   * SH1106G OLED Display: A 128x64 monochrome display. Its objective is to
     provide real-time status updates, sensor readings, and alert messages
     directly on the device, so a person on-site can understand the
     environmental conditions without needing to check the web dashboard.
   * LEDs (RGB & Single-Color): Used for visual status indication. The RGB
     LED shows the current process state (e.g., calibrating, sending data),
     while the alarm LED provides a clear, bright warning.
   * Buzzer: Provides audible alerts. The firmware contains logic to play
     different tones (warning_tone, hazard_tone) depending on the severity
     of the situation, ensuring that hazards are noticed even if the visual
     displays are not being watched.

---

  Software Components and Technical Analysis

  The software translates raw sensor data into meaningful information, transmits
   it, stores it, and presents it to the user.

1. Arduino Firmware (`main.cpp`)

   * Objective: To be the embedded software that runs the hardware. It
     initializes all sensors, performs continuous readings, checks for hazard
     conditions, controls the local display/alerts, and formats/sends data
     to the server.
   * Technical Analysis: The code is structured around the main loop()
     function. It reads each sensor sequentially. A key part of the logic is
     building the payload string. This is a custom, space-separated key-value
     format (e.g., "humidity:55.2 temp:25.1 flame:5 ..."). This format is
     simple to generate on the memory-constrained Arduino. The firmware uses
     specific libraries for each hardware component (DHT.h,
     MQUnifiedsensor.h, Adafruit_SH110X.h) to abstract low-level hardware
     interactions. The send_command function is a critical utility that
     encapsulates the logic for sending an AT command to the ESP8266 and
     waiting for a response, which is fundamental for network communication.
2. Django Server (Backend)

   * Objective: To act as the central data hub. It receives data from the
     Arduino, persists it in a database, and serves it to the web client via
     a RESTful API.
   * Technical Analysis:
     * Data Ingestion: The post_data view in server/api/views.py is the
       entry point for the Arduino. It receives the raw HTTP POST request.
       A crucial piece of code here is the regular expression
       re.findall(r"([\w_]+):([-+]?\d*\.?\d+)", body) which parses the
       custom payload string from the Arduino into key-value pairs. This is
       a flexible but potentially fragile method; a format like JSON would
       be more robust but harder to implement on the Arduino.
     * Data Storage: The parsed data is validated and saved into the SQLite
       database using the SensorDataSerializer and the SensorData model
       defined in models.py. SQLite is a good choice for a self-contained,
       small-to-medium scale project like this.
     * Data Serving: The get_data view allows the React client to fetch
       data. It supports filtering by a from timestamp in ISO 8601 format,
       enabling the client to request only recent data, which is efficient.
       The data is serialized into JSON format for easy consumption by the
       frontend.
3. React Client (Frontend)

   * Objective: To provide a user-friendly, web-based graphical interface
     for monitoring the system, viewing real-time data, and analyzing
     historical trends.
   * Technical Analysis: The application, defined in client/src/App.jsx,
     uses react-router-dom to create a single-page application (SPA) with
     different sections (Overview, Sensors, etc.). It uses the useEffect
     hook to periodically fetch data from the Django API every 10 seconds
     (setInterval). This polling mechanism keeps the dashboard updated. The
     fetched JSON data is stored in the component's state
     (overviewSensorData, sensorsData) and passed down as props to child
     components (Overview.jsx, Sensors.jsx), which are responsible for
     rendering the charts and gauges.

---

  End-to-End System Integration

  Here is how a single data point travels through the entire system:

1. Sensing: The DHT22 sensor measures a humidity of 55.2%.
2. Local Processing: The Arduino's loop() function calls dht.readHumidity(),
   receives the 55.2 value, and appends the string "humidity:55.2 " to its
   payload variable. It also displays this value on the OLED screen.
3. Transmission: After collecting data from all sensors, the Arduino commands
   the ESP8266 (via AT+CIPSEND) to send an HTTP POST request to the server at
   http://192.168.0.104:8000/api/post-data/, with the payload string in the
   request body.
4. Ingestion & Storage: The Django server's post_data view receives the request.
   The regex parser extracts ('humidity', '55.2'). The serializer validates
   this data and saves it as a new row in the SensorData table in the db.sqlite3
   database, with the humidity column set to 55.2 and a new timestamp.
5. Retrieval & Presentation: A few seconds later, the React client's useEffect
   timer fires. It makes an HTTP GET request to
   http://192.168.0.104:8000/api/get-data/. The get_data view queries the
   database for recent entries, including the new row. It serializes this data
   into a JSON array and sends it back to the client. The client updates its
   state with the new data, causing React to re-render the components, and the
   user now sees the updated humidity value on their dashboard chart.
