#include <Arduino.h>
#include "DHT.h"
#include <MQUnifiedsensor.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Wire.h>

// Define OLED display dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Declare an SH110X display object (I2C interface)
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

void scrollText(String text, int y) {
  static int scrollOffset = 0; // Horizontal scrolling offset
  int textWidth = text.length() * 6; // Calculate text width assuming each character is 6 pixels wide
  int displayWidth = 128; // Adjust this based on your display's width
  int textHeight = 8; // Adjust based on the font size (e.g., 8 pixels for size 1)

  // Clear the specific line area
  display.fillRect(0, y, displayWidth, textHeight, SH110X_BLACK);

  // Draw the text at the current offset
  display.setCursor(-scrollOffset, y);
  display.print(text);

  // Handle wrapping around for scrolling
  if (textWidth - scrollOffset < displayWidth) {
    display.setCursor(textWidth - scrollOffset, y);
    display.print(text);
  }

  // Update the scroll offset
  scrollOffset++;
  scrollOffset++;
  scrollOffset++;
  scrollOffset++;
  if (scrollOffset > textWidth) {
    scrollOffset = 0; // Reset the offset when the text has fully scrolled
  }
}

// BUTTONS
#define UP_BTN 53
#define DOWN_BTN 51
#define LEFT_BTN 50
#define RIGHT_BTN 52

// LEDS
#define LED_BRIGHTNESS 20
#define ALARM_LED 48
#define PROCESS_LED_RED 42
#define PROCESS_LED_GREEN 6
#define PROCESS_LED_BLUE 7
#define SERVER_LED 44

void process_led(int red, int green, int blue) {
  analogWrite(PROCESS_LED_RED, red);
  analogWrite(PROCESS_LED_GREEN, green);
  analogWrite(PROCESS_LED_BLUE, blue);
}

void process_warning(){
  process_led(40, 40, 0);
  delay(1000);
  process_led(0, 0, 0);
}

void process_error(){
  process_led(40, 0, 0);
  delay(1000);
  process_led(0, 0, 0);
}

// BUZZER
#define BUZZER 5

// Notes of the Nokia starting tone (frequencies in Hz)
int tempo = 180;       // Tempo for the melody

// Nokia ringtone melody (notes and durations)
int melody[] = {
  659, 8, 587, 8, 370, 4, 415, 4, 
  554, 8, 494, 8, 294, 4, 330, 4, 
  494, 8, 440, 8, 277, 4, 330, 4, 
  440, 2
};

void nokia_tone()
{
  int noteCount = sizeof(melody) / sizeof(melody[0]) / 2;
  int wholeNote = (60000 * 4) / tempo;

  for (int i = 0; i < noteCount * 2; i += 2) {
    int note = melody[i];
    int duration = wholeNote / melody[i + 1];
    
    if (note > 0) {
      tone(BUZZER, note, duration * 0.9); // Play note for 90% of its duration
    }
    delay(duration); // Wait for the duration
    noTone(BUZZER); // Stop the note
  }
}

void normal_tone()
{
  noTone(BUZZER);
}

void warning_tone()
{
  tone(BUZZER, 1000);
}

void hazard_tone()
{
  tone(BUZZER, 500);
}

// MQ Gas Sensors
#define PLACA "Arduino Mega 2560"
#define VOLTAGE_RESOLUTION 5
#define ADC_BIT_RESOLUTION 10

#define PIN_MQ5 A0                  //Analog input 0 of your arduino
#define TYPE_MQ5 "MQ-5"             //MQ5
#define RatioMQ5CleanAir 6.5        //RS / R0 = 6.5 ppm

#define PIN_MQ7 A1                  //Analog input 1 of your arduino
#define TYPE_MQ7 "MQ-7"             //MQ7
#define RatioMQ7CleanAir 27.5       //RS / R0 = 27.5 ppm

#define PIN_MQ8 A2                  //Analog input 2 of your arduino
#define TYPE_MQ8 "MQ-8"             //MQ8
#define RatioMQ8CleanAir 70         //RS / R0 = 70 ppm 

#define PIN_MQ135 A3                //Analog input 3 of your arduino
#define TYPE_MQ135 "MQ-135"         //MQ135
#define RatioMQ135CleanAir 3.6      //RS / R0 = 3.6 ppm 

MQUnifiedsensor MQ5(PLACA, VOLTAGE_RESOLUTION, ADC_BIT_RESOLUTION, PIN_MQ5, TYPE_MQ5);
MQUnifiedsensor MQ8(PLACA, VOLTAGE_RESOLUTION, ADC_BIT_RESOLUTION, PIN_MQ8, TYPE_MQ8);
MQUnifiedsensor MQ7(PLACA, VOLTAGE_RESOLUTION, ADC_BIT_RESOLUTION, PIN_MQ7, TYPE_MQ7);
MQUnifiedsensor MQ135(PLACA, VOLTAGE_RESOLUTION, ADC_BIT_RESOLUTION, PIN_MQ135, TYPE_MQ135);

// DHT
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Flame sensor
#define FLAME_SENSOR A4

// DSM501A
#define pin2_1um 3
#define pin4_25um 4
#define sample_time 15000
int sample_count = 0;

float calc_low_ratio(float lowPulse) {
  return lowPulse / (sample_time) * 100.0;  // low ratio in %
}

float calc_c_ugm3(float lowPulse) {
  float r = calc_low_ratio(lowPulse);
  float c_mgm3 = 1000 * 0.00258425 * pow(r, 2) + 0.0858521 * r - 0.01345549;
  return max(0, c_mgm3);
}

float calc_c_pcs283ml(float lowPulse) {
  float r = calc_low_ratio(lowPulse);
  float c_pcs283ml =  625 * r;
  return min(c_pcs283ml, 12500);
}

unsigned long init_time;


// ESP8266
#define ESP8266 Serial1
#define BAUD_RATE 115200

#define PING_ADDR "www.google.com"

// #define IP_ADDR "192.168.6.100"
// #define DEFAULT_GATEWAY "192.168.6.1"
// #define SUBNET_MASK "255.255.255.0"
// #define TCP_SERVER_PORT 333

#define SERVER_IP "192.168.0.104"
// #define SERVER_IP "192.168.0.104"
#define SERVER_PORT "8000"

// Function to send AT commands and wait for a valid response
String send_command(const String& command, unsigned long timeout, bool debug = false) {
  process_led(0, 0, LED_BRIGHTNESS);
  ESP8266.println(command);  // Send AT command to check ESP8266 connection
  unsigned long startTime = millis();
  bool completeResponse = false;
  String response = "";

  while (millis() - startTime < timeout) {
    if (ESP8266.available()) {
      char c = ESP8266.read();
      response += c;

      if (response.indexOf("OK") != -1 || response.indexOf("ERROR") != -1 || response.indexOf("busy") != -1) {
        completeResponse = true;
      }
    }
  }

  // debugging
  if(debug) {
    Serial.println("[DEBUG] command: ");
    Serial.println(command);
    Serial.println("[DEBUG] response: ");
    Serial.println(response);
    Serial.print("[DEBUG] input buffer:");
    Serial.println(ESP8266.available());
  }

  if(!completeResponse) {
    Serial.println("[WARNING] incomplete response from ESP8266.");
    process_warning();
  }

  Serial.println("[COMPLETED] " + command);
  process_led(0, 0, 0);
  return response;
}

// Function to ping an internet address
bool ping_server(int total_count, unsigned long timeout, bool debug = false) {
  int count = 0;
  
  for(int i = 0; i < total_count; i++) {
    String response = send_command("AT+PING=\"" + String(PING_ADDR) + "\"", timeout, debug);
    if (response.indexOf("OK") != -1) {
      count++;
    }
  }

  if(debug) {
    Serial.print("[INFO] Pinged ");
    Serial.print(count);
    Serial.print("/");
    Serial.println(total_count);
  }
  
  if(count == total_count){
    Serial.println("[SUCCESS] Internet connection is good.");
    process_led(0, 0, LED_BRIGHTNESS);

    return true;
  }else if (count > 0){
    Serial.println("[WARNING] Internet connection is unstable.");
    process_led(0, 0, LED_BRIGHTNESS);
    return true;
  }else{
    Serial.println("[ERROR] No internet connection.");
    process_error();

    return false;
  }
}

// Function to set up the ESP8266
void setupESP8266() {
  Serial.println("[INFO] Configuring ESP8266...");
  Serial.println("[INFO] Resetting ESP8266...");
  
  send_command("AT+RST", 6500, true);

  Serial.println("[INFO] Turning on station mode on ESP8266");
  send_command("AT+CWMODE_CUR=1", 2000, true);

  // disconnect wifi
  send_command("AT+CWQAP", 3000, true);

  // access to wifi
  String ssid = "no internet";
  String password = "humamhumam09";
  send_command("AT+CWSAP_CUR=\"" + ssid + "\",\"" + password + "\",5,3", 1000, true);
  // send_command("AT+CWJAP_CUR=\"" + ssid + "\",\"" + password + "\",5,3", 1000, true);

  // check ip and mac address
  send_command("AT+CIFSR", 1500, true);

  // check internet connection
  ping_server(1, 2000);
  send_command("AT+CWMODE_CUR?", 1000, true);

  // Serial.println("[INFO] Setting up TCP server...");
  // send_command("AT+CIPAP_CUR=\"" + String(IP_ADDR) + "\",\"" + String(DEFAULT_GATEWAY) + "\",\"" + String(SUBNET_MASK) + "\"", 1000);
  // send_command("AT+CIPMUX=1", 1000);
  // send_command("AT+CIPSERVER=1," + String(TCP_SERVER_PORT), 1000);

  Serial.println("[INFO] ESP8266 setup complete.");
}

void handle_requests() {
  String status = send_command("AT+CIPSTATUS", 1000, true);
  if (status.indexOf("STATUS:2") != -1) {
    Serial.println("[INFO] ESP8266 is connected to a Wi-Fi network (station mode) but does not have any active TCP/UDP connections.");
  } else if (status.indexOf("STATUS:3") != -1){
    Serial.println("[INFO] There is an active TCP/UDP connection.");
  } else if (status.indexOf("STATUS:4") != -1){
    Serial.println("[INFO] The TCP connection was disconnected.");
  } else if (status.indexOf("STATUS:5") != -1){
    Serial.println("[INFO] The ESP8266 is not connected to any Wi-Fi network.");
  } else {
    Serial.println("[ERROR] AT+CIPSTATUS command failed.");
  }
}

void tcp_connect()
{
    // Establish a TCP connection with the server
  String connectCommand = "AT+CIPSTART=\"TCP\",\"" + String(SERVER_IP) + "\"," + String(SERVER_PORT);
  if (send_command(connectCommand, 1000, true).indexOf("OK") == -1) {
    Serial.println("[ERROR] Failed to connect to the server.");
    return;
  }
  Serial.println("[SUCCESS] Established TCP connection with server: " + String(SERVER_PORT) + " on port: " + String(SERVER_PORT));
}

void server_get_req() {
  String path = "/api/get-data/";

  Serial.println("[INFO] Get data from server: " + String(SERVER_IP) + " on port: " + String(SERVER_PORT));
  
  tcp_connect();

  // Prepare the HTTP request
  String httpRequest = "GET " + path + " HTTP/1.1\r\n" + 
                       "Host: " + String(SERVER_IP) + "\r\n" + 
                       "Connection: close\r\n\r\n";

  String sendCommand = "AT+CIPSEND=" + String(httpRequest.length());
  if (send_command(sendCommand, 1000, true).indexOf(">") == -1) {
    Serial.println("[ERROR] ESP8266 not ready to send data.");
    send_command("AT+CIPCLOSE", 1000, true);
    return;
  }
  Serial.println("[SUCCESS] ESP8266 is ready to send data.");

  // Send the actual GET request
  String response = send_command(httpRequest, 2000, true);
  Serial.println("[INFO] Server Response: " + response);
}

void server_post_req(const String& payload) {
  String path = "/api/post-data/";

  Serial.println("[INFO] send data to server: " + String(SERVER_IP) + " on port: " + String(SERVER_PORT));
  
  tcp_connect();

  // Create POST request
  String httpRequest = "POST " + path + " HTTP/1.1\r\n" +
                       "Host: " + String(SERVER_IP) + "\r\n" +
                       "Content-Type: text/plain\r\n" +  
                       "Content-Length: " + String(payload.length()) + "\r\n" +
                       "Connection: close\r\n\r\n" +
                       payload; // Append the payload to the request
  
  // Notify ESP8266 of the data length
  String sendCmd = "AT+CIPSEND=" + String(httpRequest.length());
  if (send_command(sendCmd, 2000, true).indexOf(">") == -1) {
    Serial.println("[ERROR] ESP8266 not ready to send data.");
    send_command("AT+CIPCLOSE", 1000, true);
    return;
  }

  // Send the HTTP POST request
  String response = send_command(httpRequest, 2000, true);
  Serial.println("[INFO] Server Response: " + response);
}

void setup() {
  Wire.begin();

  process_led(0, LED_BRIGHTNESS, 0);
  Serial.begin(BAUD_RATE); // Set up Serial communication for debugging
  while(!Serial){
    process_warning();
    Serial.println("[WARNING] Waiting for Serial to be ready...");
  };  // Wait for Serial to be ready

  // Initialize the OLED display
  if (!display.begin(0x3C)) { // Default I2C address is 0x3C
    Serial.println(F("SH1106G allocation failed"));
    process_error();
  }

  // Clear the buffer
  display.clearDisplay();

  // Set text properties
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SH110X_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner

  // Display welcome message
  display.println(F("INDUSTRIAL ENV"));
  display.println(F("QUALITY & HAZARD"));
  display.println(F("MONITORING SYSTEM"));
  display.setTextColor(SH110X_BLACK, SH110X_WHITE); // Draw white text
  display.println(F("---PROJECT MEMBERS---")); // reverse
  display.setTextColor(SH110X_WHITE); // Draw white text
  display.println(F("Humam Hossain"));
  display.println(F("Mahmudur Rahman Marfy"));
  display.println(F("Saihan Bin Sajjad"));
  display.println(F("Mahmudur Rahman Riyad"));
  display.display();          // Send buffer to the display

  nokia_tone();
  display.clearDisplay();

  display.setCursor(0, 0);
  display.println(F("I:Initializing Setup."));
  display.display();

  // BUTTONS SETUP
  pinMode(UP_BTN, INPUT);
  pinMode(DOWN_BTN, INPUT);
  pinMode(LEFT_BTN, INPUT);
  pinMode(RIGHT_BTN, INPUT);

  // LEDS SETUP
  pinMode(ALARM_LED, OUTPUT);
  pinMode(PROCESS_LED_RED, OUTPUT);
  pinMode(PROCESS_LED_GREEN, OUTPUT);
  pinMode(PROCESS_LED_BLUE, OUTPUT);
  pinMode(SERVER_LED, OUTPUT);

  // BUZZER SETUP
  pinMode(BUZZER, OUTPUT);
  delay(500);
  process_led(0, 0, 0);

  // ESP8266 SETUP
  display.println(F("I:Setting up ESP8266."));
  display.display();
  process_led(0, 0, LED_BRIGHTNESS);
  ESP8266.begin(BAUD_RATE); // Set up Serial1 for ESP8266 communication
  while(!ESP8266){
    Serial.println("[WARNING] Waiting for ESP8266 to be ready...");
    process_warning();
  }  // Wait for ESP8266 to be ready

  String response = "";
  while(ESP8266.available()) {
    Serial.println(ESP8266.available());
    char c = ESP8266.read();
    response += c;
  }  // Clear the input buffer
  Serial.println(response);
  process_led(0, 0, 0);

  setupESP8266();

  // setup gas sensors
  //MQ5
  display.println(F("I:Setting up MQ5 LPG."));
  display.display();
  process_led(0, LED_BRIGHTNESS, 0);
  MQ5.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ5.setA(80.897); MQ5.setB(-2.431); // Configure the equation to to calculate H2 concentration

  //   Exponential regression:
  // Gas    | a      | b
  // H2     | 1163.8 | -3.874
  // LPG    | 80.897 | -2.431
  // CH4    | 177.65 | -2.56
  // CO     | 491204 | -5.826
  // Alcohol| 97124  | -4.918
  
  MQ5.init();   
  Serial.print("[INFO] MQ5: Calibrating please wait.");
  float calcR0 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ5.update(); // Update data, the arduino will read the voltage from the analog pin
    calcR0 += MQ5.calibrate(RatioMQ5CleanAir);
    Serial.print(".");
  }
  MQ5.setR0(calcR0/10);
  Serial.println("  done!.");
  if(isinf(calcR0)) {
    Serial.println("[WARNING] Conection issue, R0 is infinite (Open circuit detected) please check your wiring and supply"); 
    process_warning();
  }
  if(calcR0 == 0){
    Serial.println("[WARNING] Conection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply"); 
    process_error();
  }
  process_led(0, 0, 0);

  //MQ7
  display.println(F("I:Setting up MQ7 CO."));
  display.display();
  process_led(0, LED_BRIGHTNESS, 0);
  MQ7.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ7.setA(99.042); MQ7.setB(-1.518); // Configure the equation to calculate CO concentration value

  //   Exponential regression:
  // GAS     | a      | b
  // H2      | 69.014  | -1.374
  // LPG     | 700000000 | -7.703
  // CH4     | 60000000000000 | -10.54
  // CO      | 99.042 | -1.518
  // Alcohol | 40000000000000000 | -12.35

  MQ7.init(); 
  Serial.print("[INFO] MQ7: Calibrating please wait.");
  float calcR02 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ7.update(); // Update data, the arduino will read the voltage from the analog pin
    calcR02 += MQ7.calibrate(RatioMQ7CleanAir);
    Serial.print(".");
  }
  MQ7.setR0(calcR02/10);
  Serial.println("  done!.");
  if(isinf(calcR02)) {
    Serial.println("[WARNING] Conection issue, R0 is infinite (Open circuit detected) please check your wiring and supply");
    process_warning();
  }
  if(calcR02 == 0){
    Serial.println("[WARNING] Conection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply"); 
    process_error();
  }
  process_led(0, 0, 0);

/*
  //MQ8
  MQ8.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ8.setA(976.97); MQ8.setB(-0.688); // Configure the equation to to calculate H2 concentration
  
  //   Exponential regression:
  // GAS     | a      | b
  // H2      | 976.97  | -0.688
  // LPG     | 10000000 | -3.123
  // CH4     | 80000000000000 | -6.666
  // CO      | 2000000000000000000 | -8.074
  // Alcohol | 76101 | -1.86
  
  MQ8.init();
  Serial.print("[INFO] MQ8: Calibrating please wait.");
  float calcR01 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ8.update(); // Update data, the arduino will read the voltage from the analog pin
    calcR01 += MQ8.calibrate(RatioMQ8CleanAir);
    Serial.print(".");
  }
  MQ8.setR0(calcR01/10);
  Serial.println("  done!.");
  if(isinf(calcR01)) {Serial.println("[WARNING] Conection issue, R0 is infinite (Open circuit detected) please check your wiring and supply"); while(1);}
  if(calcR01 == 0){Serial.println("[WARNING] Conection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply"); while(1);}
*/
  
  //MQ135
  display.println(F("I:Setting up MQ135."));
  display.display();
  process_led(0, LED_BRIGHTNESS, 0);
  MQ135.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ135.setA(102.2 ); MQ135.setB(-2.473);
  // Exponential regression:
  // GAS      | a      | b
  // CO       | 605.18 | -3.937  
  // Alcohol  | 77.255 | -3.18 
  // CO2      | 110.47 | -2.862
  // Toluen  | 44.947 | -3.445
  // NH4      | 102.2  | -2.473
  // Aceton  | 34.668 | -3.369
  
  MQ135.init(); 
  Serial.print("[INFO] MQ135: Calibrating please wait.");
  float calcR03 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ135.update(); // Update data, the arduino will read the voltage from the analog pin
    calcR03 += MQ135.calibrate(RatioMQ135CleanAir);
    Serial.print(".");
  }
  MQ135.setR0(calcR03/10);
  Serial.println("  done!.");
  
  if(isinf(calcR03)) {
    Serial.println("[WARNING] Conection issue, R0 is infinite (Open circuit detected) please check your wiring and supply");
    process_warning();
  }
  if(calcR03 == 0){
    Serial.println("[WARNING] Conection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply");
    process_error();
  }
  
  // DHT Setup
  display.println(F("I:Setting up DHT22"));
  display.display();
  process_led(40, 25, 0);
  Serial.println(F("DHT test!"));
  dht.begin();
  delay(500);
  process_led(0, 0, 0);

  // flame sensor Setup
  display.println(F("I:Setting up Flame"));
  display.display();
  process_led(40, 29, 31);
  pinMode(FLAME_SENSOR, INPUT);
  delay(500);
  process_led(0, 0, 0);

  // DSM501A Setup
  display.println(F("I:Setting up DSM501A"));
  display.display();
  process_led(0, LED_BRIGHTNESS, 0);
  pinMode(pin2_1um, INPUT);
  pinMode(pin4_25um, INPUT);
  delay(500);
  process_led(0, 0, 0);

  analogWrite(SERVER_LED, LED_BRIGHTNESS);
  init_time = millis();
  display.clearDisplay();

}

void loop() {
  static unsigned long t_start = millis();
  float t1, t2, t3;
  const char* data[] = {
    "RH(%): ",
    "Temp(C):",
    "Flame: ",
    "LPG(ppm): ",
    "CO(ppm): ",
    "AQ(ppm): ",
    "Samples: ",
    "PM25(ug/m3): ",
    "PM10(ug/m3): ",
    "t: ",
  };

  const char* sensor_warnings_msg[] = {
    "",
    "",
    "Flame detected nearby!",
    "LPG levels rising stay alert!",
    "CO levels increasing be cautious!",
    "Air quality worsening take care!",
    "",
    "PM2.5 elevated limit exposure!",
    "PM10 elevated check filters!",
    ""  
  };

  const char* sensor_alarms_msg[] = {
    "",
    "",
    "",
    "Critical LPG detected fire risk!",
    "Dangerous CO levels evacuate!",
    "Severe air pollution avoid area!",
    "",
    "PM2.5 critical health risk!",
    "PM10 critical protect lungs!",
    ""
  };


  const int data_size = sizeof(data) / sizeof(data[0]); // Count of data items
  float sensor_values[data_size];
  // 0=good, 1=warning, 2=hazardous
  int sensor_alert[data_size];
  int current_sensor = 0;

  String payload = "";
  String alarm = "";
  String warning = "";
  String message = "";

  // DHT
  process_led(40, 25, 0);
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("[ERROR] Failed to read from DHT sensor!"));
    process_error();
    return;
  }

  sensor_values[current_sensor] = h;
  ++current_sensor;
  sensor_values[current_sensor] = t;
  ++current_sensor;
  payload = "humidity:" + String(h) + " temp:" + String(t) + " ";
  process_led(0, 0, 0);

  // flame sensor
  process_led(40, 29, 31);
  int flame_intensity = 1023 - analogRead(FLAME_SENSOR);
  sensor_values[current_sensor] = flame_intensity;
  if(flame_intensity >= 10) {
    Serial.println("[WARNING] Flame detected!");
    process_warning();
    sensor_alert[current_sensor] = 1;
    warning += "flame";
  }else{
    sensor_alert[current_sensor] = 0;
  }
  ++current_sensor;
  payload += "flame:" + String(flame_intensity) + " ";
  process_led(0, 0, 0);

  //MQ5
  process_led(0, LED_BRIGHTNESS, 0);
  MQ5.update(); // Update data, the arduino will read the voltage from the analog pin
  sensor_values[current_sensor] = MQ5.readSensor();
  payload += "MQ5_LPG:" + String(sensor_values[current_sensor]) + " "; // Sensor will read PPM concentration using the model, a and b values set previously or from the setup
  if(sensor_values[current_sensor] >= 1500){
    Serial.println("[MESSAGE] LPG level is critically high!");
    sensor_alert[current_sensor] = 2;
    alarm += "MQ5_LPG";
    process_warning();
  }
  else if(sensor_values[current_sensor] >= 250){
    Serial.println("[MESSAGE] LPG level is cautiously high!");
    sensor_alert[current_sensor] = 1;
    alarm += "MQ5_LPG";
    process_error();
  }else{
    sensor_alert[current_sensor] = 0;

  }
  ++current_sensor;

  //MQ7
  MQ7.update(); // Update data, the arduino will read the voltage from the analog pin
  sensor_values[current_sensor] = MQ7.readSensor();
  payload += "MQ7_CO:" + String(sensor_values[current_sensor]) + " "; // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  if(sensor_values[current_sensor] >= 5000){
    Serial.println("[MESSAGE] CO level is critically high!");
    sensor_alert[current_sensor] = 2;
    alarm += "MQ7_CO";
    process_warning();
  }
  else if(sensor_values[current_sensor] >= 1000){
    Serial.println("[MESSAGE] CO level is cautiously high!");
    sensor_alert[current_sensor] = 1;
    alarm += "MQ7_CO";
    process_error();
  }else{
    sensor_alert[current_sensor] = 0;

  }
  ++current_sensor;
  
/*
  //MQ8
  MQ8.update(); // Update data, the arduino will read the voltage from the analog pin
  payload += "MQ8_H2:" + String(MQ8.readSensor()) + " "; // Sensor will read PPM concentration using the model, a and b values set previously or from the setup
*/

  //MQ135
  MQ135.update(); // Update data, the arduino will read the voltage from the analog pin
  sensor_values[current_sensor] = MQ135.readSensor();
  payload += "MQ135_AQ:" + String(sensor_values[current_sensor]) + " "; // Sensor will read PPM concentration using the model, a and b values set previously or from the setup
  if(sensor_values[current_sensor] >= 7000){
    Serial.println("[MESSAGE] AQ level is critically high!");
    sensor_alert[current_sensor] = 2;
    alarm += "MQ135_AQ";
    process_warning();
  }
  else if(sensor_values[current_sensor] >= 1500){
    Serial.println("[MESSAGE] AQ level is cautiously high!");
    sensor_alert[current_sensor] = 1;
    alarm += "MQ135_AQ";
    process_error();
  }else{
    sensor_alert[current_sensor] = 0;

  }
  ++current_sensor;
  process_led(0, 0, 0);

  t1 = millis() - t_start;
  Serial.print("[DEBUG] t1: ");
  Serial.println(t1);

  // DSM501A
  static float lowPM25, lowPM1 = 0;

  lowPM25 += pulseIn(pin4_25um, LOW) / 1000.0;
  lowPM1 += pulseIn(pin2_1um, LOW) / 1000.0;
  sample_count++;

  t2 = millis() - t_start;
  Serial.print("[DEBUG] t2: ");
  Serial.println(t2);

  if ((millis() - t_start) >= sample_time) {
    process_led(LED_BRIGHTNESS, 0, LED_BRIGHTNESS);
    
    sensor_values[current_sensor] = sample_count;
    ++current_sensor;

    float r_25um = calc_low_ratio(lowPM25);
    float r_1um = calc_low_ratio(lowPM1);
    float pcs_25um = calc_c_pcs283ml(lowPM25);
    float pcs_1um = calc_c_pcs283ml(lowPM1);
    float ugm3_25um = calc_c_ugm3(lowPM25);
    sensor_values[current_sensor] = ugm3_25um;
    if(sensor_values[current_sensor] >= 100){
      Serial.println("[MESSAGE] Hazardous Air Quality");
      sensor_alert[current_sensor] = 2;
      alarm += "ugm3_25um";
      process_warning();
    }
    else if(sensor_values[current_sensor] >= 25){
      Serial.println("[MESSAGE] Unhealthy Air Quality!");
      sensor_alert[current_sensor] = 1;
      alarm += "ugm3_25um";
      process_error();
    }else{
      sensor_alert[current_sensor] = 0;
  
    }
    ++current_sensor;
    float ugm3_1um = calc_c_ugm3(lowPM1);
    sensor_values[current_sensor] = ugm3_1um;
    if(sensor_values[current_sensor] >= 300){
      Serial.println("[MESSAGE] Hazardous Air Quality");
      sensor_alert[current_sensor] = 2;
      alarm += "ugm3_1um";
      process_warning();
    }
    else if(sensor_values[current_sensor] >= 80){
      Serial.println("[MESSAGE] Unhealthy Air Quality!");
      sensor_alert[current_sensor] = 1;
      alarm += "ugm3_1um";
      process_error();
    }else{
      sensor_alert[current_sensor] = 0;
  
    }
    ++current_sensor;
    float t = (millis() - t_start)/1000.0;
    sensor_values[current_sensor] = t;
    ++current_sensor;


    payload += "t:" + String(t) + " " + "samples:" + String(sample_count) + " " + "r_25um:" + String(r_25um) + " " + "ugm3_25um:" + String(ugm3_25um) + " " + "pcs_25um:" + String(pcs_25um) + " " + "r_1um:" + String(r_1um) + " " + "ugm3_1um:" + String(ugm3_1um) + " " + "pcs_1um:" + String(pcs_1um) + " ";

    // reset
    lowPM25 = 0;
    lowPM1 = 0;
    sample_count = 0;
    t_start = millis();
    process_led(0, 0, 0);

    server_post_req(payload);
  }

  // Serial.println(payload);  
  // server_get_req();
  // delay(1000);

  process_led(0, 0, 0);


  // condition logic
  int alarm_count = 0;
  int warning_count = 0;

  for(int i=0; i<data_size; ++i){
    if(sensor_alert[i] == 2){
      alarm_count++;
    }
    if(sensor_alert[i] == 1){
      warning_count++;
    }
  }

  String condition;
  if(alarm_count >0){
    condition = "CRITICAL";
    analogWrite(ALARM_LED, 255);
    hazard_tone();
  }else if(warning_count > 0){
    condition = "CAUTION";
    analogWrite(ALARM_LED, 255);
    warning_tone();
  }else{
    condition = "SAFE";
    analogWrite(ALARM_LED, 0);
    noTone(BUZZER);
  }

  // Display
  const int textHeight = 8;    // Height of each line of text
  const int maxVisibleLines = SCREEN_HEIGHT / textHeight;
  const int scrollSpeed = 100; // Scroll speed in milliseconds

  // display summary
  display.clearDisplay();
  display.setTextSize(1); // Small text size
  display.setTextColor(SH110X_WHITE); // Draw white text on black background

  display.setCursor(0, 0); // Start at top-left corner
  display.print("CONDITION: ");
  display.print(condition);

  // Scroll text on the first line
  static int scrollOffset = 0; // Horizontal scrolling offset
  // String message = "This is a long string that needs to be scrolled. ";
  int charWidth = 6; // Width of one character in pixels for text size 1
  int displayWidth = 128; // Width of the display in pixels
  int visibleChars = displayWidth / charWidth; // Maximum characters visible at a time

  for(int i = 0; i < data_size; i++) {
    if(sensor_alert[i] == 1){
      message += sensor_warnings_msg[i];
      message += " ";
    }

    if(sensor_alert[i] == 2){
      message += sensor_alarms_msg[i];
      message += " ";
    }
  }

  // Calculate the starting index and number of characters to display
  int startIdx = scrollOffset / charWidth; // Start at the character corresponding to the offset
  int endIdx = startIdx + visibleChars; // End index for visible characters

  // Create a substring of the visible portion
  String visibleText = message.substring(startIdx, min(endIdx, message.length()));

  // Print the visible text
  display.setCursor(0, 8); // Top-left corner of the display
  display.print(visibleText);

  // Handle wrapping around when the text scrolls out
  if (endIdx >= message.length()) {
    int remainingChars = visibleChars - (message.length() - startIdx); // Characters left to fill
    String wrapText = message.substring(0, remainingChars);
    display.print(wrapText);
  }

  // Update the scroll offset
  scrollOffset += 6*charWidth;
  if (scrollOffset >= message.length() * charWidth) {
    scrollOffset = 0; // Reset offset when the text has fully scrolled
  }

  // Display sensor readings
  /*
  */
  const int summary_lines = 3;
  static int offset = 0; // Start offset for scrolling

  // Clear and draw the display
  display.setTextSize(1); // Small text size
  display.setTextColor(SH110X_BLACK, SH110X_WHITE); // Draw white text on black background
  display.setCursor(0, 2*textHeight); // Start at top-left corner
  display.print("---SENSOR READINGS---");

  for (int i = 0; i < maxVisibleLines; i++) {
    int dataIndex = (i + offset) % data_size;
    display.setTextSize(1); // Small text size
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, (i+summary_lines) * textHeight);
    display.print(data[dataIndex]);
    display.print(sensor_values[dataIndex]);
  }
  display.display();

  // Update the offset to scroll the display
  offset++;
  if (offset >= data_size) {
    offset = 0; // Reset offset when all lines have been scrolled
  }

  delay(scrollSpeed); // Adjust scroll speed

  t3 = millis() - t_start;
  Serial.print("[DEBUG] t3: ");
  Serial.println(t3);
}
