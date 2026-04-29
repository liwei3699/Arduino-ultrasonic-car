# 🚗 Arduino Nano Obstacle Avoidance Car

A DIY obstacle-avoiding robot car based on the Arduino Nano. It uses an ultrasonic sensor on a servo mount to scan the environment forward, left, 和 right. With front-wheel steering and rear-wheel drive, it autonomously detects obstacles and picks the best path to navigate around them.[简体中文](https://github.com/liwei3699/Arduino-ultrasonic-car/blob/main/zh-cn.md)

## 🧰 Bill of Materials

| Part                                     | Qty       | Notes                                                         |
| ---------------------------------------- | --------- | ------------------------------------------------------------- |
| Arduino Nano                             | 1         | Main controller                                               |
| HC-SR04 Ultrasonic Sensor                | 1         | Distance measurement                                          |
| SG90 Servo (or similar)                  | 2         | One for steering, one for scanning the ultrasonic sensor      |
| L298N Motor Driver                       | 1         | Drives the rear DC motor(s)                                   |
| DC Gear Motor                            | 2         | Rear wheel drive (with wheels)                                |
| Car chassis with wheels                  | 1 set     | Includes front steering mechanism (e.g., Ackermann or caster) |
| Battery (7.4V or 9V)                     | 1         | Powers the motors and control board                           |
| Jumper wires, power wires                | several   | Interconnections                                              |
| Miscellaneous: screws, standoffs, switch | as needed | Assembly                                                      |

## ⚙️ How It Works

The car uses an **HC-SR04 ultrasonic sensor** to continuously measure the distance ahead. When the distance drops below a preset threshold (e.g., 90 cm), the car stops and a **servo-mounted sensor** scans the left and right sides to measure the available space (L and R). Based on L and R, different strategies are used:

- **Narrow-space avoidance**: If either side is tighter than a safe limit (e.g., 30 cm), the car reverses a short distance and then steers toward the more open side.
- **Similar distances on both sides**: If L and R differ by less than 5 cm (both relatively open), the car straightens up and drives forward while continuously monitoring the front. If the front distance suddenly falls below 55 cm, it backs up immediately.
- **Normal avoidance**: The more open side is chosen (left if L > R, right otherwise). Before turning, the car re-checks the front: if the path is already clear, it simply drives straight ahead instead of turning.

The rear wheels are driven by an **L298N module** with PWM speed control. The front wheels are precisely positioned by a **steering servo** for smooth turning.

## 🧠 Code Logic

The main `loop()` executes the following each cycle:

1. **Reset & measure**: Steering centered, sensor facing forward, front distance measured.
2. **Obstacle threshold**: If distance < threshold (default 90 cm), enter avoidance mode; otherwise drive straight at full speed.
3. **Environment scan**: Servo turns the sensor left, then right; left distance L and right distance R are recorded.
4. **Decision branches**:
   - `L < 30 || R < 30` → reverse 800 ms, then turn toward the more open side (optimized angles: left 45°, right 95°) and drive forward 800 ms.
   - `|L - R| < 5` → straighten wheels and drive forward for 2 seconds while monitoring the front every 100 ms; emergency reverse if < 55 cm.
   - `L > R` or `L < R` → check front again before turning: if clear, straighten out and drive forward; otherwise turn gently (45°/95°) and drive forward 800 ms.
5. **Loop back**: Continues autonomous navigation.

All thresholds, speed, angles, and timers are easily configurable via macros at the top of the code.

## 🔌 Pin Layout

| Arduino Nano | Module Pin           |
| ------------ | -------------------- |
| D13          | HC-SR04 Trig         |
| D11          | HC-SR04 Echo         |
| D5           | L298N IN1            |
| D4           | L298N IN2            |
| D6           | L298N ENA (PWM)      |
| D9           | Steering servo (LR)  |
| D10          | Scanning servo (CSB) |

*Pins can be changed in the code as needed.*

## 🛠 Debugging & Customization

- The Serial Monitor (9600 baud) prints L, R, and front distances in real time to help observe decision-making.
- Steering angles, speeds, and timing constants can be adapted directly to your chassis.
- It is recommended to test at low speed in an open area first, then increase speed and fine-tune thresholds.

---

**Enjoy your autonomous robot!**
