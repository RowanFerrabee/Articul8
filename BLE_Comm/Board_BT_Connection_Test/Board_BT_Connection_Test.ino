
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
    moduleSerial.write("Received data\n");
    moduleSerial.write(String(bluetoothBuffer.getSize()).c_str());
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
    
    moduleSerial.write("After write, ");
    moduleSerial.write(String(bluetoothBuffer.getSize()).c_str());
    moduleSerial.write(" bytes in the BT buffer\n");

    int foundPacketResult = bluetoothBuffer.findPacket();
    if(foundPacketResult == BUFFER_SUCCESS)
    {
      int packetStart = bluetoothBuffer.getPacketStart();
      int invalid = 0;
      uchar checksum = 0;
      if(bluetoothBuffer.peek(packetStart) != SOP) { invalid++; }
      for(int j = 1; j < PACKET_SIZE - 1 && !invalid; ++j)
      {
        checksum += bluetoothBuffer.peek(packetStart + j);
      }
      if(bluetoothBuffer.peek(packetStart + PACKET_SIZE - 1) != checksum) { invalid++; }

      if(invalid)
      {
        moduleSerial.write("find packet is fking up still\n");  
      }
      
      if (!bluetoothBuffer.readPacket((unsigned char*)serialBuffer))
      {
        moduleSerial.write("readPacket failed. Now, ");
        moduleSerial.write(String(bluetoothBuffer.getSize()).c_str());
        moduleSerial.write(" bytes in the BT buffer\n");
      }

      reversedBuffer[0] = SOP;
      reversedBuffer[PACKET_SIZE - 1] = serialBuffer[PACKET_SIZE - 1];

      // I'm only reversing the inner data, not the header and check sum
      for(unsigned i = 1; i < PACKET_SIZE - 1; ++i) {
        reversedBuffer[i] = serialBuffer[PACKET_SIZE - 1 - i];  
      }

      Serial.write(reversedBuffer, PACKET_SIZE);
      moduleSerial.write("Received packet, reversed it, and returned it \n");
      moduleSerial.write(serialBuffer, PACKET_SIZE);
      
    }
    else if(foundPacketResult == 4)
    {
      moduleSerial.write("Checksum failed, value was: ");
      moduleSerial.write(String(bluetoothBuffer.getError()).c_str());
      moduleSerial.write("\n");
    }
    else
    {
      moduleSerial.write("Failed to find packet - ErrNo: ");
      moduleSerial.write(String(foundPacketResult).c_str());
      moduleSerial.write("\n");

      moduleSerial.write("Buffer data: ");
      for (unsigned i = 0; i < bluetoothBuffer.getSize(); i++) {
        moduleSerial.write(bluetoothBuffer.peek(i));
      }
      moduleSerial.write("\n");
    }
  }
}
