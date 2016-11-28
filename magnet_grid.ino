#include <Stepper.h>
#include <QueueArray.h>

#define STEPS 32
#define ONE_REVOLUTION 2048

#define SPEED_FAST 640
#define SPEED_MEDIUM 320
#define SPEED_SLOW 160

#define LEFT 1
#define RIGHT 2
#define GRID_SIZE_X 400
#define GRID_SIZE_Y 400

Stepper rightStepper(STEPS, 7, 5, 6, 4);
Stepper leftStepper(STEPS, 11, 9, 10, 8);

boolean leftStepperDirection = true; // true = clockwise, false = counter clockwise
boolean rightStepperDirection = true; // true = clockwise, false = counter clockwise

int leftStepsRemaining = 0;
int rightStepsRemaining = 0;

int currentSpeed = SPEED_FAST;

boolean isReadyForNewPoint = true;

QueueArray <int> xCoordinates;
QueueArray <int> yCoordinates;

void setup() {
  // setup serial (for debugging)
  Serial.begin(9600);
  xCoordinates.setPrinter(Serial);
  yCoordinates.setPrinter(Serial);

  // setup pins from 4 to 11
  for (int i = 4; i <= 11; i++) {
    pinMode(i, OUTPUT);
  }

  // add points
  addPoint(80, 190);
  addPoint(200, 123);
  addPoint(360, 200);

  // setting the speed for the steppers
  setStepperSpeed(currentSpeed);
  Serial.println("Setup is done...");
}

void loop() {
  // go to new point once last point has been reached
  if (isReadyForNewPoint && !xCoordinates.isEmpty() && !yCoordinates.isEmpty()) {
    int x = xCoordinates.dequeue();
    int y = yCoordinates.dequeue();
    turn(LEFT, x);
    turn(RIGHT, y);
  }

  // needs to be called in every loop – can NOT be omitted!
  updateSteppers();
}

/********** HELPER METHODS **********/
/* Stepper functions */
void updateSteppers() {
  isReadyForNewPoint = (leftStepsRemaining <= 0 && rightStepsRemaining <= 0);

  if (leftStepsRemaining > 0) {
    leftStepper.step(leftStepperDirection ? 1 : -1);
    leftStepsRemaining = leftStepsRemaining - 1;
  }

  if (rightStepsRemaining > 0) {
    rightStepper.step(rightStepperDirection ? 1 : -1);
    rightStepsRemaining = rightStepsRemaining - 1;
  }
}

void setStepperSpeed(int newSpeed) {
  leftStepper.setSpeed(newSpeed);
  rightStepper.setSpeed(newSpeed);
  Serial.println("Speed set to: " + String(newSpeed));
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

/* Point functions */
void goToPoint(int x, int y) {
  /*TODO: Implement the 'go to point function'.
  Remember to keep track of the current distance for each thread 
  */
}

void addPoint(int x, int y) {
  if (x > GRID_SIZE_X || y > GRID_SIZE_Y) {
    Serial.print("Point (" + String(x) + "," + String(y) + ") exceeds grid of ");
    Serial.println(String(GRID_SIZE_X) + "x" + String(GRID_SIZE_Y));
  } else {
    xCoordinates.push(x);
    yCoordinates.push(y);
  }
}
