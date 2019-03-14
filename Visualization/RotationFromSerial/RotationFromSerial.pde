import processing.net.*;  //<>//
import processing.serial.*;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import shapes3d.*;
import shapes3d.animation.*;
import shapes3d.utils.*;

Ellipsoid upperLeg;
int upperLegLength = 55;
int upperLegRadius = 15;
int lowerLegLength = 40;
int lowerLegRadius = 10;
Ellipsoid lowerLeg;
Box foot;
int footLength = 50;

void setupLeg()
{

  upperLeg = new Ellipsoid(this, 20, 20);
  upperLeg.setRadius(upperLegLength, upperLegRadius, upperLegRadius);
  upperLeg.fill(#19BF38);

  lowerLeg = new Ellipsoid(this, 20, 20);
  lowerLeg.setRadius(lowerLegLength, lowerLegRadius, lowerLegRadius);
  lowerLeg.fill(#19BF38);  
  
  foot = new Box(this, footLength/5, footLength, footLength/5.0);
  
}

PImage textures[];
String texNames[] = {"front.png", "back.png", "bottom.png", "top.png", "right.png", "left.png"};

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
boolean exercising = false;

static final int N_SIDES = 1;
static final int BANDS_PER_SIDE = 2;

static LRAMsg[][] lastLraMsg;
static Quaternion[][] segQuats;
static Quaternion[][] initialSegQuats;
static boolean[][] gotInitialVals;
static float[][] spinFreqs;

Quaternion to_global = new Quaternion(0, 1, 0, 0);
static Quaternion rotQs;
static Quaternion rotQt;


void setup() {
  size(900, 540, P3D);
  textures = new PImage[6];
  for (int i = 0; i < 6; i++) {
    textures[i] = loadImage(texNames[i]);
  }
  textureMode(NORMAL);
  fill(255); //<>//
  stroke(color(44, 48, 32));

  tcpClient = new Client(this, "127.0.0.1", socketPort);
  lastLraMsg = new LRAMsg[N_SIDES][BANDS_PER_SIDE];
  segQuats = new Quaternion[N_SIDES][BANDS_PER_SIDE];
  initialSegQuats = new Quaternion[N_SIDES][BANDS_PER_SIDE];
  gotInitialVals = new boolean[N_SIDES][BANDS_PER_SIDE];
  spinFreqs = new float[N_SIDES][BANDS_PER_SIDE];
  
  for(int i = 0; i < N_SIDES; i++)
  {
    for(int j = 0; j < BANDS_PER_SIDE; j++)
    {
      lastLraMsg[i][j] = new LRAMsg(6);
      segQuats[i][j] = new Quaternion(1, 0, 0, 0);
      initialSegQuats[i][j] = new Quaternion(1, 0, 0, 0);
      gotInitialVals[i][j] = false;
      spinFreqs[i][j] = 0;
    }
  }
  
  setupLeg();
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
        gotInitialVals[left][upper] = true;
      }
      
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

      if (isSpin != 0) {
        // TODO: handle spin
        spinFreqs[left][upper] = ByteBuffer.wrap(buf, 5, 4).order(ByteOrder.LITTLE_ENDIAN).getFloat();
        return true;
      }

      spinFreqs[left][upper] = 0;

      int numLRAs = 6;
      if (upper == 1) {
        numLRAs = 8;
      }
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

  if(tcpClient.available() > 10 * PACKET_SIZE)
  {
    print("Clearing\n");
    tcpClient.clear();
  }

  return Buffer.get();
}

void draw() {
  while(readPacket())
    packets++;
    
  background(0);

  textSize(14);
  text(packets, 40, 40);

  stroke(255, 0, 255);
  
  strokeWeight(3);
  stroke(255);
  line(450, 0, 450, 540);
  noStroke();
  
  int textXCoord = 630;
  int textYCoord = 70;
  if (recording) {
    stroke(255, 0, 0);
    fill(255, 0, 0);
    ellipse(textXCoord, textYCoord, 10, 10);
    text("Recording", textXCoord+15, textYCoord+6);
  } else if (exercising) {
    stroke(0, 255, 0);
    fill(0, 255, 0);
    ellipse(textXCoord, textYCoord, 10, 10);
    text("Exercising", textXCoord+15, textYCoord+6);
  }
  
  while(readPacket())
    packets++;

  int xCoord = 675;
  int yCoord = 0;
  if (lastLraMsg[0][0] != null) {
    int numLRAs = lastLraMsg[0][0].numLRAs;
    fill(0);
    stroke(255);
    strokeWeight(3);
    ellipse(xCoord, yCoord+200, 150, 150);
    noStroke();
  
    for (int i = 0; i < numLRAs; i++) {
      float angle = 2*PI/numLRAs;
      fill(2*lastLraMsg[0][0].intensities[i]);
      ellipse(xCoord + 75*cos(i*angle), yCoord+200 + 75*sin(i*angle), 20, 20);
    }
    
    noFill();
    stroke(255);
    if (spinFreqs[0][0] < -0.01) {
      arc(xCoord, yCoord+200, 170, 170, 0, HALF_PI);
      line(xCoord, yCoord+200+170, 10+xCoord, 10+yCoord+200+85);
    }
    if (spinFreqs[0][0] > 0.01) {
      arc(xCoord, yCoord+200, 170, 170, -HALF_PI, 0);
      line(xCoord, yCoord+200-170, 10+xCoord, 10+yCoord+200-85);
    }
    fill(255);
    textSize(20);
    text("Shin", xCoord-23, yCoord+205);
  }
  if (lastLraMsg[0][1] != null) {
    int numLRAs = lastLraMsg[0][1].numLRAs;
    fill(0);
    stroke(255);
    strokeWeight(3);
    ellipse(xCoord, yCoord+400, 150, 150);
    noStroke();
  
    for (int i = 0; i < numLRAs; i++) {
      float angle = 2*PI/numLRAs;
      fill(2*lastLraMsg[0][1].intensities[i]);
      ellipse(xCoord + 75*cos(i*angle), yCoord+400 + 75*sin(i*angle), 20, 20);
    }
    
    noFill();
    stroke(255);
    if (spinFreqs[0][1] < -0.01) {
      arc(xCoord, yCoord+400, 170, 170, 0, HALF_PI);
      line(xCoord, yCoord+400+170, 10+xCoord, 10+yCoord+400+85);
    }
    if (spinFreqs[0][1] > 0.01) {
      arc(xCoord, yCoord+400, 170, 170, -HALF_PI, 0);
      line(xCoord, yCoord+400-170, 10+xCoord, 10+yCoord+400-85);
    }
    fill(255);
    textSize(20);
    text("Thigh", xCoord-25, yCoord+405);
    
  }

  while(readPacket())
    packets++;

  //if(gotInitialVals[0][0] && gotInitialVals[0][1])
  if(gotInitialVals[0][0])
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
    
    if(rFoot_LowLeg.x * rKnee_UpperLeg.x > lowerLegLength * upperLegLength / 10)
    {
      rotQt = new Quaternion((float)Math.cos(Math.PI/2), 0, 0, (float)Math.sin(Math.PI/2)).mult(rotQt);
      print("Switching view orientation");
    }
    
    int plotXOffset = 140;
    int plotYOffset = 150;
    
    // XY Plot
    stroke(255);
    text("Top", plotXOffset+100, plotYOffset-40);
    text(" x", plotXOffset+70, plotYOffset);
    text(" y", plotXOffset, plotYOffset-70);
    stroke(255,0,0);
    line(plotXOffset, plotYOffset, plotXOffset+70, plotYOffset);
    stroke(0,0,255);
    line(plotXOffset, plotYOffset, plotXOffset, plotYOffset-70);
    stroke(255,0,255);
    line(plotXOffset, plotYOffset, -rFoot_LowLeg.x + plotXOffset, -rFoot_LowLeg.y + plotYOffset);
    line(-rFoot_LowLeg.x + plotXOffset, -rFoot_LowLeg.y + plotYOffset,
         -rFoot_LowLeg.x + plotXOffset + -rKnee_UpperLeg.x, -rFoot_LowLeg.y + plotYOffset + -rKnee_UpperLeg.y
    );

    // XZ Plot
    plotYOffset += 170;
    stroke(255);
    text("Side", plotXOffset+100, plotYOffset-40);
    text(" x", plotXOffset+70, plotYOffset);
    text(" z", plotXOffset, plotYOffset-70);
    stroke(255,0,0);
    line(plotXOffset, plotYOffset, plotXOffset+70, plotYOffset);
    stroke(0,255,0);
    line(plotXOffset, plotYOffset, plotXOffset, plotYOffset-70);
    stroke(255,0,255);
    line(plotXOffset, plotYOffset, -rFoot_LowLeg.x + plotXOffset, -rFoot_LowLeg.z + plotYOffset);
    line(-rFoot_LowLeg.x + plotXOffset, -rFoot_LowLeg.z + plotYOffset,
         -rFoot_LowLeg.x + plotXOffset + -rKnee_UpperLeg.x, -rFoot_LowLeg.z + plotYOffset + -rKnee_UpperLeg.z
    );
    
    // YZ Plot
    plotYOffset += 170;
    stroke(255);
    text("Front", plotXOffset+100, plotYOffset-40);
    text(" y", plotXOffset+70, plotYOffset);
    text(" z", plotXOffset, plotYOffset-70);
    stroke(0,0,255);
    line(plotXOffset, plotYOffset, plotXOffset+70, plotYOffset);
    stroke(0,255,0);
    line(plotXOffset, plotYOffset, plotXOffset, plotYOffset-70);
    stroke(255,0,255);
    line(plotXOffset, plotYOffset, -rFoot_LowLeg.y + plotXOffset, -rFoot_LowLeg.z + plotYOffset);
    line(-rFoot_LowLeg.y + plotXOffset, -rFoot_LowLeg.z + plotYOffset,
         -rFoot_LowLeg.y + plotXOffset + -rKnee_UpperLeg.y, -rFoot_LowLeg.z + plotYOffset + -rKnee_UpperLeg.z
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
    } else {
      guiControlMsg[POS_DATA+1] = byte(STOP_RECORDING);
      recording = false;
    }
    tcpClient.write(guiControlMsg);
  }

  if (key == 'e')
  {
    if (!exercising) {
      guiControlMsg[POS_DATA+1] = byte(START_EXERCISE);
      exercising = true;
    } else {
      guiControlMsg[POS_DATA+1] = byte(STOP_EXERCISE);
      exercising = false;
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
