#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "FS.h"

/* Put your SSID & Password */
const char* ssid = "NodeMCU";  // Enter SSID here
const char* password = "12345678";  //Enter Password here

/* Put IP Address details */
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

ESP8266WebServer server(80);

// global variables
uint8_t LED1pin = D3;
uint8_t LED2pin = D3;
uint8_t LED3pin = D3;
uint8_t LED4pin = D3;
uint8_t LED1status;
String PATH = "/state.txt";

uint8_t LED1_remember = false;
char buffer[2];
File f;

// function declarations
void handle_OnConnect();
void handle_led1on();
void handle_led1off();
void handle_NotFound();
String SendHTML(uint8_t);
void checkOpen(File);

void turnAllOn();
void turnAllOff();

void setup() {
  Serial.begin(115200);

  // setup and load from flash
  if (SPIFFS.begin()){
    Serial.println("[INFO] filesystem mounted");
  }

  if (SPIFFS.exists(PATH)){
    f = SPIFFS.open(PATH, "r+");
    checkOpen(f);
    f.seek(0);
    Serial.println(f.available());

    String value = f.readStringUntil('\n');
    value.toCharArray(buffer, 2);
    LED1status = atoi(buffer);

    Serial.println("[INFO] Found saved state: " + String(atoi(buffer)));
    Serial.println(LED1status);
  }else{
    f = SPIFFS.open(PATH, "w+");
    checkOpen(f);
    LED1status = false;
  }

  LED1_remember = LED1status;

  pinMode(LED1pin, OUTPUT);
  digitalWrite(LED1pin, LED1status);

  // configure webserver
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);
  
  server.on("/", handle_OnConnect);
  server.on("/led1on", handle_led1on);
  server.on("/led1off", handle_led1off);
  server.onNotFound(handle_NotFound);
  
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();

  if(LED1status != LED1_remember){
    Serial.println("[INFO] new value written to eeprom");
    char write_value = (LED1status)? '1':'0';
    Serial.println(write_value);
    f.close();
    f = SPIFFS.open(PATH, "w+");
    f.println(write_value);
    f.flush();

    LED1_remember = LED1status;
  }
}

void handle_OnConnect() {
  String saved_state = (LED1status)? "ON":"OFF";
  digitalWrite(LED1pin, LED1status);
  
  Serial.println("GPIO7 Saved State: " + saved_state);
  server.send(200, "text/html", SendHTML(LED1status)); 
}

void turnAllOn(){
  digitalWrite(LED1pin, LOW);
  digitalWrite(LED2pin, LOW);
  digitalWrite(LED3pin, LOW);
  digitalWrite(LED4pin, LOW);
}

void turnAllOff(){
  digitalWrite(LED1pin, HIGH);
  digitalWrite(LED2pin, HIGH);
  digitalWrite(LED3pin, HIGH);
  digitalWrite(LED4pin, HIGH);
}

void handle_led1on() {
  LED1status = 1;
  // turn on all lights
  turnAllOn();
  Serial.println("GPIO7 Status: ON");
  server.send(200, "text/html", SendHTML(true)); 
}

void handle_led1off() {
  LED1status = 0;
  // turn off all lights
  turnAllOff();
  Serial.println("GPIO7 Status: OFF");
  server.send(200, "text/html", SendHTML(false)); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}
