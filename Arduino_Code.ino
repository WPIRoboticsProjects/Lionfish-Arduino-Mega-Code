#include "ping1d.h"
#include <Servo.h>

#define SOSPIN 22 // SOS leak detector connected to digital pin 22

// ESC Setup
byte servoPin = 4;  // Spear motor connected as PWM pin 4
Servo servo;        // Create servo variable

// Ping Down Pins
static const uint8_t arduinoRxPin1 = 19;  //Serial1 rx
static const uint8_t arduinoTxPin1 = 18;  //Serial1 tx

// Ping Forward Pins
static const uint8_t arduinoRxPin2 = 17;  //Serial2 rx
static const uint8_t arduinoTxPin2 = 16;  //Serial2 tx

// Ping Down
static Ping1D ping1 { Serial1 };  // Establish Ping Down Serial Communication

// Ping Forward
static Ping1D ping2 { Serial2 };  // Estblish Ping Forward Serial Communication

static const uint8_t ledPin = 13; // Set LED pin

int analogPin1 = A0;  // Battery Voltage Input - analog pin 0
int analogPin2 = A1;  // Battery Current Input - analong pin 1
int V = 0;            // Raw voltage measurement
int I = 0;            // Raw current measurement
int Batt_V = 0;       // Variable to store Battery Voltage
int Batt_A = 0;       // Variable to store Battery Current


// Initialize Loop
void setup()
{
  pinMode(SOSPIN, INPUT);   // Sets leak sensor as digital input pin 22
  Serial1.begin(115200);    // Begin Down Serial Communication
  Serial2.begin(115200);    // Begin Forward Serial Communication
  Serial.begin(9600);       // Begin Serial Communication
  pinMode(ledPin, OUTPUT);  // Set LED pin as output

  servo.attach(servoPin);        // Attach servo to servo pin 4
  servo.writeMicroseconds(1500); // Send "stop" signal to ESC
  delay(1000);                   // Delay to allow the ESC to recognize the stopped signal
  
  // Ping Down Initialize Loop
  // If ping 1 communication fails, print to serial monitor
  while (!ping1.initialize()) {
    Serial.println("\nDown Fail");
  }

  // Ping Forward Initialize Loop
  // If ping 2 communication fails, print to serial monitor
  while (!ping2.initialize()) {
    Serial.println("\nForward Fail");
  }
  ping2.set_ping_enable(0);   // Disable Forward Ping
}


// Receive Sonar Distance and Confidence Data, Battery Data, and Leak Data
void loop()
{
  
  // If down ping confidence level > 80%, then send distance reading to Jetson
  if (ping1.update()) {
    if (ping1.confidence() > 80) {
      Serial.print(ping1.distance());
      delay(100);                      // Delay to let sound wave to dissipate before pinging other
      ping1.set_ping_enable(0);       // Disable Down Ping
      ping2.set_ping_enable(1);       // Enable Forward Ping
    }
  } else {
    Serial.println("No Down update received!");
  }
  
  // If forward ping confidence level > 80%, then send distance reading to Jetson
  if (ping2.update()) {
    if (ping2.confidence() > 80) {
      Serial.print(ping2.distance());
      ping2.set_ping_enable(0);       // Disable Forward Ping
      ping1.set_ping_enable(1);       // Enable Forward Ping
    }
  } else {
    Serial.println("No Forward update received!");
  }

  // Battery Data
  V = analogRead(analogPin1);           // Read the battery voltage pin
  I = analogRead(analogPin2);           // Read the battery current pin
  Batt_V = V*(5.0/1024)*11.0;           // Convert analog read to actual voltage value
  Batt_A = (I*(5.0/1024)-0.33)*38.8788; // Convert analog read to actual current value

  // If battery voltage drops below threshold, send to Jetson
  if (Batt_V <= 12.0) {
    Serial.println(Batt_V);          // Send Voltage to return home
  }
  // else if (Batt_A <= # OR Batt_A >= #){
    // Serial.println(Batt_A);          // Send Current to return home
  // }
  else {
                                     // If battery is OK, do nothing
  }
  
  // Leak Sensor
  int leakState = digitalRead(SOSPIN); // read the input pin

  if (leakState == HIGH) {            // prints “LEAK!” to Jetson if input pin is high
    Serial.println("LEAK");
  }
  else if (leakState == LOW) {        // if input pin is low (dry), do nothing
  }

  // ESC Controls for Spear Motor
  int signal = 1700; // Set signal value, which should be between 1100 and 1900
  servo.writeMicroseconds(signal); // Send signal to ESC.

  // Toggle the LED to show that the program is running
  digitalWrite(ledPin, !digitalRead(ledPin));
}
