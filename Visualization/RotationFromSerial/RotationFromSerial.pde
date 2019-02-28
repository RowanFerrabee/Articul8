import processing.net.*; 
import processing.serial.*;

PImage textures[];
String texNames[] = {"front.png", "back.png", "bottom.png", "top.png", "right.png", "left.png"};

Client tcpClient;
int socketPort = 5432;

float rotx = PI/4;
float roty = PI/4;
float rotz = 0;
int failed = 0;
int packets = 0;

int start_time = millis();
boolean recording = false;
boolean exercising = false;

LRAMsg lastLraMsg;
IMUMsg lastImuMsg;

Quaternion to_global = new Quaternion(0,1,0,0);

void setup() {
  size(900, 540, P3D);
  textures = new PImage[6];
  for (int i = 0; i < 6; i++) {
    textures[i] = loadImage(texNames[i]);
  }
  textureMode(NORMAL);
  fill(255);
  stroke(color(44,48,32));

  tcpClient = new Client(this, "127.0.0.1", socketPort);
  
  lastLraMsg = new LRAMsg();
  lastImuMsg = new IMUMsg();
}

void readPacket() {
  byte[] rxBuffer = new byte[PACKET_SIZE];

  if (tcpClient.available() >= PACKET_SIZE)
  {
    int bytesRead = tcpClient.readBytes(rxBuffer);
    
    packets++;
    if (isValidPacket(rxBuffer, bytesRead))
    {
      if (rxBuffer[POS_DATA] == IMU_DATA_MSG)
      {
        lastImuMsg = IMUMsg.fromBytes(rxBuffer);
      }
      else if (rxBuffer[POS_DATA] == LRA_CONTROL_MSG)
      {
        lastLraMsg = LRAMsg.fromBytes(rxBuffer);
      }
    }
    else
    {
      failed++;
      print("Packet Failure: ");
      print(failed);
      print(" / ");
      println(packets);
    }
  }
  
  tcpClient.clear();
}
 //<>//
void draw() {
  readPacket();
  
  background(0);
  fill(255);
  text(packets, 40, 40);
  
  strokeWeight(3);
  stroke(255);
  line(450, 0, 450, 540);
  
  noStroke();
  
  if (recording) {
    fill(255, 0, 0);
    ellipse(470,20,10,10);
  }
  
  if (exercising) {
    fill(0, 255, 0);
    ellipse(490,20,10,10);
  }

  noStroke();
  
  if (lastImuMsg.validQuat) {
    translate(width/4.0, height/2.0, -100);
    Quaternion quat = to_global.mult(lastImuMsg.quat);
    rotate(quat.getAngle(), quat.getAxisX(), quat.getAxisY(), quat.getAxisZ());
    scale(90);
    TexturedCube(textures);
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
    }
    else {
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
    }
    else {
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
    to_global = lastImuMsg.quat.getInverse();
    start_time = millis();
    packets = 0;
  }
}

void TexturedCube(PImage[] texs) {
  // +Z "front" face
  beginShape(QUADS);
  texture(texs[0]);
  vertex(-1, -1,  1, 0, 0);
  vertex( 1, -1,  1, 1, 0);
  vertex( 1,  1,  1, 1, 1);
  vertex(-1,  1,  1, 0, 1);
  endShape();

  // -Z "back" face
  beginShape(QUADS);
  texture(texs[1]);
  vertex( 1, -1, -1, 0, 0);
  vertex(-1, -1, -1, 1, 0);
  vertex(-1,  1, -1, 1, 1);
  vertex( 1,  1, -1, 0, 1);
  endShape();

  // +Y "bottom" face
  beginShape(QUADS);
  texture(texs[2]);
  vertex(-1,  1,  1, 0, 0);
  vertex( 1,  1,  1, 1, 0);
  vertex( 1,  1, -1, 1, 1);
  vertex(-1,  1, -1, 0, 1);
  endShape();

  // -Y "top" face
  beginShape(QUADS);
  texture(texs[3]);
  vertex(-1, -1, -1, 0, 0);
  vertex( 1, -1, -1, 1, 0);
  vertex( 1, -1,  1, 1, 1);
  vertex(-1, -1,  1, 0, 1);
  endShape();

  // +X "right" face
  beginShape(QUADS);
  texture(texs[4]);
  vertex( 1, -1,  1, 0, 0);
  vertex( 1, -1, -1, 1, 0);
  vertex( 1,  1, -1, 1, 1);
  vertex( 1,  1,  1, 0, 1);
  endShape();
  
  // -X "left" face
  beginShape(QUADS);
  texture(texs[5]);
  vertex(-1, -1, -1, 0, 0);
  vertex(-1, -1,  1, 1, 0);
  vertex(-1,  1,  1, 1, 1);
  vertex(-1,  1, -1, 0, 1);
  endShape();
}
