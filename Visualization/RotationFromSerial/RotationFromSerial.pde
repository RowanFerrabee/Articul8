import processing.net.*; 
import processing.serial.*;

PImage textures[];
String texNames[] = {"front.png", "back.png", "bottom.png", "top.png", "right.png", "left.png"};

Serial imuPort;
String portName = "/dev/cu.usbmodem1431";

Client imuClient;
int socketPort = 5204;

float rotx = PI/4;
float roty = PI/4;
float rotz = 0;
int failed1 = 0;
int failed2 = 0;
int frames = 0;

int start_time = millis();

Quaternion meas_quat = new Quaternion(0,1,0,0);
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

  imuPort = new Serial(this, portName, 38400);
  imuClient = new Client(this, "127.0.0.1", socketPort);
  
  // Wait for MPU to set up
  delay(2000);
}

float get4bytesFloat(byte[] data, int offset) {
  String hexint=hex(data[offset+3])+hex(data[offset+2])+hex(data[offset+1])+hex(data[offset]); 
  return Float.intBitsToFloat(unhex(hexint)); 
}

void tryUpdateRotation() {
  byte[] imuBuffer = new byte[17];
  if (imuClient.available() > 0) {
    int bytesRead = imuClient.readBytes(imuBuffer);
    if (bytesRead == 16) {
      float x = get4bytesFloat(imuBuffer, 0);
      float y = get4bytesFloat(imuBuffer, 4);
      float z = get4bytesFloat(imuBuffer, 8);
      float w = get4bytesFloat(imuBuffer, 12);
      if (!Float.isNaN(x) && !Float.isNaN(y) && !Float.isNaN(z) && !Float.isNaN(w) &&
          !(x == 0 && y == 0 && z == 0)) {
        meas_quat = new Quaternion(x, y, z, w);
      } else {
        failed1++;
        print("Failed 1: ");
        print(failed1);
        print(" / ");
        println(frames);
      }
    } else {
      failed2++;
      print("Failed 2: ");
      print(failed2);
      print(" / ");
      println(frames);
    }
    imuClient.clear();
  }
}

void keyPressed() {
  to_global = meas_quat.getInverse();
  start_time = millis();
  frames = 0;
}
 //<>//
void draw() {
  tryUpdateRotation();
  background(0);
  frames++;
  text(frames, 40, 40);
  noStroke();
  translate(width/2.0, height/2.0, -100);
  Quaternion quat = to_global.mult(meas_quat);
  rotate(quat.getAngle(), quat.getAxisX(), quat.getAxisY(), quat.getAxisZ());
  scale(90);
  TexturedCube(textures);
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
