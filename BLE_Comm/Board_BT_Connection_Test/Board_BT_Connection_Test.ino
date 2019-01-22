
// Communication bridge between module and laptop for debugging
#include <SoftwareSerial.h>

#define BT_RST 4
#define SRL_RX 12
#define SRL_TX 11
#define MOD_BAUD 19200
#define BT_BAUD 19200

#define SERIAL_BUFFER_SIZE 5
#define NUM_TRANSMISSIONS 1000

SoftwareSerial moduleSerial(SRL_RX, SRL_TX);

bool readyToTransmit = false;
bool doneTransmit = true;
int transmissionCount = 0;

char serialBuffer[SERIAL_BUFFER_SIZE];
char blankString[] = "     ";
char connectedString[] = "%CONN";
char disconnectedString[] = "%DISC";

void setup() {
    // Initialize serial pins
    pinMode(BT_RST, OUTPUT);
    pinMode(SRL_RX, INPUT);
    pinMode(SRL_TX, OUTPUT);

    digitalWrite(BT_RST, HIGH);
  
    // Initialize serial port
    Serial.begin(BT_BAUD);
    moduleSerial.begin(MOD_BAUD);

    Serial.flush();
    moduleSerial.flush();

    delay(10);
}

void loop() {
  if (Serial.available() > 0) {
    // Read first few bytes of incoming message
    strcpy(serialBuffer, blankString);
    Serial.readBytes(serialBuffer, SERIAL_BUFFER_SIZE);

    if (strcmp(serialBuffer, connectedString) == 0)
    {
      // Incoming message was about BT device being connected
      if (!readyToTransmit and !transmitting) {
        readyToTransmit = true;
      }
    }
    else if (strcmp(serialBuffer, disconnectedString) == 0)
    {
      // Incoming message was about BT device being disconnected
      if (transmitting) {
        transmitting = false;
      }
    }
    else
    {
      // Incoming message unknown
      // Do something?
    }
  }

  if (readyToTransmit && !transmitting) {
    transmissionCount = 0;
    readyToTransmit = false;
    transmitting = true;
  }

  if (transmitting) {
    // Make a transmission
    Serial.write('A');
    moduleSerial.write('A');
    transmissionCount++;

    if (transmissionCount >= NUM_TRANSMISSIONS) {
      transmitting = false;
      readyToTransmit = false; // Not necessary?  
    }
  }
}
