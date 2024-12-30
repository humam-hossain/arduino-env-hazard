#include <Arduino.h>
#include "DHT.h"
#include <MQUnifiedsensor.h>

// MQ Gas Sensors
#define PLACA "Arduino Mega 2560"
#define VOLTAGE_RESOLUTION 5
#define ADC_BIT_RESOLUTION 10

#define PIN_MQ5 A0                  //Analog input 0 of your arduino
#define TYPE_MQ5 "MQ-5"             //MQ5
#define RatioMQ5CleanAir 6.5        //RS / R0 = 6.5 ppm

#define PIN_MQ8 A1                  //Analog input 1 of your arduino
#define TYPE_MQ8 "MQ-8"             //MQ8
#define RatioMQ8CleanAir 70         //RS / R0 = 70 ppm 

#define PIN_MQ7 A2                  //Analog input 2 of your arduino
#define TYPE_MQ7 "MQ-7"             //MQ7
#define RatioMQ7CleanAir 27.5       //RS / R0 = 27.5 ppm

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

// ESP8266
#define ESP8266 Serial1
#define BAUD_RATE 115200

#define PING_ADDR "www.google.com"

// #define IP_ADDR "192.168.6.100"
// #define DEFAULT_GATEWAY "192.168.6.1"
// #define SUBNET_MASK "255.255.255.0"
// #define TCP_SERVER_PORT 333

#define SERVER_IP "192.168.0.100"
#define SERVER_PORT "8000"

// Function to send AT commands and wait for a valid response
String send_command(const String& command, unsigned long timeout, bool debug = false) {
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
  }

  Serial.println("[COMPLETED] " + command);
  return response;
}

// Function to ping an internet address
void ping_internet(int total_count, unsigned long timeout, bool debug = false) {
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
  }else if (count > 0){
    Serial.println("[WARNING] Internet connection is unstable.");
  }else{
    Serial.println("[ERROR] No internet connection.");
  }
}

// Function to set up the ESP8266
void setupESP8266() {
  Serial.println("[INFO] Configuring ESP8266...");

  Serial.println("[INFO] Resetting ESP8266...");
  send_command("AT+RST", 6500, true);
  Serial.println("[INFO] Turning on station mode on ESP8266");
  send_command("AT+CWMODE_CUR=1", 2000, true);

  // access to wifi
  String ssid = "no internet";
  String password = "humamhumam09";
  send_command("AT+CWSAP_CUR=\"" + ssid + "\",\"" + password + "\",5,3", 1000, true);

  // check ip and mac address
  send_command("AT+CIFSR", 1500, true);

  // check internet connection
  ping_internet(1, 2000);
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
  Serial.begin(BAUD_RATE); // Set up Serial communication for debugging
  while(!Serial){
    Serial.println("[WARNING] Waiting for Serial to be ready...");
  };  // Wait for Serial to be ready
  
  ESP8266.begin(BAUD_RATE); // Set up Serial1 for ESP8266 communication
  while(!ESP8266){
    Serial.println("[WARNING] Waiting for ESP8266 to be ready...");
  }  // Wait for ESP8266 to be ready

  String response = "";
  while(ESP8266.available()) {
    Serial.println(ESP8266.available());
    char c = ESP8266.read();
    response += c;
  }  // Clear the input buffer
  Serial.println(response);

  setupESP8266();

/*
  // setup gas sensors
  MQ5.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ8.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ7.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ135.setRegressionMethod(1); //_PPM =  a*ratio^b

  //MQ5
  MQ5.setA(80.897); MQ5.setB(-2.431); // Configure the equation to to calculate H2 concentration

  //   Exponential regression:
  // Gas    | a      | b
  // H2     | 1163.8 | -3.874
  // LPG    | 80.897 | -2.431
  // CH4    | 177.65 | -2.56
  // CO     | 491204 | -5.826
  // Alcohol| 97124  | -4.918
  
  MQ5.init();   
  Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ5.update(); // Update data, the arduino will read the voltage from the analog pin
    calcR0 += MQ5.calibrate(RatioMQ5CleanAir);
    Serial.print(".");
  }
  MQ5.setR0(calcR0/10);
  Serial.println("  done!.");
  if(isinf(calcR0)) {Serial.println("Warning: Conection issue, R0 is infinite (Open circuit detected) please check your wiring and supply"); while(1);}
  if(calcR0 == 0){Serial.println("Warning: Conection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply"); while(1);}

  //MQ8
  MQ8.setA(976.97); MQ8.setB(-0.688); // Configure the equation to to calculate H2 concentration
  
  //   Exponential regression:
  // GAS     | a      | b
  // H2      | 976.97  | -0.688
  // LPG     | 10000000 | -3.123
  // CH4     | 80000000000000 | -6.666
  // CO      | 2000000000000000000 | -8.074
  // Alcohol | 76101 | -1.86
  
  MQ8.init();
  Serial.print("Calibrating please wait.");
  float calcR01 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ8.update(); // Update data, the arduino will read the voltage from the analog pin
    calcR01 += MQ8.calibrate(RatioMQ8CleanAir);
    Serial.print(".");
  }
  MQ8.setR0(calcR01/10);
  Serial.println("  done!.");
  if(isinf(calcR01)) {Serial.println("Warning: Conection issue, R0 is infinite (Open circuit detected) please check your wiring and supply"); while(1);}
  if(calcR01 == 0){Serial.println("Warning: Conection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply"); while(1);}

  //MQ7
  MQ7.setA(99.042); MQ7.setB(-1.518); // Configure the equation to calculate CO concentration value

  //   Exponential regression:
  // GAS     | a      | b
  // H2      | 69.014  | -1.374
  // LPG     | 700000000 | -7.703
  // CH4     | 60000000000000 | -10.54
  // CO      | 99.042 | -1.518
  // Alcohol | 40000000000000000 | -12.35

  MQ7.init(); 
  Serial.print("Calibrating please wait.");
  float calcR02 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ7.update(); // Update data, the arduino will read the voltage from the analog pin
    calcR02 += MQ7.calibrate(RatioMQ7CleanAir);
    Serial.print(".");
  }
  MQ7.setR0(calcR02/10);
  Serial.println("  done!.");
  if(isinf(calcR02)) {Serial.println("Warning: Conection issue, R0 is infinite (Open circuit detected) please check your wiring and supply"); while(1);}
  if(calcR02 == 0){Serial.println("Warning: Conection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply"); while(1);}
  
  //MQ135
  MQ135.init(); 
  Serial.print("Calibrating please wait.");
  float calcR03 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ135.update(); // Update data, the arduino will read the voltage from the analog pin
    calcR03 += MQ135.calibrate(RatioMQ135CleanAir);
    Serial.print(".");
  }
  MQ135.setR0(calcR03/10);
  Serial.println("  done!.");
  
  if(isinf(calcR03)) {Serial.println("Warning: Conection issue, R0 is infinite (Open circuit detected) please check your wiring and supply"); while(1);}
  if(calcR03 == 0){Serial.println("Warning: Conection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply"); while(1);}

  // Exponential regression:
  // GAS      | a      | b
  // CO       | 605.18 | -3.937  
  // Alcohol  | 77.255 | -3.18 
  // CO2      | 110.47 | -2.862
  // Toluen  | 44.947 | -3.445
  // NH4      | 102.2  | -2.473
  // Aceton  | 34.668 | -3.369
*/
  // DHT Setup
  Serial.println(F("DHT test!"));
  dht.begin();

  // setup flame sensor
  pinMode(FLAME_SENSOR, INPUT);

}

void loop() {
  String payload = "";

  // DHT
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  payload = "humidity:" + String(h) + " temp:" + String(t) + " ";

/*
  //MQ5
  MQ5.update(); // Update data, the arduino will read the voltage from the analog pin
  Serial.print("MQ5_LPG:");
  Serial.print(MQ5.readSensor()); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup
  Serial.print(" ");
  //MQ8
  MQ8.update(); // Update data, the arduino will read the voltage from the analog pin
  Serial.print("MQ8_H2:");
  Serial.print(MQ8.readSensor()); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup
  Serial.print(" ");
  //MQ7
  MQ7.update(); // Update data, the arduino will read the voltage from the analog pin
  Serial.print("MQ7_CO:");
  Serial.print(MQ7.readSensor());// Sensor will read PPM concentration using the model, a and b values set previously or from the setup
  Serial.print(" ");
  //MQ135
  MQ135.update(); // Update data, the arduino will read the voltage from the analog pin

  MQ135.setA(605.18); MQ135.setB(-3.937); // Configure the equation to calculate CO concentration value
  float CO = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ135.setA(77.255); MQ135.setB(-3.18); //Configure the equation to calculate Alcohol concentration value
  float Alcohol = MQ135.readSensor(); // SSensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ135.setA(110.47); MQ135.setB(-2.862); // Configure the equation to calculate CO2 concentration value
  float CO2 = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ135.setA(44.947); MQ135.setB(-3.445); // Configure the equation to calculate Toluen concentration value
  float Toluen = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup
  
  MQ135.setA(102.2 ); MQ135.setB(-2.473); // Configure the equation to calculate NH4 concentration value
  float NH4 = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ135.setA(34.668); MQ135.setB(-3.369); // Configure the equation to calculate Aceton concentration value
  float Aceton = MQ135.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup
  
  Serial.print("MQ135_CO:"); Serial.print(CO); Serial.print(" ");
  Serial.print("MQ135_Alcohol:"); Serial.print(Alcohol); Serial.print(" ");
  Serial.print("MQ135_CO2:"); Serial.print(CO2 + 422.38); Serial.print(" ");
  Serial.print("MQ135_Toluen:"); Serial.print(Toluen); Serial.print(" ");
  Serial.print("MQ135_NH4:"); Serial.print(NH4); Serial.print(" ");
  Serial.print("MQ135_Aceton:"); Serial.println(Aceton);  
  Serial.println();
*/

  int flame_intensity = 1023 - analogRead(FLAME_SENSOR);
  payload += "flame:" + String(flame_intensity) + " ";
  // Serial.println(payload);

  payload += "t:125.80 samples:8 r_25um:6.68 ugm3_25um:0.68 pcs_25um:4176.45 r_1um:7.16 ugm3_1um:0.73 pcs_1um:4473.25";
  server_post_req(payload);
  // server_get_req();
  
  // delay(1000);
}
