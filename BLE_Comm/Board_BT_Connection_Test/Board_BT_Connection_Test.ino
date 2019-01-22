
// Communication bridge between module and laptop for debugging
#include <SoftwareSerial.h>

#define BT_RST 4
#define SRL_RX 12
#define SRL_TX 11
#define MOD_BAUD 19200
#define BT_BAUD 19200

#define SERIAL_BUFFER_SIZE 100
#define NUM_TRANSMISSIONS 1000

SoftwareSerial moduleSerial(SRL_RX, SRL_TX);

bool readyToTransmit = false;
bool doneTransmit = true;
int transmissionCount = 0;
bool transmitting = false;

char serialBuffer[SERIAL_BUFFER_SIZE];
char reversedBuffer[SERIAL_BUFFER_SIZE];
char blankString[] = "     ";
char connectedString[] = "%CONN";
char disconnectedString[] = "%DISC";

#include "circularbuffer.h"

CircularBuffer<2*PACKET_SIZE> bluetoothBuffer;

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
  int nb = Serial.available();
  if (nb > 0) {
    
    // Read first few bytes of incoming message
    if(nb > SERIAL_BUFFER_SIZE)
    {
      // we have a problem
      moduleSerial.write("Linear buffer full");
      nb = SERIAL_BUFFER_SIZE;
    }
    
    Serial.readBytes(serialBuffer, nb);
    bluetoothBuffer.write((unsigned char*)serialBuffer, nb);

    bool foundPacket = bluetoothBuffer.findPacket();
    if(foundPacket)
    {
      bluetoothBuffer.readPacket((unsigned char*)serialBuffer);

      // I'm only reversing the inner data, not the header and check sum
      for(unsigned i = 1; i < PACKET_SIZE - 1; ++i) {
        reversedBuffer[i] = serialBuffer[PACKET_SIZE - 2 - i];  
      }

      Serial.write(reversedBuffer, PACKET_SIZE);
      moduleSerial.write("Received packet, reversed it, and returned it \n");
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
