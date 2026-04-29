/*
  Arduino Nano DIY 小车（高响应版）
  - 扫描频率提高：测距间隔20ms，舵机等待300ms
  - 电机速度：150（实际代码中使用255，注释有误）
  - 后退800ms，转弯后前进1500ms
  - 极窄/直行监测阈值保持不变

  Arduino Nano DIY Obstacle Avoidance Car (High Responsiveness)
  - Increased scan frequency: 20ms between pings, 300ms servo settle
  - Motor speed: 150 (actually 255 in code, comment mismatch)
  - Reverse 800ms, forward after turn 1500ms
  - Tight space / straight-monitor thresholds unchanged
*/

#include <L298N.h>   // 电机驱动库  Motor driver library
#include <Servo.h>   // 舵机库      Servo library

// ========== 超声波引脚定义 / Ultrasonic pins ==========
int trigPin = 13;    // 触发引脚 Trig pin
int echoPin = 11;    // 回声引脚 Echo pin
long duration, cm;   // 脉宽时间、距离  Pulse duration, distance

// ========== L298N 电机驱动引脚 / Motor driver pins ==========
const unsigned int IN1 = 5;   // 方向1  Direction 1
const unsigned int IN2 = 4;   // 方向2  Direction 2
const unsigned int EN  = 6;   // 使能/PWM  Enable / PWM

// ========== 舵机对象 / Servo objects ==========
Servo LR, CSB;  // LR: 前轮转向舵机 (Left/Right steering)
                // CSB: 超声波扫描舵机 (Ultrasonic scanning servo)

int SR04 = 0;   // 存储前方距离  Stores front distance
L298N motor(EN, IN1, IN2);  // 电机对象  Motor instance

// ============================================================
// setup()：初始化函数，上电后执行一次
// Initialization function, runs once at power-up
// ============================================================
void setup() {
  Serial.begin(9600);         // 打开串口，波特率9600  Start serial at 9600 baud
  pinMode(trigPin, OUTPUT);   // 设置Trig为输出  Set Trig as output
  pinMode(echoPin, INPUT);    // 设置Echo为输入  Set Echo as input
  motor.setSpeed(0);          // 上电电机停止      Stop motor initially
  LR.attach(9);               // 转向舵机接D9     Steering servo on pin 9
  CSB.attach(11);             // 扫描舵机接D11    Scanning servo on pin 11
}

// ============================================================
// loop()：主循环，不断执行避障逻辑
// Main loop, repeatedly runs obstacle avoidance logic
// ============================================================
void loop() {
  // ----------------------------------------------------------
  // 1. 舵机归位：前轮回正，探头朝前
  //    Reset servos: wheels straight, sensor forward
  // ----------------------------------------------------------
  LR.write(70);      // 转向中位 (Mid position for steering)
  CSB.write(85);     // 探头朝前角度 (Sensor facing front)

  // ----------------------------------------------------------
  // 2. 测量正前方距离
  //    Measure front distance
  // ----------------------------------------------------------
  SR04 = JULI();     // 调用测距函数，返回值单位厘米
                     // Call distance function, returns cm
  Serial.print("前");
  Serial.println(SR04);  // 打印前方距离   Print front distance

  // ----------------------------------------------------------
  // 3. 前方障碍判断（阈值60cm）
  //    Obstacle detection (threshold 60cm)
  // ----------------------------------------------------------
  if (SR04 < 60) {
    // 停车  Stop
    motor.stop();
    motor.setSpeed(0);

    // --------------------------------------------------------
    // 4. 向左扫描：探头转到150度，读取左侧距离L
    //    Left scan: sensor to 150°, get left distance L
    // --------------------------------------------------------
    CSB.write(150);
    delay(300);          // 等待舵机稳定  Wait for servo to settle
    int L = JULI();      // 左侧距离 Left distance
    Serial.print("L");
    Serial.println(L);
    delay(300);          // 串口缓冲  Serial buffer

    // --------------------------------------------------------
    // 5. 向右扫描：探头转到0度，读取右侧距离R
    //    Right scan: sensor to 0°, get right distance R
    // --------------------------------------------------------
    CSB.write(0);
    delay(300);
    int R = JULI();      // 右侧距离 Right distance
    Serial.print("R");
    Serial.println(R);
    delay(300);

    CSB.write(85);       // 探头回正  Sensor back to front

    // --------------------------------------------------------
    // 6. 极窄空间避障：任一侧距离小于25cm
    //    Tight space avoidance: either side < 25cm
    // --------------------------------------------------------
    if (L < 25 || R < 25) {
      LR.write(70);       // 前轮回正  Straighten wheels
      delay(300);
      motor.setSpeed(255); // 设置后退速度255 (注释误写150)  Set reverse speed 255
      motor.backward();   // 后退  Reverse
      delay(800);         // 后退800ms  Reverse for 800ms
      motor.stop();
      motor.setSpeed(0);

      // 根据左右空旷程度选择转向方向
      // Choose turn direction based on L vs R
      if (L > R) LR.write(20);    // 左侧更空旷，左转  Left is more open, turn left
      else       LR.write(140);   // 右侧更空旷，右转  Right is more open, turn right
      delay(600);                 // 等待转向到位  Wait for steering

      motor.setSpeed(255);        // 前进速度255  Forward speed 255
      motor.forward();
      delay(1500);                // 前进1500ms  Forward for 1500ms
      motor.stop();
      motor.setSpeed(0);
    }
    // --------------------------------------------------------
    // 7. 左右距离很接近（差<10cm），回正直行并实时监测
    //    Sides similar (diff < 10cm), go straight with monitoring
    // --------------------------------------------------------
    else if (abs(L - R) < 10) {
      LR.write(70);       // 回正  Center steering
      delay(300);
      motor.setSpeed(255);
      motor.forward();

      unsigned long start = millis();
      bool danger = false;
      // 直行2秒，每100ms检测一次前方距离
      // Go straight for 2s, check front every 100ms
      while (millis() - start < 2000) {
        int front = JULI();
        if (front < 40) {   // 前方突然出现障碍（<40cm） Front suddenly <40cm
          danger = true;
          break;            // 立即退出  Exit immediately
        }
        delay(100);
      }
      motor.stop();
      motor.setSpeed(0);

      // 如果直行过程中发现危险，后退
      // If danger detected, reverse
      if (danger) {
        motor.setSpeed(255);
        motor.backward();
        delay(800);
        motor.stop();
        motor.setSpeed(0);
      }
    }
    // --------------------------------------------------------
    // 8. 正常避障左：左侧更空旷
    //    Normal avoidance left: L > R
    // --------------------------------------------------------
    else if (L > R) {
      LR.write(20);       // 左转  Turn left
      delay(600);
      motor.setSpeed(255);
      motor.forward();
      delay(1500);        // 前进1500ms
      motor.stop();
      motor.setSpeed(0);
    }
    // --------------------------------------------------------
    // 9. 正常避障右：右侧更空旷（含L==R且差>=10的情况）
    //    Normal avoidance right: R >= L (includes tie with diff>=10)
    // --------------------------------------------------------
    else {
      LR.write(140);      // 右转  Turn right
      delay(600);
      motor.setSpeed(255);
      motor.forward();
      delay(1500);
      motor.stop();
      motor.setSpeed(0);
    }
  }
  // ----------------------------------------------------------
  // 10. 前方安全，全速直行
  //     Path clear, full speed ahead
  // ----------------------------------------------------------
  else {
    motor.setSpeed(255);
    motor.forward();
  }
}

// ============================================================
// printSomeInfo()：调试函数，打印电机状态（当前未使用）
// Debug function, prints motor state (currently unused)
// ============================================================
void printSomeInfo() {
  Serial.print("Mv:");
  Serial.print(motor.isMoving());   // 是否运动中  Is moving?
  Serial.print(" Sp:");
  Serial.println(motor.getSpeed()); // 当前速度  Current speed
}

// ============================================================
// JULI()：超声波测距函数，返回厘米值
// Ultrasonic distance function, returns centimeters
// ============================================================
int JULI() {
  // 发送10us触发脉冲
  // Send 10us trigger pulse
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // 测量Echo高电平时间（微秒）
  // Measure Echo high time (microseconds)
  duration = pulseIn(echoPin, HIGH);
  // 距离换算：声速343m/s，除以2（往返），再除以29.1得到厘米
  // Convert to cm: (duration/2) / 29.1
  cm = (duration / 2) / 29.1;

  delay(20);   // 高频扫描间隔，保证超声波模块稳定（可调10~30）
               // Short delay for sensor stabilization (adjustable 10~30)
  return cm;
}