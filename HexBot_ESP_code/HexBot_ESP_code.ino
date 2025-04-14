#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// Create an instance of the Adafruit PWM Servo Driver
Adafruit_PWMServoDriver servoDriver = Adafruit_PWMServoDriver();


// Define channels for each servo motor
#define BASE_CHANNEL 0      // Channel for base servo
#define SHOULDER_CHANNEL 1  // Channel for shoulder servo
#define ELBOW_CHANNEL 2     // Channel for elbow servo
#define GRIPPER_CHANNEL 3   // Channel for gripper servo

#define BUZZER_PIN 26  // or any other GPIO you use


// Motor Pins
#define m1 19
#define m2 2
#define m3 4
#define m4 23
#define e1 5
#define e2 18

// IR Sensor Pins
#define ir1 27
#define ir2 32
#define ir3 33
#define ir4 34
#define ir5 35

// Ultrasonic Sensor
const int TRIG_PIN = 12;
const int ECHO_PIN = 13;

// Control Variables
bool started = false;
bool new_color_received = false;
String detected_Color = "";

int mapAngleToPWM(int angle) {
  return map(angle, 0, 180, 150, 600);  // Maps angle to PCA9685 PWM range
}

void setup() {
  pinMode(m1, OUTPUT); pinMode(m2, OUTPUT); pinMode(m3, OUTPUT); pinMode(m4, OUTPUT);
  pinMode(e1, OUTPUT); pinMode(e2, OUTPUT);
  pinMode(ir1, INPUT); pinMode(ir2, INPUT); pinMode(ir3, INPUT);
  pinMode(ir4, INPUT); pinMode(ir5, INPUT);
  pinMode(TRIG_PIN, OUTPUT); pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);  // Ensure buzzer is off initially

  servoDriver.begin(); // Initialize PWM servo driver
  servoDriver.setPWMFreq(60); // Set PWM frequency to 60 Hz
  Serial.println("Servo control initialized...");
  Serial.begin(115200);
}

void loop() {
  delay(100);  // Prevent Serial spam

  if (Serial.available()) {
    String command_in = Serial.readStringUntil('\n');
    command_in.trim();
    Serial.println("Received: " + command_in);

    if (command_in == "START") {
      started = true;
      Serial.println(">> START received");
    } else if (command_in == "STOP") {
      started = false;
      stopMotors();
      Serial.println(">> STOP received");
    } else if (started && (command_in == "RED" || command_in == "BLUE" || command_in == "YELLOW" || command_in == "BLACK" || command_in == "WHITE")) {
      detected_Color = command_in;
      new_color_received = true;
      Serial.println(">> Color queued: " + detected_Color);
    } else {
      Serial.println(">> Ignored (either unknown or sent before START)");
    }
  }

  // Check distance to object
  long distance = getDistance();

  if (started && new_color_received && distance <= 8) {
    Serial.println(">> Object detected at close range. Handling color...");
    stopMotors();
    started = false;
    handleColor(detected_Color);
    new_color_received = false;
    return;
  }

  if (started) {
    followLine(distance);

  }
}

// === Helper Functions ===

void handleColor(String color) {
  Serial.print("Handling Color: ");
  // moveBase(90);            // Rotate base to 45 degrees
  // // delay(100);
  // moveElbow(90);           // Move elbow to 90 degrees
  // moveGripper(180); 
  Serial.println(color);
    if (color == "RED") {
     moveBase(90);            // Rotate base to 45 degrees
  // delay(100);
     moveElbow(90);           // Move elbow to 90 degrees
     moveGripper(180); 
     pickupObject();
     dropObject1();
  } else if (color == "BLUE") {
     moveBase(90);            // Rotate base to 45 degrees
  // delay(100);
     moveElbow(90);           // Move elbow to 90 degrees
     moveGripper(180); 
     pickupObject();
     dropObject2();
  } else if (color == "YELLOW") {
     moveBase(90);            // Rotate base to 45 degrees
  // delay(100);
     moveElbow(90);           // Move elbow to 90 degrees
     moveGripper(180); 
     pickupObject();
     dropObject3();
  }
  // } else if (color == "BLACK") {
  //    pickupObject();
  //    dropObject4();
  // } else if (color == "WHITE") {
  //    pickupObject();
  //    dropObject5();
  // }
  started = true;  // Resume movement
  detected_Color = "";
}

void stopMotors() {
  digitalWrite(m1, LOW); digitalWrite(m2, LOW);
  digitalWrite(m3, LOW); digitalWrite(m4, LOW);
}

long getDistance() {
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  return duration * 0.034 / 2;
}

void followLine(long distance) {
  int s1 = !digitalRead(ir1);
  int s2 = !digitalRead(ir2);
  int s3 = !digitalRead(ir3);
  int s4 = !digitalRead(ir4);
  int s5 = !digitalRead(ir5);

  if (distance > 8) {
    digitalWrite(BUZZER_PIN, LOW);
    if (s1 == 0 && s2 == 0 && s3 == 1 && s4 == 0 && s5 == 0) {
      analogWrite(e1, 220); analogWrite(e2, 220);
      digitalWrite(m1, HIGH); digitalWrite(m2, LOW);
      digitalWrite(m3, HIGH); digitalWrite(m4, LOW);
      Serial.println("FORWARD");
    } else if (s2 == 1) {
      turnRight();
    } else if (s1 == 1) {
      sharpRight();
    } else if (s4 == 1) {
      turnLeft();
    } else if (s5 == 1) {
      sharpLeft();
    } else if (s3 == 1 && s4 == 1) {
      turnLeft();
    } else if (s2 == 1 && s3 == 1) {
      turnRight();
    } else if (s1 == 1 && s2 == 1 && s3 == 1) {
      turnRight();
    } else if (s3 == 1 && s4 == 1 && s5 == 1) {
      turnLeft();
    } else if (s1 == 0 && s2 == 0 && s3 == 0 && s4 == 0 && s5 == 0) {
      stopMotors();
      Serial.println("NO LINE");
    }
  } else {
    if (!new_color_received) {
    tone(BUZZER_PIN, 1000);  // 1000 Hz tone
    delay(1500);             // Beep duration
    noTone(BUZZER_PIN);      // Turn off buzzer
    avoidObstacle();
  }
    else {
    Serial.println(">> Object detected, color already queued. Holding...");
    stopMotors();  // Stop until handleColor() is triggered in the main loop
  }
  }
}

void turnLeft() {
  analogWrite(e1, 255); analogWrite(e2, 255);
  digitalWrite(m1, LOW); digitalWrite(m2, LOW);
  digitalWrite(m3, HIGH); digitalWrite(m4, LOW);
  Serial.println("LEFT");
}

void sharpLeft() {
  analogWrite(e1, 255); analogWrite(e2, 255);
  digitalWrite(m1, LOW); digitalWrite(m2, HIGH);
  digitalWrite(m3, HIGH); digitalWrite(m4, LOW);
  Serial.println("SHARP LEFT");
}

void turnRight() {
  analogWrite(e1, 255); analogWrite(e2, 255);
  digitalWrite(m1, HIGH); digitalWrite(m2, LOW);
  digitalWrite(m3, LOW); digitalWrite(m4, LOW);
  Serial.println("RIGHT");
}

void sharpRight() {
  analogWrite(e1, 255); analogWrite(e2, 0);
  digitalWrite(m1, HIGH); digitalWrite(m2, LOW);
  digitalWrite(m3, LOW); digitalWrite(m4, HIGH);
  Serial.println("SHARP RIGHT");
}

void avoidObstacle() {
  Serial.println("Obstacle Detected");
  stopMotors(); delay(1000);
  // BACK
  digitalWrite(m1, LOW); digitalWrite(m2, HIGH);
  digitalWrite(m3, LOW); digitalWrite(m4, HIGH); delay(1000);
  Serial.println("BACK");
  // RIGHT
  analogWrite(e1, 255); analogWrite(e2, 255);
  digitalWrite(m1, LOW); digitalWrite(m2, LOW);
  digitalWrite(m3, HIGH); digitalWrite(m4, LOW); delay(1900);
  Serial.println("RIGHT");
  // LEFT
  analogWrite(e1, 255); analogWrite(e2, 255);
  digitalWrite(m1, HIGH); digitalWrite(m2, LOW);
  digitalWrite(m3, LOW); digitalWrite(m4, LOW); delay(3000);
  Serial.println("LEFT");
  // STRAIGHT
  analogWrite(e1, 255); analogWrite(e2, 255);
  digitalWrite(m1, HIGH); digitalWrite(m2, LOW);
  digitalWrite(m3, HIGH); digitalWrite(m4, LOW); delay(3000);
  Serial.println("STRAIGHT");
}


// Function to move the base servo to a specified angle
void moveBase(int baseAngle) {
  Serial.print("Rotating base to ");
  Serial.print(baseAngle);
  Serial.println("°");
  servoDriver.setPWM(BASE_CHANNEL, 0, mapAngleToPWM(baseAngle));
  delay(1000); // Delay for movement duration
}


// Function to move the shoulder servo to a specified angle
void moveShoulder(int shoulderAngle) {
  Serial.print("Moving shoulder to ");
  Serial.print(shoulderAngle);
  Serial.println("°");
  servoDriver.setPWM(SHOULDER_CHANNEL, 0, mapAngleToPWM(shoulderAngle));
  delay(1000); // Delay for movement duration
}

// Function to move the elbow servo to a specified angle
void moveElbow(int elbowAngle) {
  Serial.print("Moving elbow to ");
  Serial.print(elbowAngle);
  Serial.println("°");
  servoDriver.setPWM(ELBOW_CHANNEL, 0, mapAngleToPWM(elbowAngle));
  delay(1000); // Delay for movement duration
}

// Function to move the gripper servo to a specified angle
void moveGripper(int gripperAngle) {
  Serial.print("Moving gripper to ");
  Serial.print(gripperAngle);
  Serial.println("°");
  servoDriver.setPWM(GRIPPER_CHANNEL, 0, mapAngleToPWM(gripperAngle));
  delay(1000); // Delay for movement duration
}

void pickupObject() {
    moveBase(90);            // Rotate base to 45 degrees
  // delay(100);
    moveElbow(90);           // Move elbow to 90 degrees
    moveGripper(180);
          // Open gripper to 180 degrees
  for (double shoulderSweepAngle = 75; shoulderSweepAngle <= 140; shoulderSweepAngle += 0.85) {  
    servoDriver.setPWM(SHOULDER_CHANNEL, 0, mapAngleToPWM(shoulderSweepAngle));
    delay(50); // Delay for smooth motion
  }
  moveGripper(0); // Close gripper to 0 degrees
  
  for (double shoulderSweepAngle = 140; shoulderSweepAngle >= 75; shoulderSweepAngle -= 0.85) {  
    servoDriver.setPWM(SHOULDER_CHANNEL, 0, mapAngleToPWM(shoulderSweepAngle));
    delay(50); // Delay for smooth motion 
  }
  }
void dropObject1() {
   for (double baseSweepAngle = 90; baseSweepAngle >= 12; baseSweepAngle -= 0.85) {  
    servoDriver.setPWM(BASE_CHANNEL, 0, mapAngleToPWM(baseSweepAngle));
    delay(50); // Delay for smooth motion 
}
for (double ElbowSweepAngle = 90; ElbowSweepAngle >= 25; ElbowSweepAngle -= 0.85) {  
    servoDriver.setPWM(ELBOW_CHANNEL, 0, mapAngleToPWM(ElbowSweepAngle));
    delay(50); // Delay for smooth motion 
}
 moveGripper(180);
for (double ElbowSweepAngle = 25; ElbowSweepAngle <= 90; ElbowSweepAngle += 0.85) {  
    servoDriver.setPWM(ELBOW_CHANNEL, 0, mapAngleToPWM(ElbowSweepAngle));
    delay(50); // Delay for smooth motion 
}
 for (double baseSweepAngle = 12; baseSweepAngle <= 90; baseSweepAngle += 0.85) {  
    servoDriver.setPWM(BASE_CHANNEL, 0, mapAngleToPWM(baseSweepAngle));
    delay(50); // Delay for smooth motion 
}
}
void dropObject2(){
  for (double baseSweepAngle = 90; baseSweepAngle >= 53; baseSweepAngle -= 0.85) {  
    servoDriver.setPWM(BASE_CHANNEL, 0, mapAngleToPWM(baseSweepAngle));
    delay(50); // Delay for smooth motion 
}
for (double ElbowSweepAngle = 90; ElbowSweepAngle >= 32; ElbowSweepAngle -= 0.85) {  
    servoDriver.setPWM(ELBOW_CHANNEL, 0, mapAngleToPWM(ElbowSweepAngle));
    delay(50); // Delay for smooth motion 
}
 moveGripper(180);
for (double ElbowSweepAngle = 32; ElbowSweepAngle <= 90; ElbowSweepAngle += 0.85) {  
    servoDriver.setPWM(ELBOW_CHANNEL, 0, mapAngleToPWM(ElbowSweepAngle));
    delay(50); // Delay for smooth motion 
}
 for (double baseSweepAngle = 53; baseSweepAngle <= 90; baseSweepAngle += 0.85) {  
    servoDriver.setPWM(BASE_CHANNEL, 0, mapAngleToPWM(baseSweepAngle));
    delay(50); // Delay for smooth motion 
}
 }
 void dropObject3(){
  for (double baseSweepAngle = 90; baseSweepAngle <= 90; baseSweepAngle += 0.85) {  
    servoDriver.setPWM(BASE_CHANNEL, 0, mapAngleToPWM(baseSweepAngle));
    delay(50); // Delay for smooth motion 
}
for (double shoulderSweepAngle = 75; shoulderSweepAngle >= 70; shoulderSweepAngle -= 0.85) {  
    servoDriver.setPWM(SHOULDER_CHANNEL, 0, mapAngleToPWM(shoulderSweepAngle));
    delay(50); // Delay for smooth motion 
  }
for (double ElbowSweepAngle = 90; ElbowSweepAngle >= 25; ElbowSweepAngle -= 0.85) {  
    servoDriver.setPWM(ELBOW_CHANNEL, 0, mapAngleToPWM(ElbowSweepAngle));
    delay(50); // Delay for smooth motion 
}
 moveGripper(180);
for (double ElbowSweepAngle = 25; ElbowSweepAngle <= 90; ElbowSweepAngle += 0.85) {  
    servoDriver.setPWM(ELBOW_CHANNEL, 0, mapAngleToPWM(ElbowSweepAngle));
    delay(50); // Delay for smooth motion 
}
for (double shoulderSweepAngle = 70; shoulderSweepAngle <= 75; shoulderSweepAngle += 0.85) {  
    servoDriver.setPWM(SHOULDER_CHANNEL, 0, mapAngleToPWM(shoulderSweepAngle));
    delay(50); // Delay for smooth motion 
  }
 for (double baseSweepAngle = 90; baseSweepAngle >= 90; baseSweepAngle -= 0.85) {  
    servoDriver.setPWM(BASE_CHANNEL, 0, mapAngleToPWM(baseSweepAngle));
    delay(50); // Delay for smooth motion 
}
 }
 void dropObject4(){
  for (double baseSweepAngle = 90; baseSweepAngle <= 127; baseSweepAngle += 0.85) {  
    servoDriver.setPWM(BASE_CHANNEL, 0, mapAngleToPWM(baseSweepAngle));
    delay(50); // Delay for smooth motion 
}
for (double ElbowSweepAngle = 90; ElbowSweepAngle >= 32; ElbowSweepAngle -= 0.85) {  
    servoDriver.setPWM(ELBOW_CHANNEL, 0, mapAngleToPWM(ElbowSweepAngle));
    delay(50); // Delay for smooth motion 
}
 moveGripper(180);
for (double ElbowSweepAngle = 32; ElbowSweepAngle <= 90; ElbowSweepAngle += 0.85) {  
    servoDriver.setPWM(ELBOW_CHANNEL, 0, mapAngleToPWM(ElbowSweepAngle));
    delay(50); // Delay for smooth motion 
}
 for (double baseSweepAngle = 127; baseSweepAngle >= 90; baseSweepAngle -= 0.85) {  
    servoDriver.setPWM(BASE_CHANNEL, 0, mapAngleToPWM(baseSweepAngle));
    delay(50); // Delay for smooth motion 
}
 }
 void dropObject5(){
  for (double baseSweepAngle = 90; baseSweepAngle <= 165; baseSweepAngle += 0.85) {  
    servoDriver.setPWM(BASE_CHANNEL, 0, mapAngleToPWM(baseSweepAngle));
    delay(50); // Delay for smooth motion 
}
for (double ElbowSweepAngle = 90; ElbowSweepAngle >= 25; ElbowSweepAngle -= 0.85) {  
    servoDriver.setPWM(ELBOW_CHANNEL, 0, mapAngleToPWM(ElbowSweepAngle));
    delay(50); // Delay for smooth motion 
}
 moveGripper(180);
for (double ElbowSweepAngle = 25; ElbowSweepAngle <= 90; ElbowSweepAngle += 0.85) {  
    servoDriver.setPWM(ELBOW_CHANNEL, 0, mapAngleToPWM(ElbowSweepAngle));
    delay(50); // Delay for smooth motion 
}
 for (double baseSweepAngle = 165; baseSweepAngle >= 90; baseSweepAngle -= 0.85) {  
    servoDriver.setPWM(BASE_CHANNEL, 0, mapAngleToPWM(baseSweepAngle));
    delay(50); // Delay for smooth motion 
}
 }


