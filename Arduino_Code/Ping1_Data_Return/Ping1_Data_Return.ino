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

// Confidence Threshold
int Conf_thres = 80; // 80%

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
  Serial1.begin(115200);
  Serial2.begin(115200);
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  while (!ping1.initialize()) {
    delay(1000);
  }
  while (!ping2.initialize()) {
    delay(1000);
  }
  ping2.set_ping_enable(0);
}



void loop()
{
  
  if (ping1.update()) {
    if (ping1.confidence() > conf_thres) {
      sendMsg(0, 1, ping1.distance());
      ping1.set_ping_enable(0);
      ping2.set_ping_enable(1);
    }
  }

  if (ping2.update()) {
    if (ping2.confidence() > conf_thres) {
      sendMsg(0, 2, ping2.distance());
      ping2.set_ping_enable(0);
      ping1.set_ping_enable(1);
    }
  }

  getData();
  updateSpearPos();

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

void sendMsg(int msgType, int id, uint32_t value) {
  Serial.print("<Msg ");
  Serial.print(msgType);
  Serial.print(" ");
  Serial.print(id);
  Serial.print(" ");
  Serial.println(value);
  Serial.print(">");
}

void sendMsg(int msgType, int id, int value) {
  Serial.print("<Msg ");
  Serial.print(msgType);
  Serial.print(" ");
  Serial.print(id);
  Serial.print(" ");
  Serial.println(value);
  Serial.print(">");
}
