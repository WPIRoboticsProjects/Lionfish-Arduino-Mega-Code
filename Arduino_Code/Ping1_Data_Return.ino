#include "ping1d.h"

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

static const uint8_t ledPin = 13;

void setup()
{
  Serial1.begin(115200);
  Serial2.begin(115200);
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  Serial.println("Blue Robotics ping1d-simple.ino");
  while (!ping1.initialize()) {
    Serial.println("\nPing Down failed to initialize!");
    Serial.println("Are the Ping rx/tx wired correctly?");
    Serial.print("Ping rx is the green wire, and should be connected to Arduino pin ");
    Serial.print(arduinoTxPin1);
    Serial.println(" (Arduino tx)");
    Serial.print("Ping tx is the white wire, and should be connected to Arduino pin ");
    Serial.print(arduinoRxPin1);
    Serial.println(" (Arduino rx)");
    delay(2000);
  }
  while (!ping2.initialize()) {
    Serial.println("\nPing Down failed to initialize!");
    Serial.println("Are the Ping rx/tx wired correctly?");
    Serial.print("Ping rx is the green wire, and should be connected to Arduino pin ");
    Serial.print(arduinoTxPin2);
    Serial.println(" (Arduino tx)");
    Serial.print("Ping tx is the white wire, and should be connected to Arduino pin ");
    Serial.print(arduinoRxPin2);
    Serial.println(" (Arduino rx)");
    delay(2000);
  }
  ping2.set_ping_enable(0);
}



void loop()
{
  if (ping1.update()) {
    Serial.print("Down Distance: ");
    Serial.print(ping1.distance());
    Serial.print("\tDown Confidence: ");
    Serial.println(ping1.confidence());
    ping1.set_ping_enable(0);
    ping2.set_ping_enable(1);
  } else {
    Serial.println("No Down update received!");
  }

  if (ping2.update()) {
    Serial.print("Forward Distance: ");
    Serial.print(ping2.distance());
    Serial.print("\tForward Confidence: ");
    Serial.println(ping2.confidence());
    ping2.set_ping_enable(0);
    ping1.set_ping_enable(1);
  } else {
    Serial.println("No Forward update received!");
  }

  // Toggle the LED to show that the program is running
  digitalWrite(ledPin, !digitalRead(ledPin));
}
