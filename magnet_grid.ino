#include <Stepper.h>
#include <QueueArray.h>

/********** Variables and constants **********/
#define STEPS 32
#define ONE_REVOLUTION 2048

#define SPEED_FAST 1000
#define SPEED_MEDIUM 500
#define SPEED_SLOW 250

#define LEFT 'L'
#define RIGHT 'R'

// values to be defined before (first) startup
#define GRID_SIZE_X 9
#define GRID_SIZE_Y 10
#define STARTING_COORDINATE_X 4
#define STARTING_COORDINATE_Y 6
#define REEL_CIRCUMFERENCE 11; // C = d * pi

Stepper leftStepper(STEPS, 7, 5, 6, 4);
Stepper rightStepper(STEPS, 11, 9, 10, 8);

boolean leftStepperDirection = true; // true = clockwise, false = counter clockwise
boolean rightStepperDirection = true; // true = clockwise, false = counter clockwise

// values related to the steppers
const int rightStepperPosition[] = {GRID_SIZE_X, 0};
const int leftStepperPosition[] = {0, 0};
double leftCurrentDistance;
double rightCurrentDistance;
int leftStepsRemaining = 0;
int rightStepsRemaining = 0;
double leftStepSize;
double rightStepSize;

int currentSpeed = SPEED_FAST;
int delayBetweenPoints = 0;
boolean isReadyForNewPoint = true;

QueueArray <int> xCoordinates;
QueueArray <int> yCoordinates;

void setup() {
  // setup serial (for debugging) and Photon communication
  Serial.begin(9600);
  Serial1.begin(115200);
  xCoordinates.setPrinter(Serial);
  yCoordinates.setPrinter(Serial);

  // setup pins from 4 to 11 for steppers
  for (int i = 4; i <= 11; i++) {
    pinMode(i, OUTPUT);
  }

  // calculating the initial distance between steppers and magnet point
  leftCurrentDistance = getDistanceBetweenPoints(STARTING_COORDINATE_X, STARTING_COORDINATE_Y,
                                        leftStepperPosition[0], leftStepperPosition[1]);
  rightCurrentDistance = getDistanceBetweenPoints(STARTING_COORDINATE_X, STARTING_COORDINATE_Y,
                                        rightStepperPosition[0], rightStepperPosition[1]);

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

  // handle serial input
  if (Serial1.available()) {
    String input = Serial1.readString();
    char firstCharacter = input.charAt(0);

    // add a new point – e.g A(2,7)
    if (firstCharacter == 'A') {
      int x = input.substring(2, 3).toInt();
      int y = input.substring(4, 5).toInt();
      addPoint(x, y);
    }

    // wind the steppers a certain number of degrees – e.g. is R180 or L-360
    if (firstCharacter == 'R'  || firstCharacter == 'L') {
      char stepper = firstCharacter;
      int degrees = input.substring(1).toInt();
      wind(stepper, degrees);
    }

    // set the initial position of the magnet – e.g. S(5,5)
    if (firstCharacter == 'S' ) {
      reset();
      int x = input.substring(2, 3).toInt();
      int y = input.substring(4, 5).toInt();
      leftCurrentDistance = getDistanceBetweenPoints(x, y,
                                  leftStepperPosition[0], leftStepperPosition[1]);
      rightCurrentDistance = getDistanceBetweenPoints(x, y,
                                rightStepperPosition[0], rightStepperPosition[1]);
      Serial.println("Updated the initial starting position!");
    }
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

void wind(char stepper, double degrees) {
  double oneDegree = ONE_REVOLUTION / 360.0;
  double absDegrees = abs(degrees);

  // clear queue and set remaining steps to 0
  reset();

  switch(stepper) {
    case LEFT:
      leftStepperDirection = degrees < 0;
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
  double newLeftDistance = getDistanceBetweenPoints(x, y,
                              leftStepperPosition[0], leftStepperPosition[1]);
  double newRightDistance = getDistanceBetweenPoints(x, y,
                              rightStepperPosition[0], rightStepperPosition[1]);

  // calculate the difference between current distance of the steppers and new distances
  double differenceLeft = newLeftDistance - leftCurrentDistance;
  double differenceRight = newRightDistance - rightCurrentDistance;

  // change direction of the steppers based on the difference (< 0 --> reverse)
  leftStepperDirection = differenceLeft > 0;
  rightStepperDirection = differenceRight < 0;

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

  Serial.println("Added point (" + String(x) + ", " + String(y) + ") to the queue");
}

double getDistanceBetweenPoints(int x1, int y1, int x2, int y2) {
  int distance = sqrt(sq(x2 - x1) + sq(y2 - y1));
  return distance;
}

/* Misc functions */
void reset() {
  // clear point queue
  while(!xCoordinates.isEmpty() || !yCoordinates.isEmpty()) {
    xCoordinates.dequeue();
    yCoordinates.dequeue();
  }
  // set steps remaining for both steppers to 0
  leftStepsRemaining = 0;
  rightStepsRemaining = 0;

  Serial.println("Cleared the point queue!");
}
