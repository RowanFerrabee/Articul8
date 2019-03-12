import processing.net.*;  //<>//
import processing.serial.*;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import shapes3d.*;
import shapes3d.animation.*;
import shapes3d.utils.*;

Ellipsoid upperLeg;
int upperLegLength = 85;
int upperLegRadius = 25;
int lowerLegLength = 60;
int lowerLegRadius = 15;
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
  fill(255);
  stroke(color(44, 48, 32)); //<>//

  tcpClient = new Client(this, "127.0.0.1", socketPort);
  lastLraMsg = new LRAMsg[N_SIDES][BANDS_PER_SIDE];
  segQuats = new Quaternion[N_SIDES][BANDS_PER_SIDE];
  initialSegQuats = new Quaternion[N_SIDES][BANDS_PER_SIDE];
  gotInitialVals = new boolean[N_SIDES][BANDS_PER_SIDE];
  
  for(int i = 0; i < N_SIDES; i++)
  {
    for(int j = 0; j < BANDS_PER_SIDE; j++)
    {
      lastLraMsg[i][j] = new LRAMsg(6);
      segQuats[i][j] = new Quaternion(1, 0, 0, 0);
      initialSegQuats[i][j] = new Quaternion(1, 0, 0, 0);
      gotInitialVals[i][j] = false;
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
    
    //print(nb, " ", rxBufLen, " ", rxBufLen1, "\n");
    
    //int offset = 0;
    //while(offset < nb)
    //{
    //  if(rxBufLen > 0 && rxBufLen < PACKET_SIZE)
    //  {
    //    int space = PACKET_SIZE - rxBufLen;
    //    int nCopy = min(space, nb-offset);
    //    print("Copying 0\n");
    //    System.arraycopy(tmp, offset, rxbuf, rxBufLen, nCopy);
    //    rxBufLen += nCopy;
    //    offset += nCopy;
    //  }
    //  else if(rxBufLen1 > 0 && rxBufLen1 < PACKET_SIZE)
    //  {
    //    int space = PACKET_SIZE - rxBufLen1;
    //    int nCopy = min(space, nb-offset);
    //    print("Copying 1\n");
    //    System.arraycopy(tmp, offset, rxbuf1, rxBufLen1, nCopy);
    //    rxBufLen1 += nCopy;
    //    offset += nCopy;
    //  }
    //  else
    //  {
    //    int o = 0;
    //    while(o < nb && tmp[offset+o] != (byte)253)
    //      o++;
        
    //    int nCopy = min(nb - o, PACKET_SIZE);
    //    if(nCopy > 0)
    //    {
    //      if(rxBufLen == PACKET_SIZE && rxBufLen1 == 0)
    //      {
    //        System.arraycopy(tmp, offset+o, rxbuf1, 0, nCopy);
    //        print("Copying -- 1\n");
    //        rxBufLen1 = nCopy;
    //        offset += o + nCopy;
    //      }
    //      else
    //      {
    //        System.arraycopy(tmp, o, rxbuf, 0, nCopy);
    //        //print("Copying ", nCopy, " -- 0\n");
    //        rxBufLen = nCopy;    
    //        offset += o + nCopy;
    //      }
    //    }
  
    //  }
    
    //}
    
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
      
      if (gotInitialVals[left][upper] == false) {
        
        if(upper == 1 && gotInitialVals[left][0] == false)
        {}
        else
        {
          gotInitialVals[left][upper] = true;
          initialSegQuats[left][upper] = new Quaternion(w, x, y, z);
          print("hitit\n");
        }
        
        if(gotInitialVals[left][upper])
        {
          PVector v = new PVector(1, 0, 0);
          if(upper == 0)
            rotQs = turnToX(initialSegQuats[left][0].rotateVec(v));
          else
          {
            v = initialSegQuats[left][1].rotateVec(v);
            rotQt = turnToX(rotQs.rotateVec(v));
            //rotQt = new Quaternion( (float)Math.cos(Math.PI/2), 0, 0, (float)Math.sin(Math.PI/2) ).mult(rotQt);
          }
        }
      }
      segQuats[left][upper] = new Quaternion(w, x, y, z);
      //print(Integer.toString(left) + ":" + Integer.toString(upper) + "\n");
      
      //printQuat(segQuats[left][upper]);
      return true;
    }
    else if (buf[1] == GUI_LRA_MSG)
    {
      int left = buf[2];
      int upper = buf[3];
      int isSpin = buf[4];

      if (isSpin != 0) {
        // TODO: handle spin
        return true;
      }

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
  if(readPacket())
    packets++;
    
  background(0);

  text(packets, 40, 40);

  camera(0.0, -500.0, -60.0,
          0, 0, 0,
          0, 0, 1);

  lights();
  directionalLight(255, 255, 255, -1, 0, 0);

  stroke(255, 0, 0);
  line(0, 0, 0, 50, 0, 0);  // red x
  stroke(0, 0, 255);
  line(0, 0, 0, 0, 50, 0);  // blue y
  stroke(0, 255, 0);
  line(0, 0, 0, 0, 0, -50); // green z

  stroke(255, 0, 255);
  
  //strokeWeight(3);
  //stroke(255);
  //line(450, 0, 450, 540);
  //noStroke();

  pushMatrix();
  rotateY((float)Math.PI/2);
  rotateX((float)Math.PI/2);
  rotateZ((float)Math.PI/2);
  
  int xCoord = 200;
  int yCoord = -100;
  if (recording) {
    stroke(255, 0, 0);
    fill(255, 0, 0);
    ellipse(xCoord, yCoord, 10, 10);
    text("Recording", xCoord, yCoord+20);
  } else if (exercising) {
    stroke(0, 255, 0);
    fill(0, 255, 0);
    ellipse(xCoord, yCoord, 10, 10);
    text("Exercising", xCoord, yCoord+20);
  }

  if (lastLraMsg[0][0] != null) {
    int numLRAs = lastLraMsg[0][0].numLRAs;
    fill(0);
    stroke(255);
    strokeWeight(3);
    ellipse(xCoord - 350, yCoord+200, 150, 150);
    noStroke();
  
    for (int i = 0; i < numLRAs; i++) {
      float angle = 2*PI/numLRAs;
      fill(2*lastLraMsg[0][0].intensities[i]);
      ellipse(xCoord - 350 + 75*cos(i*angle), yCoord+200 + 75*sin(i*angle), 20, 20);
    }
    fill(255);
    textSize(20);
    text("Shin", xCoord-367, yCoord+205);
  }
  if (lastLraMsg[0][1] != null) {
    int numLRAs = lastLraMsg[0][1].numLRAs;
    fill(0);
    stroke(255);
    strokeWeight(3);
    ellipse(xCoord, yCoord+200, 150, 150);
    noStroke();
  
    for (int i = 0; i < numLRAs; i++) {
      float angle = 2*PI/numLRAs;
      fill(2*lastLraMsg[0][1].intensities[i]);
      ellipse(xCoord + 75*cos(i*angle), yCoord+200 + 75*sin(i*angle), 20, 20);
    }
    fill(255);
    textSize(20);
    text("Thigh", xCoord-25, yCoord+205);
    
  }
  
  popMatrix();
  //foot.draw();
  //translate(0, footLength/2, 0);
  
  if(gotInitialVals[0][0] && gotInitialVals[0][1])
  {
    stroke(255);
    PVector rFoot_LowLeg = new PVector(lowerLegLength, 0, 0);
    PVector rKnee_UpperLeg = new PVector(upperLegLength, 0, 0);
    
    Quaternion lowerQuat = rotQs.mult(segQuats[0][0]);
    Quaternion upperQuat = rotQt.mult(rotQs.mult(segQuats[0][1]));
    
    rFoot_LowLeg = lowerQuat.rotateVec(rFoot_LowLeg);
    rKnee_UpperLeg = upperQuat.rotateVec(rKnee_UpperLeg);
    
    if(rFoot_LowLeg.x * rKnee_UpperLeg.x > lowerLegLength * upperLegLength / 10)
    {
      rotQt = new Quaternion((float)Math.cos(Math.PI/2), 0, 0, (float)Math.sin(Math.PI/2)).mult(rotQt);
      print("ASDF");
    }
    
    line(0, 0, 0,
        rFoot_LowLeg.x, rFoot_LowLeg.y, -rFoot_LowLeg.z);
        
    line(rFoot_LowLeg.x, rFoot_LowLeg.y, -rFoot_LowLeg.z,
         rFoot_LowLeg.x + rKnee_UpperLeg.x, rFoot_LowLeg.y + rKnee_UpperLeg.y, -rFoot_LowLeg.z  - rKnee_UpperLeg.z
    );
    
  }
  
  
      
  
  //print(Float.toString(rFoot_LowLeg.x) + " " + Float.toString(rFoot_LowLeg.y) + " " + Float.toString(rFoot_LowLeg.z) + "\n");
  ////translate(rFoot_LowLeg.x,rFoot_LowLeg.y,rFoot_LowLeg.z); 
  //translate(0,0,rFoot_LowLeg.z); 
  ////translate(rFoot_LowLeg.x,rFoot_LowLeg.y);
  //rotate(lowerQuat.getAngle(), lowerQuat.getAxisX(), lowerQuat.getAxisY(), lowerQuat.getAxisZ());
  //lowerLeg.draw();
  //rotate(-lowerQuat.getAngle(), lowerQuat.getAxisX(), lowerQuat.getAxisY(), lowerQuat.getAxisZ());
  //translate(rFoot_LowLeg.x,rFoot_LowLeg.y,rFoot_LowLeg.z); 

  //translate(rKnee_UpperLeg.x,rKnee_UpperLeg.y,rKnee_UpperLeg.z);
  //rotate(upperQuat.getAngle(), upperQuat.getAxisX(), upperQuat.getAxisY(), upperQuat.getAxisZ());
  //upperLeg.draw();
  //rotate(-upperQuat.getAngle(), upperQuat.getAxisX(), upperQuat.getAxisY(), upperQuat.getAxisZ());
  
  //for(int i = 0; i < BANDS_PER_SIDE; i++)
  //{
  //  Quaternion quat = segQuats[0][i];
    
  //  //Quaternion quat = to_global.mult(segQuats[0][i]);
  //  //PVector grav = getGravityVector(quat);
  //  //float mag = 500;
  
  //  pushMatrix();
  //  translate(width/4.0, height/4.0*i, -100);
  //  rotate(quat.getAngle(), quat.getAxisX(), quat.getAxisY(), quat.getAxisZ());
  //  scale(90);
  //  TexturedCube(textures);
  //  popMatrix();
  
  //  //pushMatrix();
  //  //translate(width/4.0, height/2.0, -100);
  //  //stroke(255);
  //  //strokeWeight(3);
  //  //noFill();
  //  //beginShape(LINES);
  //  //vertex(0, 0, 0);
  //  //vertex(mag * grav.x, mag * grav.y, mag * grav.z);
  //  //endShape();
  //  //popMatrix();
    
    
  //}
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

  if (key == ' ')
  {
    to_global = segQuats[0][0].getInverse();
    start_time = millis();
    packets = 0;
  }
}

void TexturedCube(PImage[] texs) {
  // +Z "front" face
  beginShape(QUADS);
  //texture(texs[0]);
  fill(255, 255, 0);
  vertex(-1, -1, 1);
  vertex( 1, -1, 1);
  vertex( 1, 1, 1);
  vertex(-1, 1, 1);
  endShape();

  // -Z "back" face
  beginShape(QUADS);
  fill(255, 0, 255);
  vertex( 1, -1, -1);
  vertex(-1, -1, -1);
  vertex(-1, 1, -1);
  vertex( 1, 1, -1);
  endShape();

  // +Y "bottom" face
  beginShape(QUADS);
  fill(0, 0, 255);
  vertex(-1, 1, 1);
  vertex( 1, 1, 1);
  vertex( 1, 1, -1);
  vertex(-1, 1, -1);
  endShape();

  // -Y "top" face
  beginShape(QUADS);
  fill(255);
  vertex(-1, -1, -1);
  vertex( 1, -1, -1);
  vertex( 1, -1, 1);
  vertex(-1, -1, 1);
  endShape();

  // +X "right" face
  beginShape(QUADS);
  fill(0, 255, 255);
  vertex( 1, -1, 1);
  vertex( 1, -1, -1);
  vertex( 1, 1, -1);
  vertex( 1, 1, 1);
  endShape();

  // -X "left" face
  beginShape(QUADS);
  fill(255, 0, 0);
  vertex(-1, -1, -1);
  vertex(-1, -1, 1);
  vertex(-1, 1, 1);
  vertex(-1, 1, -1);
  endShape();
}
