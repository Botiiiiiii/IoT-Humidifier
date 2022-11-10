#include <SoftwareSerial.h>
#include <AFMotor.h>

AF_DCMotor motor_1(1);
AF_DCMotor motor_2(2);

void setup(){
  Serial.begin(9600);
  Serial.println("Start Receive");
  motor_1.setSpeed(100);
  motor_2.setSpeed(100);
  motor_1.run(RELEASE);
  motor_2.run(RELEASE);
}

void loop(){
  char buf[2];
  char str[20];
  char wait;
  String sig;

// motor_1 은 backward 가 앞으로 간다.

  while(Serial.available()){
    sig = Serial.readStringUntil('\n');
  
    Serial.println("Input : " + sig);
    
    sig.substring(0,2).toCharArray(buf, 3);
  
    if(buf[0] == 'Q'){
      sprintf(str, "Data : %c", buf[1]);
      Serial.println(str);
    // STOP motor
      if(buf[1] == '0'){
        Serial.println("STOP");
        motor_1.run(RELEASE);
        motor_2.run(RELEASE);
      }
      // RUN motor
      else if(buf[1] == '1'){
        Serial.println("RUN");
      motor_1.setSpeed(100);
      motor_2.setSpeed(100);
        motor_1.run(BACKWARD);
        motor_2.run(FORWARD);
      }
     
     // Turn Left
     else if(buf[1] == '2'){
        Serial.println("Turn LEFT");
      motor_1.setSpeed(140);
      motor_2.setSpeed(80);
        motor_1.run(BACKWARD);
        motor_2.run(FORWARD);
      }
     
     // Turn Right
     else if(buf[1] == '3'){
        Serial.println("Turn RIGHT");
      motor_1.setSpeed(80);
      motor_2.setSpeed(140);
        motor_1.run(BACKWARD);
        motor_2.run(FORWARD);
      }

      // Back motor
      else if(buf[1] == '4'){
        Serial.println("BACK");
      motor_1.setSpeed(80);
      motor_2.setSpeed(80);
        motor_1.run(FORWARD);
        motor_2.run(BACKWARD);
      }
     
     // LEFT BACK
      else if(buf[1] == '5'){
        Serial.println("Turn LEFT BACK");
      motor_1.setSpeed(80);
      motor_2.setSpeed(48);
        motor_1.run(FORWARD);
        motor_2.run(BACKWARD);
      }
     // turn RIGHT
      else if(buf[1] == '6'){
        Serial.println("Turn RIGHT BACK");
      motor_1.setSpeed(48);
      motor_2.setSpeed(80);
        motor_1.run(FORWARD);
        motor_2.run(BACKWARD);
      }
     
      else{
        Serial.println("EEE");
      }
    }
  }
}
