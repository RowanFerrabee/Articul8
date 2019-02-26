#ifndef BT_MAN_H
#define BT_MAN_H

#include "msg_defs.h"
#include "circular_buffer.h"

#define SERIAL_BUFFER_SIZE 100

#define BtSerial Serial1

CircularBuffer<4*PACKET_SIZE> bluetoothBuffer;
char serialRxBuffer[SERIAL_BUFFER_SIZE];

// Helper function to add SOP and checksum to packet
void populateBTPacket(uchar* packetBytes)
{
  packetBytes[POS_SOP] = SOP;

  uchar checksum = 0;
  for (int i = 0; i < POS_CHECKSUM; i++) {
    checksum += packetBytes[i];
  }
  
  packetBytes[POS_CHECKSUM] = checksum;
}

void sendBTPacket(uchar* packetBytes)
{
  BtSerial.write(packetBytes, PACKET_SIZE);
}

bool checkForBTPacket(uchar* dst)
{
  int nb = BtSerial.available();
  
  if (nb > 0) {
    
    if(nb > SERIAL_BUFFER_SIZE) {
      
      Serial.write("Linear buffer full");     // This is a problem
      nb = SERIAL_BUFFER_SIZE;
    }
    
    BtSerial.readBytes(serialRxBuffer, nb);             // Read incoming message    
    bluetoothBuffer.write((unsigned char*)serialRxBuffer, nb);

    int foundPacketResult = bluetoothBuffer.findPacket();
    if(foundPacketResult == BUFFER_SUCCESS)
    {
      // Populate buffer with first complete BT packet
      if (bluetoothBuffer.readPacket(dst))
      {
        Serial.println("Received BT packet");
        return true;
      }
    }

  }
  
  return false;
}

#endif
