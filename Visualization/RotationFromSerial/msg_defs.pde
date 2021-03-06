// packet related defs

static char PACKET_SIZE = 19;
static char PACKET_OVERHEAD = 2;
static char SOP = 253; //TODO
static char POS_SOP = 0;
static char POS_DATA = 1;
static char POS_CHECKSUM = char(PACKET_SIZE-1);

// message related defines

static char MSG_OVERHEAD = 1;

static char NONE_MSG = 0;
static char ACK_MSG = 1;
static char STANDBY_MSG = 2;
static char STATE_CHANGE_MSG = 3;
static char IMU_DATA_MSG = 4;
static char LRA_CONTROL_MSG = 5;
static char GUI_CONTROL_MSG = 6;
static char NUM_MSG_TYPES = 7;

static char START_RECORDING = 0;
static char STOP_RECORDING = 1;
static char START_EXERCISE = 2;
static char STOP_EXERCISE = 3;
static char PRINT_RECORDING = 4;

static boolean isValidPacket(byte[] packet, int packet_size) {
  if (packet_size != PACKET_SIZE) {
    println("Invalid PS");
    return false;
  }
  if (char(packet[0]) != SOP) {
    println("Invalid SOP");
    return false;
  }
  //char sum = 0;
  //for (int i = 0; i < POS_CHECKSUM; i++) {
  //  sum += packet[i];
  //}
  //if (packet[POS_CHECKSUM] != sum) {
  //  println("Invalid checksum");
  //  return false;
  //}
  return true;
}

static float get4bytesFloat(byte[] data, int offset) {
  String hexint=hex(data[offset+3])+hex(data[offset+2])+hex(data[offset+1])+hex(data[offset]); 
  return Float.intBitsToFloat(unhex(hexint)); 
}

static class IMUMsg {
  public Quaternion quat;
  public boolean validQuat;
  public IMUMsg(Quaternion quat_) {
    quat = quat_;
    validQuat = true;
  }
  public IMUMsg() {
    validQuat = false;
  }
  
  public static IMUMsg fromBytes(byte[] packet) {
    float w = get4bytesFloat(packet, 2);
    float x = get4bytesFloat(packet, 6);
    float y = get4bytesFloat(packet, 10);
    float z = get4bytesFloat(packet, 14);

    if (!Float.isNaN(x) && !Float.isNaN(y) && !Float.isNaN(z) && !Float.isNaN(w) &&
        !(x == 0 && y == 0 && z == 0)) {
      Quaternion quat = new Quaternion(w, x, y, z);
      return new IMUMsg(quat);
    }

    return new IMUMsg();
  }
}

static class LRAMsg {
  public byte[] intensities;
  public static int NUM_LRAS = 8;
  
  public LRAMsg() {
    intensities = new byte [NUM_LRAS];
    for (int i = 0; i < NUM_LRAS; i++) {
      intensities[i] = 0;
    }
  }

  public static LRAMsg fromBytes(byte[] packet) {
    LRAMsg lraMsg = new LRAMsg();
    for (int i = 0; i < NUM_LRAS; i++) {
      lraMsg.intensities[i] = packet[i+2];
    }
    return lraMsg;
  }
}
