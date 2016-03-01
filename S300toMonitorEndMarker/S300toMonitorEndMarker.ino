/*

  SD300toMonitorEndMarker

*/
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SdFat.h>

#define rxPin 10
#define txPin 11

SoftwareSerial mySerial(rxPin, txPin); // RX, TX

//Serial input withEndMarker
//http://forum.arduino.cc/index.php?topic=288234.0
const byte numChars = 32;
char receivedChars[numChars];  // an array to store the received data
boolean newData = false;

void setup() {
  Serial.begin(115200);
  Serial.println("S300toMonitorEndMarker");

  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  mySerial.begin(38400);
}

void loop() {
  recvWithEndMarker();
  receivedChars[7] = '\0';
  showNewData();
}

void recvWithEndMarker() {
  static byte ndx = 0;
  char endMarker = '\n';
  char rc;

  // if (mySerial.available() > 0) {
  while (mySerial.available() > 0 && newData == false) {
    rc = mySerial.read();

    if (rc != endMarker) {
      receivedChars[ndx] = rc;
      ndx++;
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    }
    else {
      receivedChars[ndx] = '\0'; // terminate the string
      ndx = 0;
      newData = true;
    }
  }
}

void showNewData() {
  if (newData == true) {
    //Serial.print("This just in ... ");
    Serial.println(receivedChars);
    newData = false;
  }
}


