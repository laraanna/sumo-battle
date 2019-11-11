#include <Wire.h>
#include <Zumo32U4.h>

Zumo32U4LCD lcd;
Zumo32U4Motors motors;
Zumo32U4ButtonA buttonA;
Zumo32U4ProximitySensors proxSensors;
Zumo32U4LineSensors lineSensors;

#define NUM_SENSORS 3
const uint16_t lineSensorThreshold = 500;
unsigned int lineSensorValues[NUM_SENSORS];

// set variables for defining the direction to scan
int scanDirection;
int scanDir = "right";

// set variables realted to speed
int lowSpeed = 100;
int turnSpeed = 250;
int normalSpeed = 300;
int attackSpeed = 400;

// variables related to proximity sensors
int leftSum;
int rightSum;
int sum;

// define beginning state as scanning mode
int state = 1;


// set up function
void setup() {
  
  //initializing line and proximit sensors
  lineSensors.initThreeSensors();
  proxSensors.initFrontSensor();

  // call function to detect if button A was pressed
  waitForButtonToPress();
  
}


void waitForButtonToPress() {
  lcd.clear();
  lcd.print(F("Press A"));
  buttonA.waitForButton();
  lcd.clear();
}


// the drive function enables the robot to drive forward
void drive() {
//  lcd.clear();
//  lcd.print("drive");

  // Check for the borders by reading the line sensor values
  lineSensors.read(lineSensorValues);

  // If the first lSV is below our threshold of 500 a line is detected on the right side
  if (lineSensorValues[0] < lineSensorThreshold) {
    // changing the scan direction to right and update the state to driving backwards
    scanDir = "right";
    state = 3;
    return;
  }

  // If the last lSV is below our threshold of 500 a line is detected on the left side
  if (lineSensorValues[NUM_SENSORS - 1] < lineSensorThreshold) {
    // changing the scan direction to left and update the state to driving backwards
    scanDir = "left";
    state = 3;
    return;
  }

  // read the proximity sensor values
  proxSensors.read();
  leftSum = proxSensors.countsFrontWithLeftLeds();
  rightSum = proxSensors.countsFrontWithRightLeds();

  // sum of the values detected from the left and right LEDs
  sum = rightSum + leftSum;

  // a stronger signal is detected; 
  if (sum >= 4) {
    // we set the motor speed to attack mode
    motors.setSpeeds(attackSpeed, attackSpeed);
  
  // we see nothing therefore we move slowly forward and pay attention in case something pops up
  } else if (sum == 0) {
    
    // slowly moving forward
    motors.setSpeeds(lowSpeed, lowSpeed);
    
    // in case on the left side the opponent appears
    if (leftSum >= 2) {
      // we change direction to left and update the state to scanning
      scanDir = "left";
      state = 1;
    }
    
    // in case on the right side the opponent appears
    if (rightSum >= 2) {
      // we change direction to right and update the state to scanning
      scanDir = "right";
      state = 1;
    }
    
  // not a strong reading (between 1 and 3)
  } else {

    // right reading is stronger, so veer to the right
    if (rightSum > leftSum) {
      motors.setSpeeds(turnSpeed, 0);

    // left reading is stronger, so veer to the left
    } else if (rightSum < leftSum) {
      motors.setSpeeds(0, turnSpeed);
    
    // both readings are equal, so we drive forward
    } else {
      motors.setSpeeds(normalSpeed, normalSpeed);
    }
  }

}


// the scan function enables the robot to scan around its environment, looking for his opponent
void scan() {
  //  lcd.clear();
  //  lcd.print("scan");
  
  // get time in this function
  unsigned long lastSampleTimeScan = millis();

  // while the time in this function spent is below or equal to 2000ms scan and turn
  while ((millis() - lastSampleTimeScan) <= 2000) {
     
     // turn to the left
     if (scanDir == "left") {
      motors.setSpeeds(-turnSpeed, turnSpeed);
   
    // turn to the right
     } else if (scanDir == "right") {
      motors.setSpeeds(turnSpeed, -turnSpeed);
     }

     // read the proximity sensors
     proxSensors.read();
     // in case the left or right count is more than 0 and 1 let's change to the driving state
     if (proxSensors.countsFrontWithLeftLeds() >= 2 || proxSensors.countsFrontWithRightLeds() >= 2) {
      state = 2;
      return;
     }
  
  }

  // if too much time has passed we switch to the driving state
  state = 2;

}


// the backwards function enables the robot to drive backwards
void backwards() {
  //  lcd.clear();
  //  lcd.print("back");
  
  // get time in this function
  unsigned long lastSampleTimeBackwards = millis();

  // while the time in this function spent is below or equal to 50ms drive backwards
  while ((millis() - lastSampleTimeBackwards) <= 50) {
    motors.setSpeeds(-normalSpeed, -normalSpeed);
  }

  // if too much time has passed we switch to the scanning state
  state = 1;
}



void loop() { 

  switch (state) {
  
    // scanning
    case 1:
    scan();
    break;

    //driving
    case 2:
    drive();
    break;

    // backwards
    case 3:
    backwards();

    default:
    scan();

  }
  
}
