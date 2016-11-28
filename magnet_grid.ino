#include <Stepper.h>
#include <QueueArray.h>

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

boolean isReadyForCommand = true;

QueueArray <int> xCoordinates;
QueueArray <int> yCoordinates;

void setup() {
  // setup serial (for debugging)
  Serial.begin(9600);
  xCoordinates.setPrinter(Serial);
  yCoordinates.setPrinter(Serial);

  // setup pins from 4-11
  for (int i = 4; i <= 11; i++) {
    pinMode(i, OUTPUT);
  }

  // adding coordinates to the arrays
  xCoordinates.push(90); yCoordinates.push(180);
  xCoordinates.push(180); yCoordinates.push(360);
  xCoordinates.push(360); yCoordinates.push(180);

  // setting the speed for the steppers
  setStepperSpeed(currentSpeed);
  Serial.println("Setup is done...");
}

void loop() {
  //making sure that the number of x- and y-coordinates are the same
  if (xCoordinates.count() != yCoordinates.count()) {
    Serial.println("Uneven number of coordinates – please take care of it!");
    delay(5000);
    return;
  }

  //
  if (isReadyForCommand && !xCoordinates.isEmpty() && !yCoordinates.isEmpty()) {
    int x = xCoordinates.dequeue();
    int y = yCoordinates.dequeue();
    turn(LEFT, x);
    turn(RIGHT, y);
  }

  // needs to be call in every loop – can NOT be omitted
  updateStepperPositions();
}

/* HELPER METHODS */
void updateStepperPositions() {
  isReadyForCommand = (leftStepsRemaining <= 0 && rightStepsRemaining <= 0);

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
