#include <BasicLinearAlgebra.h>
#include <math.h>
#include "Sensor.h"
VL53L0X sensor, sensor2;

//State, Output Vectors
BLA::Matrix<3> q = {0, 203, 305}; //{0.1,250,375};
BLA::Matrix<3> z = {0, 406, 610}; //{0,350,200};
BLA::Matrix<3> q_est;
BLA::Matrix<3> z_est;

//Noise Vectors
BLA::Matrix<3> w;

//Kalman Filter Matrices
BLA::Matrix<3, 3> F;
BLA::Matrix<3, 3> S;
BLA::Matrix<3, 3> Q;
BLA::Matrix<3, 3> P;
BLA::Matrix<3, 3> K;
BLA::Matrix<3, 3> R;
BLA::Matrix<3, 3> H;

BLA::Matrix<3, 3> I3 = {1, 0, 0, 0, 1, 0, 0, 0, 1};

const float L = 750;
const float W = 500;
const float sigmaR = .000115;//m/s
const float sigmaL = .0000538;//m/s
const float sumVar = sigmaR * sigmaR + sigmaL * sigmaL;
const float diffVar = sigmaR * sigmaR - sigmaL * sigmaL;
const float sigmaAngle = 15.3;//deg
const float sigmaFLaser =  3.91;//mm
const float sigmaSLaser =  5.68;//mm
const float b = 94; //mm
const int FRONT = 0;
const int SIDE = 1;
float vL, vR, vT, wAng;

//Sensor Readings, temporary
float dF, dL, hMag;

// Maze: LB, LT, RT, RBottom;
const float px[4] = {0, 0, W, W};
const float py[4] = {0, L, L, 0};
int wall_f = - 1;
int wall_s = -1;

void update_H()
{
  float theta = q_est(0);
  if (theta == 0 || theta == 90 || theta == 270  || theta == 180) theta += .1;
  float x = q_est(1);
  float y = q_est(2);
  // Determine walls:
  wall_f = det_wall(FRONT);
  wall_s = det_wall(SIDE);
<<<<<<< HEAD
  // 
  float theta = q(0);

=======
  //
>>>>>>> 7b6a2122185d89c287a7276a4f4143458ff175aa
  if (wall_f == 0 && wall_s == 2)
  {
    H  << 1, 0, 0,
    DEG_TO_RAD*(L - y)* sin(theta * DEG_TO_RAD) / pow(cos(theta * DEG_TO_RAD), 2), 0, -1 / (cos(theta * DEG_TO_RAD)),
    DEG_TO_RAD*(-y)* cos(theta * DEG_TO_RAD) / pow(sin(theta * DEG_TO_RAD), 2), 0, 1 / (sin(theta));
  }
  else if (wall_f == 0 && wall_s == 1) {
    H  << 1, 0, 0,
    DEG_TO_RAD*(L - y)* sin(theta * DEG_TO_RAD) / pow(cos(theta * DEG_TO_RAD), 2), 0, -1 / (cos(theta * DEG_TO_RAD)),
    DEG_TO_RAD*(W - x)*sin(theta * DEG_TO_RAD) / pow(cos(theta * DEG_TO_RAD), 2), -1 / (cos(theta * DEG_TO_RAD)), 0;
  }
  else if (wall_f == 0 && wall_s == 0) {
    H  << 1, 0, 0,
    DEG_TO_RAD*(L - y)* sin(theta * DEG_TO_RAD) / pow(cos(theta * DEG_TO_RAD), 2), 0, -1 / (cos(theta * DEG_TO_RAD)),
    DEG_TO_RAD*(y - L)*cos(theta * DEG_TO_RAD) / pow(sin(theta * DEG_TO_RAD), 2), 0, 1 / sin(theta * DEG_TO_RAD);
  }
  else if (wall_f == 1 && wall_s == 3) {
    H  << 1, 0, 0,
    DEG_TO_RAD*(W - x)*tan(DEG_TO_RAD * (theta - 90)) / cos(DEG_TO_RAD * (theta - 90)), -1 / (cos(DEG_TO_RAD * (theta - 90))), 0,
    -DEG_TO_RAD*x / tan(DEG_TO_RAD * (theta - 90)) / sin(DEG_TO_RAD * (theta - 90)), 1 / sin(DEG_TO_RAD * (theta - 90)), 0;
  }
  else if (wall_f == 1 && wall_s == 2) {
    H  << 1, 0, 0,
    DEG_TO_RAD*(W - x)*tan(DEG_TO_RAD * (theta - 90)) / cos(DEG_TO_RAD * (theta - 90)), -1 / (cos(DEG_TO_RAD * (theta - 90))), 0,
    DEG_TO_RAD*y*tan(DEG_TO_RAD * (theta - 90)) / cos(DEG_TO_RAD * (theta - 90)), 0, 1 / cos(DEG_TO_RAD * (theta - 90));
  }
  else if (wall_f == 1 && wall_s == 1) {
    H  << 1, 0, 0,
    DEG_TO_RAD*(W - x)*tan(DEG_TO_RAD * (theta - 90)) / cos(DEG_TO_RAD * (theta - 90)), -1 / (cos(DEG_TO_RAD * (theta - 90))), 0,
    -DEG_TO_RAD*(x - W) / sin(DEG_TO_RAD * (theta - 90)) / tan(DEG_TO_RAD * (theta - 90)), 1 / (sin(DEG_TO_RAD * (theta - 90))), 0;
  }
  else if (wall_f == 2 && wall_s == 0) {
    H  << 1, 0, 0,
    DEG_TO_RAD*y*tan(DEG_TO_RAD * (theta - 180)) / cos(DEG_TO_RAD * (theta - 180)), 0, 1 / cos(DEG_TO_RAD * (theta - 180)),
    -DEG_TO_RAD*(L - y) / tan(DEG_TO_RAD * (theta - 180)) / sin(DEG_TO_RAD * (theta - 180)), 0, -1 / sin(DEG_TO_RAD * (theta - 180));
  }
  else if (wall_f == 2 && wall_s == 3) {
    H  << 1, 0, 0,
    DEG_TO_RAD*y*tan(DEG_TO_RAD * (theta - 180)) / cos(DEG_TO_RAD * (theta - 180)), 0, 1 / cos(DEG_TO_RAD * (theta - 180)),
    DEG_TO_RAD*x*tan(DEG_TO_RAD * (theta - 180)) / cos(DEG_TO_RAD * (theta - 180)), 1 / cos(DEG_TO_RAD * (theta - 180)), 0;
  }
  else if (wall_f == 2 && wall_s == 2) {
    H  << 1, 0, 0,
    DEG_TO_RAD*y*tan(DEG_TO_RAD * (theta - 180)) / cos(DEG_TO_RAD * (theta - 180)), 0, 1 / cos(DEG_TO_RAD * (theta - 180)),
    DEG_TO_RAD*y / tan(DEG_TO_RAD * (theta - 180)) / cos(DEG_TO_RAD * (theta - 180)), 0, -1 / sin(DEG_TO_RAD * (theta - 180));
  }
  else if (wall_f == 3 && wall_s == 1) {
    H  << 1, 0, 0,
    DEG_TO_RAD*x*tan(DEG_TO_RAD * (theta - 270.0)) / cos(DEG_TO_RAD * (theta - 270.0)), 1 / cos(DEG_TO_RAD * (theta - 270.0)), 0,
    DEG_TO_RAD*(W - x) / tan(DEG_TO_RAD * (theta - 270.0)) / sin(DEG_TO_RAD * (theta - 270.0)), -1 / sin(DEG_TO_RAD * (theta - 270.0)), 0;
  }
  else if (wall_f == 3 && wall_s == 0) {
    H  << 1, 0, 0,
    DEG_TO_RAD*x*tan(DEG_TO_RAD * (theta - 270.0)) / cos(DEG_TO_RAD * (theta - 270.0)), 1 / cos(DEG_TO_RAD * (theta - 270.0)), 0,
    DEG_TO_RAD*(L - y)*tan(DEG_TO_RAD * (theta - 270.0)) / cos(DEG_TO_RAD * (theta - 270.0)), 0, -1 / cos(DEG_TO_RAD * (theta - 270.0));
  }
  else if (wall_f == 3 && wall_s == 3) {
    H  << 1, 0, 0,
    DEG_TO_RAD*x*tan(DEG_TO_RAD * (theta - 270.0)) / cos(DEG_TO_RAD * (theta - 270.0)), 1 / cos(DEG_TO_RAD * (theta - 270.0)), 0,
    DEG_TO_RAD*x / tan(DEG_TO_RAD * (theta - 270.0)) / sin(DEG_TO_RAD * (theta - 270.0)), -1 / sin(DEG_TO_RAD * (theta - 270.0)), 0;
  }
  return;
}

// Input: given state
// Output: determine wall wall_f, wall_s
// Argument:
//   int sensorType: 0 - front sensor
//      1 - right sensor
int det_wall(int sensorType)
{
  // TODO: measure sensors from middle point of vehicle to account for displacement
  float theta = q_est(0);
  float x = q_est(1);
  float y = q_est(2);

  if (sensorType == 1)
    theta += 90;
  if (theta > 360)
    theta -= 360;

  // N = 0, E = 1, S = 3, W = 4;
  // Find distances to each corner:
  float z[4];
  // distances to corners, px's are coordinates of corners
  for (int i = 0; i < 4; i++)
  {
    z[i] = sqrt(pow((px[i] - x), 2) + pow((py[i] - y), 2));
  }
  // Angles for possible 8 triangular view:
  // theta with respect to true North
  float thetas[8];
  // starting from North line rotating clockwise, total of eight divisions
  thetas[0] = acos((L - y) / z[2]) * 180 / PI;
  thetas[1] = acos((W - x) / z[2]) * 180 / PI;
  thetas[2] = acos((W - x) / z[3]) * 180 / PI;
  thetas[3] = acos(y / z[3]) * 180 / PI;
  thetas[4] = acos(y / z[0]) * 180 / PI;
  thetas[5] = acos(x / z[0]) * 180 / PI;
  thetas[6] = acos(x / z[1]) * 180 / PI;
  thetas[7] = acos((L - y) / z[1]) * 180 / PI;

  int n_sections = 8;
  // Range should be between 0-360
  // And check which walls the robot is pointing at:
  // cumulative theta
  for (int i = 0; i < n_sections; i++) {
    if (i == 0) {
      if (theta < thetas[i]) {
        return 0;
      }
      continue;
    }
    thetas[i] = thetas[i - 1] + thetas[i];
    if (theta < thetas[i]) {
      int w = (i + 1) / 2;
      if (w == 4)
        return 0;
      return w;
    }
  }
}

void getVelocities(float pwmR, float pwmL, float& vR, float& vL, float &vT, float& wAng)
{
  vR = -140 * tanh(-0.048 * (pwmR - 91.8)); //mm/s
  vL = 139 * tanh(-0.047 * (pwmL - 92.6)); //mm/s
  vT = .5 * (vL + vR);
  wAng = 1 / b * (vR - vL);
}

void update_F(dt)
{
  F << 1, 0, 0,
  -vT * sin(q(0) * DEG_TO_RAD)*dt, 1, 0,
  vT * cos(q(0) * DEG_TO_RAD)*dt, 1, 0;
}

void update_Q()
{
  // TODO: Make sure if this function depends on dt:
  Q << pow((1 / b), 2)*sumVar, 1 / (2 * b)*cos(q_est(0)*DEG_TO_RAD)*diffVar, 1 / (2 * b)*sin(q_est(0)*DEG_TO_RAD)*diffVar,
  1 / (2 * b)*cos(q_est(0)*DEG_TO_RAD)*diffVar, pow(cos(q_est(0)*DEG_TO_RAD), 2) / 4 * sumVar, sin(q_est(0)*DEG_TO_RAD)* cos(q_est(0)*DEG_TO_RAD) / 4 * sumVar,
  1 / (2 * b)*sin(q_est(0)*DEG_TO_RAD)*diffVar, sin(q_est(0)*DEG_TO_RAD)* cos(q_est(0 * DEG_TO_RAD)) / 4 * sumVar, pow(sin(q_est(0)*DEG_TO_RAD), 2) / 4 * sumVar;
  return;
}

void aPrioriUpdate(float dt)
{
  //get q^ estimate
  q_est(0) = q_est(0) + (wAng * dt)*RAD_TO_DEG;
  q_est(1) += vT * cos(q(0) * DEG_TO_RAD) * dt;
  q_est(2) += vT * sin(q(0) * DEG_TO_RAD) * dt;

  //P update
  update_F(dt);
  update_Q();
  P = ((F * P) * (~F)) + Q;

}

void updateSensor()
{
  float gz, head, fDist, sDist;
  ReadIMU(gz, head);

  fDist = sensor.readRangeSingleMillimeters();
  if (sensor.timeoutOccurred()) {
    Serial.print(" TIMEOUT");
  }

  sDist = sensor2.readRangeSingleMillimeters();
  if (sensor2.timeoutOccurred()) {
    Serial.print(" TIMEOUT");
  }
  z << head, fDist, sDist;
}

void aPosterioriUpdate(float dt)
{
  outputEstimate(z_est, q_est);
  update_H();
  updateSensor();
  BLA::Matrix<3> innovation = z - z_est;
  S = ((H * P) * (~H)) + R; //innovation covariance
  K = (P * (~H)) * (S.Inverse()); //Kalman Gain
  q_est += (K * innovation); //A Posteriori State Estimate
  P = (I3 - (K * H)) * P; //Update Covariance Estimate

  Serial.print(z_est(0));
  Serial.print(" ");
  Serial.print(z_est(1));
  Serial.print(" ");
  Serial.print(z_est(2));
  Serial.print(" ");
  Serial.print("\n");
}

void outputEstimate(BLA::Matrix<3>& z_est, BLA::Matrix<3>& q_est)
{
  // Determine walls:
  int wall_f = det_wall(FRONT);
  int wall_s = det_wall(SIDE);
  Serial.print(wall_f); Serial.print(" ");
  Serial.print(wall_s); Serial.print("\n");
  //
  float theta = q_est(0);
  float x = q_est(1);
  float y = q_est(2);
  // Adjust theta depending on which wall facing:

  if (wall_f == 0 && wall_s == 2) {
    z_est  << q_est(0), (L - y) / cos(theta * DEG_TO_RAD),
           y / (sin(theta * DEG_TO_RAD));
  }
  else if (wall_f == 0 && wall_s == 1) {
    z_est << q_est(0), (L - y) / cos(theta * DEG_TO_RAD),
          (W - x) / (cos(theta * DEG_TO_RAD));
  }
  else if (wall_f == 0 && wall_s == 0) {
    z_est << q_est(0), (L - y) / cos(theta * DEG_TO_RAD),
          -(L - y) / (sin(theta * DEG_TO_RAD));
  }
  else if (wall_f == 1 && wall_s == 3) {
    z_est << q_est(0), (W - x) / cos((theta - 90)*DEG_TO_RAD),
          x / (sin((theta - 90)*DEG_TO_RAD));
  }
  else if (wall_f == 1 && wall_s == 2) {
    z_est << q_est(0), (W - x) / cos((theta - 90)*DEG_TO_RAD),
          y / (cos((theta - 90)*DEG_TO_RAD));
  }
  else if (wall_f == 1 && wall_s == 1) {
    z_est << q_est(0), (W - x) / cos((theta - 90)*DEG_TO_RAD),
          -(W - x) / (sin((theta - 90)*DEG_TO_RAD));
  }
  else if (wall_f == 2 && wall_s == 0) {
    z_est << q_est(0), y / cos((theta - 180)*DEG_TO_RAD),
          (L - y) / (sin((theta - 180)*DEG_TO_RAD));
  }
  else if (wall_f == 2 && wall_s == 3) {
    z_est << q_est(0), y / cos((theta - 180)*DEG_TO_RAD),
          x / (cos((theta - 180)*DEG_TO_RAD));
  }
  else if (wall_f == 2 && wall_s == 2) {
    z_est << q_est(0), y / cos((theta - 180)*DEG_TO_RAD),
          -y / (sin((theta - 180)*DEG_TO_RAD));
  }
  else if (wall_f == 3 && wall_s == 1) {
    z_est << q_est(0), x / cos((theta - 270)*DEG_TO_RAD),
          (W - x) / (sin((theta - 270)*DEG_TO_RAD));
  }
  else if (wall_f == 3 && wall_s == 0) {
    z_est << q_est(0), x / cos((theta - 270)*DEG_TO_RAD),
          (L - y) / (cos((theta - 270)*DEG_TO_RAD));
  }
  else if (wall_f == 3 && wall_s == 3) {
    z_est << q_est(0), x / cos((theta - 270)*DEG_TO_RAD),
          -x / (sin((theta - 270)*DEG_TO_RAD));
  }
  BLA::Matrix <3> offset;
  offset << 0, 25, 30;
  z_est -= offset;
  return;
}

void setup() {
  // put your setup code here, to run once:

  //Initialize P to I
  P << 1, 0, 0,
  0, 1, 0,
  0, 0, 1;

  //Need to initialize Matrices here (Q, R)
  R << pow(sigmaAngle, 2), 0, 0,
  0, pow(sigmaFLaser, 2), 0,
  0, 0, pow(sigmaSLaser, 2);

  Q << 1, 0, 0, 0, 1, 0, 0, 0, 1;

  q_est = q;
  z_est = z;

  Serial.begin (115200);

  //Setup Distance Sensors
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  digitalWrite(D7, LOW);
  digitalWrite(D8, LOW);

  delay(500);
  Wire.begin(SDA_PORT, SCL_PORT);

  digitalWrite(D3, HIGH);
  delay(150);
  Serial.println("00");

  sensor.init(true);
  Serial.println("01");
  delay(100);
  sensor.setAddress((uint8_t)22);

  digitalWrite(D4, HIGH);
  delay(150);
  sensor2.init(true);
  Serial.println("03");
  delay(100);
  sensor2.setAddress((uint8_t)25);
  Serial.println("04");

  Serial.println("addresses set");

  Serial.println ("I2C scanner. Scanning ...");
  byte count = 0;

  for (byte i = 1; i < 120; i++)
  {

    Wire.beginTransmission (i);
    if (Wire.endTransmission () == 0)
    {
      Serial.print ("Found address: ");
      Serial.print (i, DEC);
      Serial.print (" (0x");
      Serial.print (i, HEX);
      Serial.println (")");
      count++;
      delay (1);  // maybe unneeded?
    } // end of good response
  } // end of for loop
  Serial.println ("Done.");
  Serial.print ("Found ");
  Serial.print (count, DEC);

  delay(3000);

  setupIMU();
}

int t0 = 0;

void loop() {
  int tF = millis() / 1000;
  float dt = tF - t0;
  t0 = tF;
  // put your main code here, to run repeatedly:
  int pwmR = 90;
  int pwmL = 90 ;

  wall_f = det_wall(FRONT);
  wall_s = det_wall(SIDE);
  aPrioriUpdate(dt);
  aPosterioriUpdate(dt);
  //outputEstimate(z_est, q_est);
  //q_est(0) += 1;
  /*
    q_est(0) = 45;
    q_est(0) = q_est(0) > 360 ? 0 : q_est(0);
    q_est(1) += 1.5;//cos(q_est(0) * DEG_TO_RAD);
    q_est(2) += 1.5;//sin(q_est(0) * DEG_TO_RAD);
    Serial.print(" Z: ");
  */
  Serial.print(q_est(0)); Serial.print(" ");
  Serial.print(q_est(1)); Serial.print(" ");
  Serial.print(q_est(2)); Serial.print("\n");

  /*
    Serial.print("Heading: ");
    Serial.print(head);
    Serial.print(" Distance: ");
    Serial.print(sensor.readRangeSingleMillimeters());
    if (sensor.timeoutOccurred()) { Serial.print(" TIMEOUT"); }
    Serial.print(" ");
    Serial.print(sensor2.readRangeSingleMillimeters());
    if (sensor2.timeoutOccurred()) { Serial.print(" TIMEOUT"); }
  */
  getVelocities(pwmR, pwmL, vL, vR, vT, wAng);
  delay(10);
}
