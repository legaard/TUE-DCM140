#include <Stepper.h>

#define STEPS 32
#define SPEED_FAST 640
#define SPEED_MEDIUM 320
#define SPEED_SLOW 160
#define ONE_REVOLUTION 2048
#define LEFT 1
#define RIGHT 2

Stepper rightStepper(STEPS, 7, 5, 6, 4);
Stepper leftStepper(STEPS, 11, 9, 10, 8);

boolean leftStepperDirection = true; // true = clockwise, false = counter clockwise
boolean rightStepperDirection = true; // true = clockwise, false = counter clockwise

int leftStepsRemaining = 0;
int rightStepsRemaining = 0;

int currentSpeed = SPEED_FAST;

boolean firstTime = true;

void setup() {
  Serial.begin(9600);

  //Setup pins from 4-11
  for (int i = 4; i <= 11; i++) {
    pinMode(i, OUTPUT);
  }

  setStepperSpeed(currentSpeed);
  Serial.println("Setup is done...");
}

void loop() {
  if (firstTime) {
    turn(LEFT, 360);
    turn(RIGHT, 360);
    firstTime = false;
  }

  // needs to be call in every loop – can NOT be omitted
  updateStepperPositions();
}

/* HELPER METHODS */
void updateStepperPositions() {
  if (leftStepsRemaining > 0) {
    leftStepper.step(leftStepperDirection ? 1 : -1);
    leftStepsRemaining = leftStepsRemaining - 1;
  }

  if (rightStepsRemaining > 0) {
    rightStepper.step(rightStepperDirection ? 1 : -1);
    rightStepsRemaining = rightStepsRemaining - 1;
  }
}

void turn(int stepper, double degrees) {
  double oneDegree = ONE_REVOLUTION / 360.0;
  double absDegrees = abs(degrees);

  switch(stepper) {
    case LEFT:
      leftStepperDirection = degrees > 0;
      leftStepsRemaining = (int) (oneDegree * absDegrees);
      break;
    case RIGHT:
      rightStepperDirection = degrees > 0;
      rightStepsRemaining = (int) (oneDegree * absDegrees);
      break;
  }
}

void setStepperSpeed(int newSpeed) {
  leftStepper.setSpeed(newSpeed);
  rightStepper.setSpeed(newSpeed);
  Serial.println("Speed set to: " + newSpeed);
}
