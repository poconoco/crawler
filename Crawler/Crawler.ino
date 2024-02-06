#include <Servo.h>

const int TRIG_PIN = 13;
const int ECHO_PIN = 12;

const int LEFT_MOTOR_PIN = 10;
const int RIGHT_MOTOR_PIN = 11;

enum State {
  NONE,
  FORWARD,
  SEARCHING_LEFT,
  SEARCHING_RIGHT
};

const int FORWARD_DISTANCE_THRESHOLD = 30;  // cm
const int SEARCH_STOP_DISTANCE_THRESHOLD = 35;  // cm

// Speeds are in range of 0..90, as half range of Servo input
// Left and right forward speeds are configured individually
// because often low quality 360 servos has not equal speeds,
// so need tu tune these for straight movement
const int FORWARD_SPEED_L = 53;  
const int FORWARD_SPEED_R = 30;
const int TURN_SPEED = 40;


State lastSearchingDirection = SEARCHING_LEFT;
State currentState = NONE;
long lastDistance = 100;

Servo leftMotor;
Servo rightMotor;

void processState(long distance);
void applyState(State newState);

void setup() {
  // initialize serial communication:
  Serial.begin(9600);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  leftMotor.attach(LEFT_MOTOR_PIN);
  rightMotor.attach(RIGHT_MOTOR_PIN);

  applyState(FORWARD);
}

void loop() {
  // establish variables for duration of the ping, and the distance result
  // in inches and centimeters:
  long duration, distance;

  // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(5);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH);

  // convert the time into a distance
  distance = microsecondsToCentimeters(duration);

//  Serial.print(distance);
//  Serial.print("cm");
//  Serial.println();

  processState(distance);

  lastDistance = distance;

  delay(20);
}

void processState(long distance) {
  switch(currentState) {
    case FORWARD:
      if (distance < FORWARD_DISTANCE_THRESHOLD) {
        if (lastSearchingDirection == SEARCHING_LEFT)
          applyState(SEARCHING_RIGHT);
        else
          applyState(SEARCHING_LEFT);
      }
      return;

    case SEARCHING_RIGHT:
    case SEARCHING_LEFT:
      lastSearchingDirection = currentState;
      if (distance > SEARCH_STOP_DISTANCE_THRESHOLD && distance < lastDistance) { 
        // We found a way with at least needed distance, and distance started to decrease
        applyState(FORWARD);
      }
      return;
  }

  applyState(currentState);
}

void applyState(State newState) {

  if (newState == currentState)
    return;

  // Stop before changing states
  leftMotor.write(90); 
  rightMotor.write(90); 
  delay(250);

  currentState = newState;
  switch(currentState) {
    case FORWARD:
 //     Serial.print("FORWARD\n");
      leftMotor.write(90 + FORWARD_SPEED_L); 
      rightMotor.write(90 - FORWARD_SPEED_R); 
      break;

    case SEARCHING_RIGHT:
//      Serial.print(">>\n");
      leftMotor.write(90 + TURN_SPEED); 
      rightMotor.write(90 + TURN_SPEED); 
      break;

    case SEARCHING_LEFT:
//      Serial.print("<<\n");
      leftMotor.write(90 - TURN_SPEED); 
      rightMotor.write(90 - TURN_SPEED); 
      break;
  }

}

float microsecondsToCentimeters(long microseconds) {
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the object we
  // take half of the distance travelled.
  return microseconds / 29 / 2;
}
