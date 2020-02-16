#include "ping1d.h"
#include <Servo.h>

// SOS Leak Detector
int leakPin = 22; // digital pin 22
int leak = 0;

// ESC Setup
byte servoPin = 4;  // Spear motor connected as PWM pin 4
Servo servo;        // Create servo variable

int analogPin1 = A0;  // Battery Voltage Input - PIN 4 to analog pin 0
int analogPin2 = A1;  // Battery Current Input - PIN 3 to analog pin 1
int V = 0;            // Raw voltage measurement
int I = 0;            // Raw current measurement
int Batt_V = 0;       // Variable to store Battery Voltage
int Batt_A = 0;       // Variable to store Battery Current

// Ping Down Pins
static const uint8_t arduinoRxPin1 = 19; //Serial1 rx
static const uint8_t arduinoTxPin1 = 18; //Serial1 tx

// Ping Forward Pins
static const uint8_t arduinoRxPin2 = 17; //Serial2 rx
static const uint8_t arduinoTxPin2 = 16; //Serial2 tx

// Ping Down
static Ping1D ping1 { Serial1 };

// Ping Forward
static Ping1D ping2 { Serial2 };

// Confidence Threshold
int conf_thres = 70; // 80%

static const uint8_t ledPin = 13;

int spearPos = 0; // 0 = retracted, 1 = extended
int newSpearPos = 0;

const byte buffSize = 40;
char inputBuffer[buffSize];
const char startMarker = '<';
const char endMarker = '>';
byte bytesRecvd = 0;
boolean readInProgress = false;
boolean newDataFromPC = false;
char messageFromPC[buffSize] = {0};

void setup()
{
  pinMode(leakPin, INPUT); // Sets leak sensor as digital input
  
  Serial1.begin(115200);
  Serial2.begin(115200);
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  while (!ping1.initialize()) {
    delay(1000);
    Serial.print("Ping 1 not init");
  }
  while (!ping2.initialize()) {
    delay(1000);
    Serial.print("Ping 2 not init");
  }
  ping2.set_ping_enable(0);

  servo.attach(servoPin);        // Attach servo to servo pin 4
  servo.writeMicroseconds(1500); // Send "stop" signal to ESC
  delay(1000);                   // Delay to allow the ESC to recognize the stopped signal
}



void loop()
{


  
  if (ping1.update()) {
//    if (ping1.confidence() > conf_thres) {
      sendMsg(0, 1, ping1.distance(), ping1.confidence());
      ping1.set_ping_enable(0);
      ping2.set_ping_enable(1);
//    }
  }

  if (ping2.update()) {
//    if (ping2.confidence() > conf_thres) {
      sendMsg(0, 2, ping2.distance(), ping2.confidence());
      ping2.set_ping_enable(0);
      ping1.set_ping_enable(1);
//    }
  }

  getData();
  updateSpearPos();
  getBatteryStatus();
  getLeakStatus();

  

  // Toggle the LED to show that the program is running
  digitalWrite(ledPin, !digitalRead(ledPin));
}

void updateSpearPos(){
  if(newSpearPos != spearPos){
    if(newSpearPos == 0){
      //retract spear
      spearPos = 0;
      sendMsg(1, 0, 0);
    } else if(newSpearPos == 1){
      //extend spear
      spearPos = 1;
      sendMsg(1, 0, 1);
    }
  }
}

void getBatteryStatus() {
  // Battery Data
  V = analogRead(analogPin1);           // Read the battery voltage pin
  I = analogRead(analogPin2);           // Read the battery current pin
  Batt_V = V*(5.0/1024)*11.0;           // Convert analog read to actual voltage value
  Batt_A = (I*(5.0/1024)-0.33)*38.8788; // Convert analog read to actual current value


  // If battery voltage drops below threshold, send to Jetson
  if (Batt_V <= 12.2) {
    sendMsg(9, 1, Batt_V);          // Send Voltage to return home
  }
  else if (Batt_A >= 90){
    sendMsg(9, 2, Batt_A);          // Send Current to return home
  }
  
}

void getLeakStatus() {
  // Leak Sensor
  int leakState = digitalRead(leakPin); // read the input pin

  if (leakState == 1) {            // Sends Leak to Jetson if input pin is high
    sendMsg(9, 3, 1);              
  }
}

void getData() {
  
  if(Serial.available() > 0) {

    char x = Serial.read();

    if (x == endMarker) {
      readInProgress = false;
      newDataFromPC = true;
      inputBuffer[bytesRecvd] = 0;
      parseData();
    }
    
    if(readInProgress) {
      inputBuffer[bytesRecvd] = x;
      bytesRecvd ++;
      if (bytesRecvd == buffSize) {
        bytesRecvd = buffSize - 1;
      }
    }

    if (x == startMarker) { 
      bytesRecvd = 0; 
      readInProgress = true;
    }
  }
}
 
void parseData() {

    // split the data into its parts
    
  char * strtokIndx;
  
  strtokIndx = strtok(inputBuffer,",");
  newSpearPos = atoi(strtokIndx);

}

void sendMsg(int msgType, int id, uint32_t value, uint16_t value2) {
  Serial.print("<Msg ");
  Serial.print(msgType);
  Serial.print(" ");
  Serial.print(id);
  Serial.print(" ");
  Serial.print(value);
  Serial.print(" ");
  Serial.print(value2);
  Serial.println(">");
}

void sendMsg(int msgType, int id, int value) {
  Serial.print("<Msg ");
  Serial.print(msgType);
  Serial.print(" ");
  Serial.print(id);
  Serial.print(" ");
  Serial.print(value);
  Serial.println(">");
}
