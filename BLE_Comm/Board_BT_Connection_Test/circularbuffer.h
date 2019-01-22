#ifndef CIRC_BUFF_H
#define CIRC_BUFF_H

typedef unsigned char uchar;

#define PACKET_SIZE 40
#define SOP 253

template<unsigned int N>
class CircularBuffer
{
public:

  unsigned getSize() { return _size; }
  unsigned getSpace() { return N - _size; }

  bool readPacket(uchar *dst) {
    if(packetStart < 0) { return false; }

    // update size and head to skip bytes before start of packet
    _size -= packetStart;
    head += packetStart;
    // module the head
    while(head >= N) { 
        head -= N;
    }

    unsigned sanityCheck = read(dst, PACKET_SIZE);
    return sanityCheck == PACKET_SIZE;
  }
  
  bool findPacket() {
    unsigned s = getSize();
    
    // if we haven't received enough bytes for a packet, we can just return
    if (s < PACKET_SIZE) { return false; }

    uchar *header = NULL;
    //search with wrap vs search without wrap
    // case 1: no wrap 
    if(head + s <= N) {
      header = (uchar*)memchr(buf + head, SOP, s);
    } else {
      unsigned x = N - head;
      // search the first part
      header = (uchar*)memchr(buf + head, SOP, x);
      
      // if we don't find anything, search the next part
      if(header == NULL) {
        header = (uchar*)memchr(buf, SOP, s - x);
      }
    }

    // if we haven't found a header, there's no packet
    if(header == NULL) { return false; }

    // first byte is header, last byte is checksum
    unsigned headerOffset = header - buf;

    // check if there are still enough bytes for a packet
    if(s < headerOffset + PACKET_SIZE) { return false; }
    
    const unsigned NUM_OVERHEAD_BYTES = 2;
    unsigned o = headerOffset + 1;
    uchar checksum = 0;
    for(unsigned i = 0; i < PACKET_SIZE - NUM_OVERHEAD_BYTES; ++i)
    {
        // modulo offset
        while(o > N) o -= N;        
        checksum += buf[o++];
    }

    if(checksum == peek(headerOffset + PACKET_SIZE - 1)) {
      packetStart = headerOffset;
      return true;  
    } else {
      packetStart = -1;
      return false;
    }
}

  uchar peek(unsigned offset)
  {
    if(offset > getSize()) return 0;
    offset = head + offset;
    // modulo
    while(offset > N) offset -= N;
    return buf[offset];
  }
  
  int write(uchar *src, unsigned l) {
      // only copy if we have enough space
      if(getSpace() < l) { return -1; }

      // do the copy
      // two cases; either we can write with or without wrap around
      // case 1: without wrap around
      if(tail + l <= N) {
        memcpy(buf + tail, src, l);
      
      // case 2: with wrap around
      } else {
        unsigned x = N - tail;
        memcpy(buf + tail, src, x);
        memcpy(buf, src + x, l - x); 
      }

      // update size and tail
      _size += l;      
      tail += l;
      // modulo the tail
      while(tail >= N) { 
        tail -= N;
      }

      // return number of bytes written
      return l;
  }

  int read(uchar *dst, unsigned l) {
    if(getSize() < l) { return -1; }

    // do the copy
    // two cases; either with or without wrap around
    // case 1: without wrap around
    if(head + l <= N) {
      memcpy(dst, buf + head, l);
      
    // case 2: with wrap around
    } else {
      unsigned x = N - head;
      memcpy(dst, buf + head, x);
      memcpy(dst, buf, l - x); 
    }

    // update size and head
    _size -= l;
    head += l;
    // module the head
    while(head >= N) { 
        head -= N;
    }

    // return number of bytes read
    return l;
  }

private:
  uchar buf[N];  
  unsigned _size = 0;
  unsigned head = 0;
  unsigned tail = 0;
  int packetStart = -1;
};

#endif
