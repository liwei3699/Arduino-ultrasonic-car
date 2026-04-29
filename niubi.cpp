/*
  Arduino Nano DIY 小车（高响应版）
  - 扫描频率提高：测距间隔20ms，舵机等待300ms
  - 电机速度：150
  - 后退800ms，转弯后前进1500ms
  - 极窄/直行监测阈值保持不变
*/

#include <L298N.h>
#include <Servo.h>

// 超声波
int trigPin = 13;
int echoPin = 11;
long duration, cm;

// L298N
const unsigned int IN1 = 5;
const unsigned int IN2 = 4;
const unsigned int EN  = 6;

// 舵机
Servo LR, CSB;

int SR04 = 0;
L298N motor(EN, IN1, IN2);

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  motor.setSpeed(0);
  LR.attach(9);
  CSB.attach(11);
}

void loop() {
  // 归位
  LR.write(70);
  CSB.write(85);

  // 测前方
  SR04 = JULI();
  Serial.print("前");
  Serial.println(SR04);

  if (SR04 < 60) {
    motor.stop();
    motor.setSpeed(0);

    // 左扫描（舵机等待缩短）
    CSB.write(150);
    delay(300);           // 原500
    int L = JULI();
    Serial.print("L");
    Serial.println(L);
    delay(300);           // 原500，给串口缓冲

    // 右扫描
    CSB.write(0);
    delay(300);
    int R = JULI();
    Serial.print("R");
    Serial.println(R);
    delay(300);

    CSB.write(85);

    // 极窄空间
    if (L < 25 || R < 25) {
      LR.write(70);
      delay(300);
      motor.setSpeed(255);      // 速度→150
      motor.backward();
      delay(800);
      motor.stop();
      motor.setSpeed(0);

      if (L > R) LR.write(20);
      else       LR.write(140);
      delay(600);

      motor.setSpeed(255);
      motor.forward();
      delay(1500);
      motor.stop();
      motor.setSpeed(0);
    }
    // 左右差<5，直行监测
    else if (abs(L - R) < 10) {
      LR.write(70);
      delay(300);
      motor.setSpeed(255);
      motor.forward();

      unsigned long start = millis();
      bool danger = false;
      while (millis() - start < 2000) {
        int front = JULI();
        if (front < 40) { danger = true; break; }
        delay(100);
      }
      motor.stop();
      motor.setSpeed(0);
      if (danger) {
        motor.setSpeed(255);
        motor.backward();
        delay(800);
        motor.stop();
        motor.setSpeed(0);
      }
    }
    // 正常避障左
    else if (L > R) {
      LR.write(20);
      delay(600);
      motor.setSpeed(255);
      motor.forward();
      delay(1500);
      motor.stop();
      motor.setSpeed(0);
    }
    // 正常避障右
    else {
      LR.write(140);
      delay(600);
      motor.setSpeed(255);
      motor.forward();
      delay(1500);
      motor.stop();
      motor.setSpeed(0);
    }
  }
  // 前方安全直行
  else {
    motor.setSpeed(255);
    motor.forward();
  }
}

void printSomeInfo() {
  Serial.print("Mv:");
  Serial.print(motor.isMoving());
  Serial.print(" Sp:");
  Serial.println(motor.getSpeed());
}

// 高速测距函数（间隔20ms）
int JULI() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  cm = (duration / 2) / 29.1;

  delay(20);   // 高频扫描间隔，可按需调整为10~30
  return cm;
}