#ifndef BT_MAN_H
#define BT_MAN_H

#include "msg_defs.h"

void populateBTPacket(uchar* packetBytes);
void sendBTPacket(uchar* packetBytes);
bool checkForBTPacket(uchar* dst);

#endif
