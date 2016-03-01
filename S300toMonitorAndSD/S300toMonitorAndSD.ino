/*
  SD300toMonitorAndSD

  Get data from an ELT S300 print on monitor
  and record to SD

  based on
  Software serial multple serial test

  CONNECTIONS
  Arduino S300

  J11
  5V      1
  GND     2

  J12
  2      2
  3      1

  2016/02/19

  Remember SS is on pin 10
  11,12 and 13 are used bi SPI and SD

*/



#include <SoftwareSerial.h>
#include <SPI.h>
#include <SdFat.h>


// Log file base name.  Must be six characters or less.
#define FILE_BASE_NAME "Data"

// SD chip select pin.  Be sure to disable any other SPI devices such as Enet.
const uint8_t chipSelect = SS;

#define rxPin 2
#define txPin 3

SoftwareSerial mySerial(rxPin, txPin); // RX, TX

// File system object.
SdFat sd;

// Log file.
SdFile file;


// Time in micros for next data record.
uint32_t logTime;

//Serial input withEndMarker
//http://forum.arduino.cc/index.php?topic=288234.0
const byte numChars = 32;
char receivedChars[numChars];  // an array to store the received data
boolean newData = false;

// Error messages stored in flash.
#define error(msg) sd.errorHalt(F(msg))

int ppm;
//==============================================================================
// User functions.

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
void showNewData() {
  if (newData == true) {
    //Serial.print("This just in ... ");
    Serial.print(receivedChars);
    ppm = atoi(receivedChars);
    Serial.print(" ");
    Serial.print(ppm);
    Serial.println();
    logData(ppm);
    newData = false;
  }
}

//------------------------------------------------------------------------------
// Write data header.
void writeHeader() {
  file.print(F("seconds"));
  file.print(F(";ppm"));
  //file.print(i, DEC);
  file.println();
}

//------------------------------------------------------------------------------
// Log a data record.
void logData(int ppm) {

  // Write data to file.  Start with log time in micros.
  file.print(logTime);

  file.write(';');
  file.print(ppm);

  file.println();
}

//==============================================================================

void setup() {
  const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
  char fileName[13] = FILE_BASE_NAME "00.csv";


  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  Serial.println(F("S300toMonitorAndSD"));
  Serial.println(F("One line every 3 seconds!"));
  // set the data rate for the SoftwareSerial port
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  mySerial.begin(38400);

  Serial.println(F("Type any character to start"));
  while (!Serial.available()) {}

  // Initialize the SD card at SPI_HALF_SPEED to avoid bus errors with
  // breadboards.  use SPI_FULL_SPEED for better performance.
  if (!sd.begin(chipSelect, SPI_HALF_SPEED)) {
    sd.initErrorHalt();
  }

  // Find an unused file name.
  if (BASE_NAME_SIZE > 6) {
    error("FILE_BASE_NAME too long");
  }
  while (sd.exists(fileName)) {
    if (fileName[BASE_NAME_SIZE + 1] != '9') {
      fileName[BASE_NAME_SIZE + 1]++;
    } else if (fileName[BASE_NAME_SIZE] != '9') {
      fileName[BASE_NAME_SIZE + 1] = '0';
      fileName[BASE_NAME_SIZE]++;
    } else {
      error("Can't create file name");
    }
  }
  if (!file.open(fileName, O_CREAT | O_WRITE | O_EXCL)) {
    error("file.open");
  }
  do {
    delay(10);
  } while (Serial.read() >= 0);

  Serial.print(F("Logging to: "));
  Serial.println(fileName);
  Serial.println(F("Type any character to stop"));

  // Write data header.
  writeHeader();

  // Start on a multiple of the sample interval.
  logTime = 3;
}

void loop() { // run over and over
  /*
    if (mySerial.available()) {
    Serial.write(mySerial.read());
    }

    if (Serial.available()) {
    mySerial.write(Serial.read());
    }
  */
  recvWithEndMarker();
  receivedChars[7] = '\0';
  showNewData();
  if (Serial.available()) {
    // Close file and stop.
    file.close();
    Serial.println(F("Done"));
    while (1) {}
  }
}

