
static class Quaternion {
  Quaternion(float w_, float x_, float y_, float z_) {
    w = w_;
    x = x_;
    y = y_;
    z = z_;
    angle = acos(w)*2;
  }
  
  Quaternion mult(Quaternion q) {
    float w_ = w*q.w - x*q.x - y*q.y - z*q.z;  // new w
    float x_ = w*q.x + x*q.w + y*q.z - z*q.y;  // new x
    float y_ = w*q.y - x*q.z + y*q.w + z*q.x;  // new y
    float z_ = w*q.z + x*q.y - y*q.x + z*q.w; // new z
    return new Quaternion(w_, x_, y_, z_);
  }
  
  Quaternion getInverse() {
    float m = getMagnitude();
    return new Quaternion(w/(m*m), -x/(m*m), -y/(m*m), -z/(m*m));
  }

  Quaternion getConjugate() {
      return new Quaternion(w, -x, -y, -z);
  }
  
  float getMagnitude() {
      return sqrt(w*w + x*x + y*y + z*z);
  }
  
  void normalize() {
      float m = getMagnitude();
      w /= m;
      x /= m;
      y /= m;
      z /= m;
  }
  
  Quaternion getNormalized() {
      Quaternion r = new Quaternion(w, x, y, z);
      r.normalize();
      return r;
  }
  
  float getAngle() {
    return angle;
  }
  
  float getAxisX() {
    if (angle == 0) {
      return default_x;
    }
    return -x/sin(angle/2);
  }
  
  float getAxisY() {
    if (angle == 0) {
      return default_y;
    }
    return y/sin(angle/2);
  }
  
  float getAxisZ() {
    if (angle == 0) {
      return default_z;
    }
    return -z/sin(angle/2);
  }
  
  float w, x, y, z, angle;
  float default_x = 1.0;
  float default_y = 0.0;
  float default_z = 0.0;
};
