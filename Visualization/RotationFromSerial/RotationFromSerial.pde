import processing.net.*;  //<>// //<>// //<>//
import processing.serial.*;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

int upperLegLength = 55;
int upperLegRadius = 15;
int lowerLegLength = 40;
int lowerLegRadius = 10;
int footLength = 50;

int bandXCoord = 675;
int bandYCoord = 0;
int textXCoord = 625;
int textYCoord = 70;
int plotXOffset = 140;
int plotYOffset = 150;

Client tcpClient;
int socketPort = 5432;

float rotx = PI/4;
float roty = PI/4;
float rotz = 0;
int failed = 0;
int packets = 0;
int receptions = 0;

int start_time = millis();
boolean recording = false;
static boolean exercising = false;

static final int N_SIDES = 1;
static final int BANDS_PER_SIDE = 2;

static LRAMsg[][] lastLraMsg;
static Quaternion[][] segQuats;
static Quaternion[][] initialSegQuats;
static boolean[][] gotInitialVals;
static float[][] spinFreqs;
static boolean[] drawEllipse;
static boolean[] isSpinning;
static float[][] lastDrawnIntensities;

Quaternion to_global = new Quaternion(0, 1, 0, 0);
static Quaternion rotQs;
static Quaternion rotQt = new Quaternion(1, 0, 0, 0);

boolean drawRecording = false;
boolean drawExercising = false;

void setup() {
  size(900, 540);
 //<>//
  fill(255); //<>// //<>//
  stroke(color(44, 48, 32));

  tcpClient = new Client(this, "127.0.0.1", socketPort);
  lastLraMsg = new LRAMsg[N_SIDES][BANDS_PER_SIDE];
  segQuats = new Quaternion[N_SIDES][BANDS_PER_SIDE];
  initialSegQuats = new Quaternion[N_SIDES][BANDS_PER_SIDE];
  gotInitialVals = new boolean[N_SIDES][BANDS_PER_SIDE];
  spinFreqs = new float[N_SIDES][BANDS_PER_SIDE];
  drawEllipse = new boolean[BANDS_PER_SIDE];
  isSpinning = new boolean[BANDS_PER_SIDE];
  lastDrawnIntensities = new float[BANDS_PER_SIDE][8];
  
  for(int i = 0; i < N_SIDES; i++)
  {
    lastLraMsg[i][0] = new LRAMsg(6);
    lastLraMsg[i][1] = new LRAMsg(8);
    for(int j = 0; j < BANDS_PER_SIDE; j++)
    {
      segQuats[i][j] = new Quaternion(1, 0, 0, 0);
      initialSegQuats[i][j] = new Quaternion(1, 0, 0, 0);
      gotInitialVals[i][j] = false;
      spinFreqs[i][j] = 0;
      drawEllipse[j] = true;
      isSpinning[j] = false;
      for (int k = 0; k < 8; k++) {
        lastDrawnIntensities[j][k] = 0;
      }
    }
  }

  background(0);

  fill(255);
  textSize(20);
  text("Thigh", bandXCoord-25, bandYCoord+405);

  fill(255);
  textSize(20);
  text("Shin", bandXCoord-23, bandYCoord+205);
  
  text("Top", plotXOffset+160, plotYOffset-40);
  text("Front", plotXOffset+160, plotYOffset-40+170);
  text("Side", plotXOffset+160, plotYOffset-40+340);
  
  strokeWeight(3);
  stroke(255);
  line(450, 0, 450, 540);
}

static void printQuat(Quaternion q)
{
  String s = " ";
  String r = Float.toString(q.w) +s+ Float.toString(q.x) +s+ Float.toString(q.y) +s+ Float.toString(q.z) + "\n";
  print(r);
}

static class Buffer {
  static int SIZE = PACKET_SIZE*4;
  static byte[] rxbuf = new byte[SIZE];
  static int rxBufLen = 0;
  static byte[] rxbuf1 = new byte[SIZE];
  static int rxBufLen1 = 0;
  
  static void tryWrite(Client tcpClient)
  {
    int nb = tcpClient.available();
    byte[] tmp = new byte[2*nb];
    if(nb > 0)
    {
        nb = tcpClient.readBytes(tmp);
    }
    
    int offset = 0;
    while(offset < nb && (rxBufLen < PACKET_SIZE || rxBufLen1 < PACKET_SIZE))
    {
      if(rxBufLen > 0 && rxBufLen < PACKET_SIZE)
      {
        int space = min(nb - offset, PACKET_SIZE - rxBufLen);
        System.arraycopy(tmp, offset, rxbuf, rxBufLen, space);
        rxBufLen += space;
        offset += space;
      }
      else if(rxBufLen == 0)
      {
        while(offset < nb && tmp[offset] != (byte)SOP)
        {
          offset++;
        }
       if(offset < nb)
       {
         int space = min(PACKET_SIZE, nb - offset);
         System.arraycopy(tmp, offset, rxbuf, rxBufLen, space);
         rxBufLen += space;
         offset += space;
       }
      }
      
      else if(rxBufLen1 > 0 && rxBufLen1 < PACKET_SIZE)
      {
        int space = min(nb - offset, PACKET_SIZE - rxBufLen1);
        System.arraycopy(tmp, offset, rxbuf1, rxBufLen1, space);
        rxBufLen1 += space;
        offset += space;
      }
      else if(rxBufLen1 == 0)
      {
        while(offset < nb && tmp[offset] != (byte)SOP)
        {
          offset++;
        }
          
       if(offset < nb)
       {
         int space = min(PACKET_SIZE, nb - offset);
         System.arraycopy(tmp, offset, rxbuf1, rxBufLen1, space);
         rxBufLen1 += space;
         offset += space;
       }
      }
    }
    
    return;
  }
  
  static Quaternion turnToX(PVector v)
  {
    double th = Math.atan2(v.y,v.x);

    //if(th > Math.PI / 2)
    //    th = th - Math.PI;

    //if(th < -Math.PI / 2)
    //    th = th + Math.PI;
  
    Quaternion q = new Quaternion((float)Math.cos(th/2), 0.0, 0.0, (float)Math.sin(th/2));
    return q;  
  }
  
  static PVector unitX = new PVector(1, 0, 0);
  static boolean processMsg(byte[] buf)
  {
    if(buf[0] != SOP)
    {
      print("SOP error\n");
      return false;
    }
    if(buf[1] == GUI_QUAT_MSG)
    {
      int left = buf[2];
      int upper = buf[3];

      int offset = 4;
      float w = ByteBuffer.wrap(buf, offset + 0, 4).order(ByteOrder.LITTLE_ENDIAN).getFloat();
      float x = ByteBuffer.wrap(buf, offset + 4, 4).order(ByteOrder.LITTLE_ENDIAN).getFloat();
      float y = ByteBuffer.wrap(buf, offset + 8, 4).order(ByteOrder.LITTLE_ENDIAN).getFloat();
      float z = ByteBuffer.wrap(buf, offset + 12, 4).order(ByteOrder.LITTLE_ENDIAN).getFloat();
      
      Quaternion q = new Quaternion(w, x, y, z);
      if(upper == 0)
      {
        rotQs = turnToX(q.rotateVec(unitX));
      }
      
      gotInitialVals[left][upper] = true;
      //else
      //{
      //  if(gotInitialVals[left][0])
      //  {
              
      //  }
      //}
      
      //if (gotInitialVals[left][upper] == false) {
        
      //  if(upper == 1 && gotInitialVals[left][0] == false)
      //  {}
      //  else
      //  {
      //    gotInitialVals[left][upper] = true;
      //    initialSegQuats[left][upper] = new Quaternion(w, x, y, z);
      //  }
        
      //  if(gotInitialVals[left][upper])
      //  {
      //    PVector v = new PVector(1, 0, 0);
      //    if(upper == 0)
      //      rotQs = turnToX(initialSegQuats[left][0].rotateVec(v));
      //    else
      //    {
      //      v = initialSegQuats[left][1].rotateVec(v);
      //      rotQt = turnToX(rotQs.rotateVec(v));
      //      //rotQt = new Quaternion( (float)Math.cos(Math.PI/2), 0, 0, (float)Math.sin(Math.PI/2) ).mult(rotQt);
      //    }
      //  }
      //}
      segQuats[left][upper] = new Quaternion(w, x, y, z);

      return true;
    }
    else if (buf[1] == GUI_LRA_MSG)
    {
      int left = buf[2];
      int upper = buf[3];
      int isSpin = buf[4];

      int numLRAs = 6;
      if (upper == 1) {
        numLRAs = 8;
      }

      if (!exercising) {
        spinFreqs[left][upper] = 0;
        lastLraMsg[left][upper] = new LRAMsg(numLRAs);
        return true;
      }

      if (isSpin != 0) {
        // TODO: handle spin
        spinFreqs[left][upper] = ByteBuffer.wrap(buf, 5, 4).order(ByteOrder.LITTLE_ENDIAN).getFloat();
        return true;
      }

      spinFreqs[left][upper] = 0;
      lastLraMsg[left][upper] = LRAMsg.fromBytes(buf, numLRAs);
      return true;
    }
    
    print("unknown msg type\n");
    return false;
  }
  
  static boolean get() 
  {
    if(rxBufLen == PACKET_SIZE)
    {
      //print("buffer0 ready\n");
      boolean success = processMsg(rxbuf);
      rxBufLen = 0;
      return success;
    }      
    else if(rxBufLen1 == PACKET_SIZE)
    {      
      boolean success = processMsg(rxbuf1);
      rxBufLen1 = 0;
      return success;
    }
    
    return false;
  }
}

boolean readPacket() {

  Buffer.tryWrite(tcpClient);

  if(tcpClient.available() > 20 * PACKET_SIZE)
  {
    print("Clearing\n");
    tcpClient.clear();
  }

  return Buffer.get();
}

int frameNum = 0;

void draw() {
  frameNum += 1;
  
  while(readPacket())
    packets++;
    
  // BIG DICK ENERGY COMMENT
  //background(0);

  //textSize(14);
  //text(packets, 40, 40);
  
  strokeWeight(3);
  stroke(255);
  //line(450, 0, 450, 540);

  if (drawRecording) {
    if (recording) {
      stroke(255, 0, 0);
      fill(255, 0, 0);
      ellipse(textXCoord, textYCoord, 10, 10);
      text("Recording", textXCoord+15, textYCoord+6);
    } else {
      noStroke();
      fill(0);
      rect(textXCoord-100, textYCoord-10, textXCoord+100, textYCoord-6);
    }
    drawRecording = false;
  } else if (drawExercising) {
    if (exercising) {
      stroke(0, 255, 0);
      fill(0, 255, 0);
      ellipse(textXCoord, textYCoord, 10, 10);
      text("Exercising", textXCoord+15, textYCoord+6);
    } else {
      noStroke();
      fill(0);
      rect(textXCoord-100, textYCoord-10, textXCoord+100, textYCoord-6);
    }
    drawExercising = false;
  }
  
  while(readPacket())
    packets++;

  if (lastLraMsg[0][0] != null) {
    stroke(255);
    strokeWeight(3);

    if (spinFreqs[0][0] < -0.01) {
      noFill();
      arc(bandXCoord, bandYCoord+200, 170, 170, 0, HALF_PI);
      isSpinning[0] = true;
      line(bandXCoord, bandYCoord+200+85, 4+bandXCoord, 4+bandYCoord+200+85);
      line(bandXCoord, bandYCoord+200+85, 4+bandXCoord, -4+bandYCoord+200+85);
    } else if (spinFreqs[0][0] > 0.01) {
      noFill();
      arc(bandXCoord, bandYCoord+200, 170, 170, -HALF_PI, 0);
      isSpinning[0] = true;
      line(bandXCoord, bandYCoord+200-85, 4+bandXCoord, 4+bandYCoord+200-85);
      line(bandXCoord, bandYCoord+200-85, 4+bandXCoord, -4+bandYCoord+200-85);
    } else {
      if (isSpinning[0] == true) {
        noFill();
        stroke(0);
        strokeWeight(5);
        arc(bandXCoord, bandYCoord+200, 170, 170, -HALF_PI, HALF_PI);
        line(bandXCoord, bandYCoord+200-85, 4+bandXCoord, 4+bandYCoord+200-85);
        line(bandXCoord, bandYCoord+200-85, 4+bandXCoord, -4+bandYCoord+200-85);
        line(bandXCoord, bandYCoord+200+85, 4+bandXCoord, 4+bandYCoord+200+85);
        line(bandXCoord, bandYCoord+200+85, 4+bandXCoord, -4+bandYCoord+200+85);
        isSpinning[0] = false;
      }
      noStroke();
      int numLRAs = lastLraMsg[0][0].numLRAs;
      for (int i = 0; i < numLRAs; i++) {
        if (lastDrawnIntensities[0][i] != lastLraMsg[0][0].intensities[i]) {
          drawEllipse[0] = true;
          float angle = 2*PI/numLRAs;
          lastDrawnIntensities[0][i] = lastLraMsg[0][0].intensities[i];
          int size = 20;
          if (lastLraMsg[0][0].intensities[i] < 5) {
            size = 23;
          }
          noStroke();
          fill(int(2*lastLraMsg[0][0].intensities[i]));
          ellipse(bandXCoord + 75*cos(i*angle), bandYCoord+200 + 75*sin(i*angle), size, size);
        }
      }
      if (drawEllipse[0]) {
        stroke(255);
        strokeWeight(3);
        noFill();
        ellipse(bandXCoord, bandYCoord+200, 150, 150);
        drawEllipse[0] = false;
      }
    }
  }

  while(readPacket())
    packets++;
  
  if (lastLraMsg[0][1] != null) {
    stroke(255);
    strokeWeight(3);

    if (spinFreqs[0][1] < -0.01) {
      noFill();
      arc(bandXCoord, bandYCoord+400, 170, 170, 0, HALF_PI);
      isSpinning[1] = true;
      line(bandXCoord, bandYCoord+400+85, 4+bandXCoord, 4+bandYCoord+400+85);
      line(bandXCoord, bandYCoord+400+85, 4+bandXCoord, -4+bandYCoord+400+85);
    } else if (spinFreqs[0][1] > 0.01) {
      noFill();
      arc(bandXCoord, bandYCoord+400, 170, 170, -HALF_PI, 0);
      isSpinning[1] = true;
      line(bandXCoord, bandYCoord+400-85, 4+bandXCoord, 4+bandYCoord+400-85);
      line(bandXCoord, bandYCoord+400-85, 4+bandXCoord, -4+bandYCoord+400-85);
    } else {
      if (isSpinning[1] == true) {
        noFill();
        stroke(0);
        strokeWeight(5);
        line(bandXCoord, bandYCoord+400+85, 4+bandXCoord, 4+bandYCoord+400+85);
        line(bandXCoord, bandYCoord+400+85, 4+bandXCoord, -4+bandYCoord+400+85);
        line(bandXCoord, bandYCoord+400-85, 4+bandXCoord, 4+bandYCoord+400-85);
        line(bandXCoord, bandYCoord+400-85, 4+bandXCoord, -4+bandYCoord+400-85);
        arc(bandXCoord, bandYCoord+400, 170, 170, -HALF_PI, HALF_PI);
        isSpinning[1] = false;
      }
      noStroke();
      int numLRAs = lastLraMsg[0][1].numLRAs;
      for (int i = 0; i < numLRAs; i++) {
        if (lastDrawnIntensities[1][i] != lastLraMsg[0][1].intensities[i]) {
          drawEllipse[1] = true;
          float angle = 2*PI/numLRAs;
          lastDrawnIntensities[1][i] = lastLraMsg[0][1].intensities[i];
          int size = 20;
          if (lastLraMsg[0][1].intensities[i] < 5) {
            size = 23;
          }
          noStroke();
          fill(int(2*lastLraMsg[0][1].intensities[i]));
          ellipse(bandXCoord + 75*cos(i*angle), bandYCoord+400 + 75*sin(i*angle), size, size);
        }
      }
      if (drawEllipse[1]) {
        stroke(255);
        strokeWeight(3);
        noFill();
        ellipse(bandXCoord, bandYCoord+400, 150, 150);
        drawEllipse[1] = false;
      }
    }
  }

  while(readPacket())
    packets++;

  if(gotInitialVals[0][0] && gotInitialVals[0][1])
  {
    stroke(255);
    PVector rFoot_LowLeg = new PVector(lowerLegLength, 0, 0);
    PVector rKnee_UpperLeg = new PVector(upperLegLength, 0, 0);
    
    //Quaternion lowerQuat = segQuats[0][0];
    //Quaternion upperQuat = segQuats[0][1];
    
    Quaternion lowerQuat = rotQs.mult(segQuats[0][0]);
    //Quaternion upperQuat = rotQt.mult(rotQs.mult(segQuats[0][1]));
    Quaternion upperQuat = rotQs.mult(segQuats[0][1]);
    
    rFoot_LowLeg = lowerQuat.rotateVec(rFoot_LowLeg);
    rKnee_UpperLeg = upperQuat.rotateVec(rKnee_UpperLeg);
    
    //if(rFoot_LowLeg.x * rKnee_UpperLeg.x > lowerLegLength * upperLegLength / 10)
    //{
    //  rotQt = new Quaternion((float)Math.cos(Math.PI/2), 0, 0, (float)Math.sin(Math.PI/2)).mult(rotQt);
    //  print("Switching view orientation");
    //}
    
    noStroke();
    fill(0);
    rect(plotXOffset-300, plotYOffset-100, 440, 3000);
    // XY Plot
    fill(255);
    strokeWeight(3);
    stroke(255);
    if (frameNum % 1 == 0) {
      text(" x", plotXOffset+70, plotYOffset);
      text(" y", plotXOffset, plotYOffset-70);
    }
    stroke(255,0,0);
    line(plotXOffset, plotYOffset, plotXOffset+70, plotYOffset);
    stroke(0,0,255);
    line(plotXOffset, plotYOffset, plotXOffset, plotYOffset-70);
    stroke(255,0,255);
    line(plotXOffset, plotYOffset, -rFoot_LowLeg.x + plotXOffset, -rFoot_LowLeg.y + plotYOffset);
    line(-rFoot_LowLeg.x + plotXOffset, -rFoot_LowLeg.y + plotYOffset,
         -rFoot_LowLeg.x + plotXOffset + -rKnee_UpperLeg.x, -rFoot_LowLeg.y + plotYOffset + -rKnee_UpperLeg.y
    );
    

    while(readPacket())
      packets++;

    // XZ Plot
    //plotYOffset += 170;
    stroke(255);
    if (frameNum % 1 == 0) {
      text(" x", plotXOffset+70, plotYOffset+170);
      text(" z", plotXOffset, plotYOffset-70+170);
    }
    stroke(255,0,0);
    line(plotXOffset, plotYOffset+170, plotXOffset+70, plotYOffset+170);
    stroke(0,255,0);
    line(plotXOffset, plotYOffset+170, plotXOffset, plotYOffset-70+170);
    stroke(255,0,255);
    line(plotXOffset, plotYOffset+170, -rFoot_LowLeg.x + plotXOffset, -rFoot_LowLeg.z + plotYOffset+170);
    line(-rFoot_LowLeg.x + plotXOffset, -rFoot_LowLeg.z + plotYOffset+170,
         -rFoot_LowLeg.x + plotXOffset + -rKnee_UpperLeg.x, -rFoot_LowLeg.z + plotYOffset+170 + -rKnee_UpperLeg.z
    );
    
    // YZ Plot
    //plotYOffset += 170;
    stroke(255);
    if (frameNum % 1 == 0) {
      text(" y", plotXOffset+70, plotYOffset+340);
      text(" z", plotXOffset, plotYOffset-70+340);
    }
    stroke(0,0,255);
    line(plotXOffset, plotYOffset+340, plotXOffset+70, plotYOffset+340);
    stroke(0,255,0);
    line(plotXOffset, plotYOffset+340, plotXOffset, plotYOffset-70+340);
    stroke(255,0,255);
    line(plotXOffset, plotYOffset+340, -rFoot_LowLeg.y + plotXOffset, -rFoot_LowLeg.z + plotYOffset+340);
    line(-rFoot_LowLeg.y + plotXOffset, -rFoot_LowLeg.z + plotYOffset+340,
         -rFoot_LowLeg.y + plotXOffset + -rKnee_UpperLeg.y, -rFoot_LowLeg.z + plotYOffset+340 + -rKnee_UpperLeg.z
    );
  }
}

void keyPressed() {
  byte[] guiControlMsg = new byte[PACKET_SIZE];
  guiControlMsg[POS_SOP] = byte(SOP);
  guiControlMsg[POS_DATA] = byte(GUI_CONTROL_MSG);

  if (key == 'r')
  {
    if (!recording) {
      guiControlMsg[POS_DATA+1] = byte(START_RECORDING);
      recording = true;
      drawRecording = true;
    } else {
      guiControlMsg[POS_DATA+1] = byte(STOP_RECORDING);
      recording = false;
      drawRecording = true;
    }
    tcpClient.write(guiControlMsg);
  }

  if (key == 'e')
  {
    if (!exercising) {
      guiControlMsg[POS_DATA+1] = byte(START_EXERCISE);
      exercising = true;
      drawExercising = true;
    } else {
      guiControlMsg[POS_DATA+1] = byte(STOP_EXERCISE);
      exercising = false;
      drawExercising = true;
    }
    tcpClient.write(guiControlMsg);
  }

  if (key == 'p')
  {
    guiControlMsg[POS_DATA+1] = byte(PRINT_RECORDING);
    tcpClient.write(guiControlMsg);
  }

  if (key == 'b')
  {
    guiControlMsg[POS_DATA+1] = byte(PRINT_BATTERY);
    tcpClient.write(guiControlMsg);
  }

  if (key == 'o')
  {
    guiControlMsg[POS_DATA+1] = byte(REPORT_OFFSETS);
    tcpClient.write(guiControlMsg);
  }

  if (key == 'c')
  {
    guiControlMsg[POS_DATA+1] = byte(CALIBRATE);
    tcpClient.write(guiControlMsg);
  }

  if (key == ' ')
  {
    to_global = segQuats[0][0].getInverse();
    start_time = millis();
    packets = 0;
  }
}
