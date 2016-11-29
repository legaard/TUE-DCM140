#include <Stepper.h>
#include <QueueArray.h>

/********** Variables and constants **********/
#define STEPS 32
#define ONE_REVOLUTION 2048

#define SPEED_FAST 1100
#define SPEED_MEDIUM 500
#define SPEED_SLOW 250

#define LEFT 'L'
#define RIGHT 'R'

#define GRID_SIZE_X 10
#define GRID_SIZE_Y 10

#define REEL_CIRCUMFERENCE 4;

Stepper rightStepper(STEPS, 7, 5, 6, 4);
Stepper leftStepper(STEPS, 11, 9, 10, 8);

boolean leftStepperDirection = true; // true = clockwise, false = counter clockwise
boolean rightStepperDirection = true; // true = clockwise, false = counter clockwise

// values related to the steppers
const int rightStepperPosition[] = {GRID_SIZE_X + 1, -1};
const int leftStepperPosition[] = {-1, -1};
double leftCurrentDistance = 5;
double rightCurrentDistance = 5;
int leftStepsRemaining = 0;
int rightStepsRemaining = 0;
double leftStepSize;
double rightStepSize;

int currentSpeed = SPEED_FAST;
int delayBetweenPoints = 1000;
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
  addPoint(5, 5);
  addPoint(2, 1);
  addPoint(10, 0);
  addPoint(10, 10);
  addPoint(5, 6);

  // setting the speed for the steppers
  setStepperSpeed(currentSpeed);
  Serial.println("Setup is done...");
}

void loop() {
  // go to new point once last point has been reached
  if (isReadyForNewPoint && !xCoordinates.isEmpty() && !yCoordinates.isEmpty()) {
    int x = xCoordinates.dequeue();
    int y = yCoordinates.dequeue();
    delay(delayBetweenPoints);
    goToPoint(x, y);
  }

  // needs to be called in every loop – can NOT be omitted!
  updateSteppers();
}

/********** HELPER METHODS **********/
/* Stepper functions */
double leftTempValue; //temporary value for keeping track of steps taken (left)
double rightTempValue; //temporary value for keeping track of steps taken (right)

void updateSteppers() {
  isReadyForNewPoint = (leftStepsRemaining <= 0 && rightStepsRemaining <= 0);

  // left stepper
  if (leftStepsRemaining > 0) {
    if (leftTempValue >= 1) {
      // throw away the decimal by casting
      int wholeTempValue = (int) leftTempValue;
      // step the number of steps in either direction
      leftStepper.step(leftStepperDirection ? wholeTempValue : -(wholeTempValue));
      // deduct and update the steps taken
      leftStepsRemaining = leftStepsRemaining - wholeTempValue;
      leftTempValue = leftTempValue - wholeTempValue;
    } else {
      leftTempValue = leftTempValue + leftStepSize;
    }
  }

  // right stepper
  if (rightStepsRemaining > 0) {
    if (rightTempValue >= 1) {
      // throw away the decimal by casting
      int wholeTempValue = (int) rightTempValue;
      // step the number of steps in either direction
      rightStepper.step(rightStepperDirection ? wholeTempValue : -(wholeTempValue));
      // deduct and update the steps taken
      rightStepsRemaining = rightStepsRemaining - wholeTempValue;
      rightTempValue = rightTempValue - wholeTempValue;
    } else {
      rightTempValue = rightTempValue + rightStepSize;
    }
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
      leftStepSize = 1;
      leftStepsRemaining = (int) (oneDegree * absDegrees);
      break;
    case RIGHT:
      rightStepperDirection = degrees > 0;
      rightStepSize = 1;
      rightStepsRemaining = (int) (oneDegree * absDegrees);
      break;
  }
}

/* Point functions */
void goToPoint(int x, int y) {
  // calculate the distance from the steppers' position to the new point
  double newLeftDistance = sqrt(sq(x - leftStepperPosition[0]) + sq(y - leftStepperPosition[1]));
  double newRightDistance = sqrt(sq(x - rightStepperPosition[0]) + sq(y - rightStepperPosition[1]));

  // calculate the difference between current distance of the steppers and new distances
  double differenceLeft = newLeftDistance - leftCurrentDistance;
  double differenceRight = newRightDistance - rightCurrentDistance;

  // change direction of the steppers based on the difference (< 0 --> reverse)
  leftStepperDirection = differenceLeft > 0;
  rightStepperDirection = differenceRight > 0;

  // calculate the number of steps to one 'unit' in the coordinate system
  double oneUnit = ONE_REVOLUTION / REEL_CIRCUMFERENCE;

  // update steps remaining
  leftStepsRemaining = (int) abs(oneUnit * differenceLeft);
  rightStepsRemaining = (int) abs(oneUnit * differenceRight);

  leftStepSize = (double) leftStepsRemaining / (double) ONE_REVOLUTION;
  rightStepSize = (double) rightStepsRemaining / (double) ONE_REVOLUTION;

  // update distances from steppers to the new position
  leftCurrentDistance = newLeftDistance;
  rightCurrentDistance = newRightDistance;
}

void addPoint(int x, int y) {
  if (x > GRID_SIZE_X || y > GRID_SIZE_Y) {
    Serial.println("Point (" + String(x) + ", " + String(y) + ") exceeds grid of ");
    Serial.print(String(GRID_SIZE_X) + "x" + String(GRID_SIZE_Y));
    return;
  }

  if (x < -1 || y < -1) {
    Serial.println("Point (" + String(x) + ", " + String(y) + ") exceeds grid of ");
    Serial.print(String(GRID_SIZE_X) + "x" + String(GRID_SIZE_Y));
    return;
  }

  // if the point is valid --> add to queue
  xCoordinates.push(x);
  yCoordinates.push(y);

  Serial.println("Added point (" + String(x) + ", " + String(y) + ") to the list");
}
