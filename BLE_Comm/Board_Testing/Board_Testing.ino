
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

char streamCmd[] =  "STREAMPLSSTREAMPLSSTREAMPLSSTREAMPLSSS";
char okResponse[] = "OKOKOKOKOKOKOKOKOKOKOKOKOKOKOKOKOKOKOK";
char serialBuffer[SERIAL_BUFFER_SIZE];
char reversedBuffer[SERIAL_BUFFER_SIZE];

#include "circularbuffer.h"
CircularBuffer<4*PACKET_SIZE> bluetoothBuffer;

//#include "AMessage.h"
//template <typename MessageType> 
//int sendMessage(const MessageType &m) { return ArticulateMessages::writeMessage(Serial, m); }

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
   // moduleSerial.write("Received data: ");
   // moduleSerial.write(String(bluetoothBuffer.getSize()).c_str());
   // moduleSerial.write(" bytes in the BT buffer\n");
    
    // Read first few bytes of incoming message
    if(nb > SERIAL_BUFFER_SIZE)
    {
      // we have a problem
      moduleSerial.write("Linear buffer full");
      nb = SERIAL_BUFFER_SIZE;
    }
    
    Serial.readBytes(serialBuffer, nb);
    bluetoothBuffer.write((unsigned char*)serialBuffer, nb);

    int foundPacketResult = bluetoothBuffer.findPacket();
    if(foundPacketResult == BUFFER_SUCCESS)
    {
      if (!bluetoothBuffer.readPacket((unsigned char*)serialBuffer))
      {
        moduleSerial.write("readPacket failed!");
      }

      reversedBuffer[0] = SOP;
      reversedBuffer[PACKET_SIZE - 1] = serialBuffer[PACKET_SIZE - 1];

      // I'm only reversing the inner data, not the header and check sum
      for(unsigned i = 1; i < PACKET_SIZE - 1; ++i) {
        reversedBuffer[i] = serialBuffer[PACKET_SIZE - 1 - i];  
      }

      Serial.write(reversedBuffer, PACKET_SIZE);
//      moduleSerial.write("Received packet, reversed it, and returned it.\n Sent: ");
//      moduleSerial.write(reversedBuffer, PACKET_SIZE);
//      moduleSerial.write("\n");
    }
    else if(foundPacketResult == 4)
    {
      moduleSerial.write("Checksum failed, value was: ");
      moduleSerial.write(String(bluetoothBuffer.getError()).c_str());
      moduleSerial.write("\n");
    }
//    else
//    {
//      moduleSerial.write("Failed to find packet - ErrNo: ");
//      moduleSerial.write(String(foundPacketResult).c_str());
//      moduleSerial.write("\n");
//
//      moduleSerial.write("Buffer data: ");
//      for (unsigned i = 0; i < bluetoothBuffer.getSize(); i++) {
//        moduleSerial.write(bluetoothBuffer.peek(i));
//      }
//      moduleSerial.write("\n");
//    }
  }
}
