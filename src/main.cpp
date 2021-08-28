#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "FS.h"

// TODO: add sleep schedule (to extend lifetime?)

/* Put your SSID & Password */
const char* ssid = "NodeMCU";  // Enter SSID here
const char* password = "12345678";  //Enter Password here

/* Put IP Address details */
IPAddress local_ip(10,1,1,1);
IPAddress gateway(10,1,1,1);
IPAddress subnet(255,255,255,0);

ESP8266WebServer server(80);

// global variables
uint8_t LED1pin = D3;
uint8_t LED2pin = D1;
uint8_t LED3pin = D2;
uint8_t LED4pin = D4;
uint8_t LEDstatus;
String PATH = "/state.txt";

uint8_t LED_remember = false;
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
void setUpPins(uint8_t);

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
    LEDstatus = atoi(buffer);

    Serial.println("[INFO] Found saved state: " + String(atoi(buffer)));
    Serial.println(LEDstatus);
  }else{
    f = SPIFFS.open(PATH, "w+");
    checkOpen(f);
    LEDstatus = false;
  }

  LED_remember = LEDstatus;

  setUpPins(LEDstatus);

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

  if(LEDstatus != LED_remember){
    Serial.println("[INFO] new value written to eeprom");
    char write_value = (LEDstatus)? '1':'0';
    Serial.println(write_value);
    f.close();
    f = SPIFFS.open(PATH, "w+");
    f.println(write_value);
    f.flush();

    LED_remember = LEDstatus;
  }
}

void handle_OnConnect() {
  String saved_state = (LEDstatus)? "ON":"OFF";
  digitalWrite(LED1pin, LEDstatus);
  
  Serial.println("GPIO7 Saved State: " + saved_state);
  server.send(200, "text/html", SendHTML(LEDstatus)); 
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

void setUpPins(uint8_t status){
  pinMode(LED1pin, OUTPUT);
  pinMode(LED2pin, OUTPUT);
  pinMode(LED3pin, OUTPUT);
  pinMode(LED4pin, OUTPUT);

  digitalWrite(LED1pin, status);
  digitalWrite(LED2pin, status);
  digitalWrite(LED3pin, status);
  digitalWrite(LED4pin, status);
}

void handle_led1on() {
  LEDstatus = 1;
  // turn on all lights
  turnAllOn();
  Serial.println("GPIO7 Status: ON");
  server.send(200, "text/html", SendHTML(true)); 
}

void handle_led1off() {
  LEDstatus = 0;
  // turn off all lights
  turnAllOff();
  Serial.println("GPIO7 Status: OFF");
  server.send(200, "text/html", SendHTML(false)); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}
