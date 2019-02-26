#ifndef A_MESSAGE_H
#define A_MESSAGE_H

#include "Arduino.h"
#include "msg_defs.h"

#define MSG_OVERHEAD 1
namespace ArticulateMessages 
{

  template<typename Message>
  int writeMessage(HardwareSerial &refSer, const Message &m)
  {
      uchar linBuf [PACKET_SIZE];

      // set packet required header
      linBuf[POS_SOP] = SOP;
      
      linBuf[POS_DATA] = m.getMsgType();
      const int SPACE = PACKET_SIZE - PACKET_OVERHEAD - MSG_OVERHEAD;
      
      int numBytesWritten = m.writeData(linBuf + POS_DATA + 1, SPACE);
      
      if(numBytesWritten < 1 || numBytesWritten > SPACE)  { return -1; }

      memset(linBuf + POS_DATA + 1 + numBytesWritten, 0, SPACE - numBytesWritten);
  
      uchar checksum = 0;
      for(int i = 1; i < PACKET_SIZE-1; ++i) { checksum += linBuf[i]; }
      linBuf[POS_CHECKSUM] = checksum;

      refSer.write(linBuf, PACKET_SIZE);
      
      return 0;
  }

  enum MessageType 
  {
    NullMessageType = 0,
    IMUMessageType = 1
  };
}

/*
 * 
 * Message Requirements

struct MyMessage {
  
  int writeData(uchar* buf, int l) const;
  ArticulateMessages::MessageType getMsgType() const;
};

*/

struct NullMessage {
  
  int writeData(uchar* buf, int l) const;
  
  // an idea would be to also parse messages this way?
  // static AMessage fromData(uchar* buf, int size, bool* success);
  
};

struct IMUMessage
{
  float x,y,z,w;

  void writeFloatLSBFirst(const float &f, uchar* buf) const
  {
    uchar* b = reinterpret_cast<uchar*>(f);
    
    for(int i = 0; i < sizeof(f); ++i)
      buf[i] = b[i];
    
  }

  ArticulateMessages::MessageType getMsgType() const { return ArticulateMessages::IMUMessageType; }
  
  int writeData(uchar* buf, int l) const
  {
      if(l < sizeof(float)*4) { return -1; }

      // either pray that byte order on this machine is the same as byte order on receiving end...
      // or better, don't pray

      writeFloatLSBFirst(x, buf);
      buf += sizeof(float);
      
      writeFloatLSBFirst(y, buf);
      buf += sizeof(float);
      
      writeFloatLSBFirst(z, buf);
      buf += sizeof(float);
      
      writeFloatLSBFirst(w, buf);
      buf += sizeof(float);      

      return 
  }


};

#endif
