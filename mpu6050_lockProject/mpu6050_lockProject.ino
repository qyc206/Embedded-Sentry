// 
// Qin Ying Chen (qyc206)
// Spring 2020
// Embedded Systems
// 
// Final Project 
// Record & unlock movement sequence using accelerometer (mpu6050)
//

#include <Adafruit_MPU6050.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;
sensors_event_t a, g, temp; // a = acceleration, g = gyro, temp = temperature


// SET UP FOR CONTINUOUS AVERAGING FILTER
/* Note: each axis has an array filter that is constantly updated and 
         used to get average in each axis */

int arrInd = 0; // array index
const int arrSize = 5; // array size

float xArr[arrSize]; // x filter
float yArr[arrSize]; // y filter
float zArr[arrSize]; // z filter

float xCenter, yCenter, zCenter; // x, y, and z for neutral position
const float threshold = 0.25; // threshold set for x, y, z boundaries


// SET UP FOR RECORDING MOVEMENTS TO USE WHEN ATTEMPTING TO UNLOCK
/* Note: movements that are recorded are placed into an array of a preset size;
         each movement is represented by an integer: 1 = left (-x), 2 = right (+x), 
         3 = away (+y), 4 = towards (-y), 5 = up (+z), 6 = down (-z) */

const int moveCapacity = 10; // array capacity
float recordedMoves[moveCapacity]; // array containing moves (each integer represents a direction)
int moveSize = 0; // # of moves recorded


// FUNCTIONS FOR FINDING AND GETTING AVERAGE OF AVERAGE FILTERING ARRAY
/* Note: this set of functions are used whenever the neutral/normalized position of the 
         accelerometer is needed (initial set up & reset) */

// calculates & returns average of values in an array
float average(float arr[]){
  float sum = 0;

  for (int i = 0; i < arrSize; i = i + 1){
    sum = sum + arr[i];
  }

  return sum/arrSize;
}

// displays values of an array
void displayArr(float arr[], int aSize){
  for (int i = 0; i < aSize; i = i + 1){
    Serial.print(arr[i]);
    Serial.print(" ");
  }
  Serial.println("");
}

// fills up the array and returns an average (used in setup for calibration)
// arr = array & xyz -> 1 = x, 2 = y, 3 = z
float getArrayAvg(float arr[], int xyz){
  for (int i = 0; i < arrSize*3; i = i + 1){
    mpu.getEvent(&a, &g, &temp);
    if (xyz == 1) arr[arrInd] = a.acceleration.x;
    if (xyz == 2) arr[arrInd] = a.acceleration.y;
    if (xyz == 3) arr[arrInd] = a.acceleration.z;
    arrInd = (arrInd + 1) % arrSize;
    delay(50);
  }
  
  return average(arr);
}

// resets the coordinate values for updated neural position
// type -> 0 = first time, 1 = reset position during program, any other # = no print message
void reset(int type){
  if (type == 0){
    Serial.println("Hold & DO NOT MOVE until instructed to");
    delay(1000);
  }
  else if (type == 1) Serial.println("RESETTING (hold)");
  xCenter = getArrayAvg(xArr, 1);
  yCenter = getArrayAvg(yArr, 2);
  zCenter = getArrayAvg(zArr, 3);
}

// FUNCTION THAT RECORDS THE MOVEMENT
/* Note: this function can be used for both recording a set of movement & for checking 
         if the unlock sequence is correct */

// type -> 0 = recording values, 1 = attempting to unlock
bool recordMovements(int type){
  bool normalized = true; // flag to indicate movement detected & reset needed
  int currIdx = 0; // array index
  Serial.println("Make your move");
  
  // loop to fill array while it is not filled
  while (currIdx < moveSize){
    mpu.getEvent(&a, &g, &temp);
  
    // continuously update array + get average value
    
    xArr[arrInd] = a.acceleration.x;
    yArr[arrInd] = a.acceleration.y;
    zArr[arrInd] = a.acceleration.z;
    
    arrInd = (arrInd + 1) % arrSize;
  
    float xAvg = average(xArr); 
    float yAvg = average(yArr); 
    float zAvg = average(zArr);

    // if zAvg is negative then the board was flipped over (user indicates they want to start over)
    if (zAvg < 0){
      currIdx = 0;
      reset(1);
      Serial.println("START OVER: Make your move");
    }

    // if movement was just detected, reset the neutral position 
    if (!normalized){
      reset(1);
  
      bool normal = true;
      // check 6 values after movement detected
      for (int i = 0; i < 6; i = i + 1){
        // if x is not in recorded range (xCenter +/- bound), not returned to normal, skip detection
        if (xAvg > (xCenter+threshold) || xAvg < (xCenter-threshold)){
          normal = false; 
          break;
        }
        // if y is not in recorded range (yCenter +/- bound), not returned to normal, skip detection
        if (yAvg > (yCenter+threshold) || yAvg < (yCenter-threshold)){
          normal = false; 
          break;
        }
        // if z is not in recorded range (xCenter +/- bound), not returned to normal, skip detection
        if (zAvg > (zCenter+threshold) || zAvg < (zCenter-threshold)){
          normal = false; 
          break;
        }
      }
  
      if (normal){
        normalized = true; // normalized = true 
        Serial.println("Make your next move");
      }
    }
  
    // x, y, z in normalized range (waiting to detect movement)
    while(normalized){
      float deltaX = xAvg-xCenter;
      float deltaY = yAvg-yCenter;
      float deltaZ = zAvg-zCenter;
      
      // skip if neutral (no movement detected)
      if ((abs(deltaX) < threshold && abs(deltaX) > -1*threshold) && (abs(deltaY) < threshold && abs(deltaY) > -1*threshold) && 
            (abs(deltaZ) < threshold && abs(deltaZ) > -1*threshold)){
        Serial.println("Waiting..");
        break;
      }
  
      // detect changes in x (left/right)    
      if (abs(deltaX) > abs(deltaY) && abs(deltaX) > abs(deltaZ)){
        if (deltaX > threshold){
          Serial.println("right");
          if (type == 0) recordedMoves[currIdx] = 2; // right
          else if (type == 1){
            if (recordedMoves[currIdx] != 2) return false; // failed to match
          }
        }
        else if (deltaX < -1*threshold){
          Serial.println("left");
          if (type == 0) recordedMoves[currIdx] = 1; // left
          else if (type == 1){
            if (recordedMoves[currIdx] != 1) return false; // failed to match
          }
        }
        else break;
  
        currIdx++;
        normalized = false;
        break;
      }
      
      // detect changes in y (away/towards)
      if (abs(deltaY) > abs(deltaX) && abs(deltaY) > abs(deltaZ)){
        if (deltaY > threshold){
          Serial.println("away");
          if (type == 0) recordedMoves[currIdx] = 3; // away
          else if (type == 1){
            if (recordedMoves[currIdx] != 3) return false; // failed to match
          }
        }
        else if (deltaY < -1*threshold){
          Serial.println("towards");
          if (type == 0) recordedMoves[currIdx] = 4; // towards
          else if (type == 1){
            if (recordedMoves[currIdx] != 4) return false; // failed to match
          }
        }
        else break;
  
        currIdx++;
        normalized = false;
        break;
      }
  
      // detect changes in z (up/down)
      if (abs(deltaZ) > abs(deltaX) && abs(deltaZ) > abs(deltaY)){
        if (deltaZ > threshold){
          Serial.println("up");
          if (type == 0) recordedMoves[currIdx] = 5; // up
          else if (type == 1){
            if (recordedMoves[currIdx] != 5) return false; // failed to match
          }
        }
        else if (deltaZ < -1*threshold){
          Serial.println("down");
          if (type == 0) recordedMoves[currIdx] = 6; // down
          else if (type == 1){
            if (recordedMoves[currIdx] != 6) return false; // failed to match
          }
        }
        else break;
  
        currIdx++;
        normalized = false;
        break;
      }
  
    }
  
    delay(50);
  }

  return true;
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10); // will pause until serial console opens

  Serial.println("MPU6050 test!");

  // try to initialize
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  
  Serial.println("WELCOME! Before you begin: ");
  Serial.println("(1) Prompts will be displayed on serial monitor");
  Serial.println("(2) Each sequence can have 1-10 moves");
  Serial.println("(3) You MUST keep it facing UPWARDS when completing movements");
  Serial.println("(4) Flip to reset DURING a sequence (otherwise refer to prompts)");
  Serial.println("(5) make each move STRONG\n");

  Serial.println("FLIP and HOLD until desired # of moves displays (flip back when done)");
  delay(500);
  reset(3); // update position of accelerometer

  while (zCenter > 0) reset(3); // wait for flip

  // while flipped over, increment & display # of moves
  while (zCenter < 0){
    Serial.println(moveSize+1);
    moveSize++;
    reset(3);
  }

  Serial.print("# of moves: ");
  Serial.println(moveSize); 
  Serial.println("You may begin..\n");

  // first time trying to record 
  reset(0);
  recordMovements(0);

  // prompt user on how they can reset the sequence 
  Serial.println("If unhappy with combo, reset by flipping");
  delay(500);
  reset(3); // update position of accelerometer
  
  while (zCenter < 0){ // if user wants to reset the lock sequence
    reset(0);
    recordMovements(0);

    // prompt user on how they can reset the sequence 
    Serial.println("If unhappy with combo, reset by flipping");
    delay(500);
    reset(3); // update position of accelerometer
  }

  Serial.println("Seems like you are happy; BEGIN UNLOCKING ATTEMPTS\n");
  delay(1000);
}

void loop() {
    bool correctCombo = recordMovements(1); // returns true if correct and false if wrong move

    if (correctCombo) {
      Serial.println("CONGRATULATIONS! You've unlocked it!");
      while(zCenter > 0); // stuck/finished 
    }
    else if (!correctCombo){
      Serial.println("FAILED TO UNLOCK: wrong combo");
      while (!correctCombo){ // while attempt failed, prompt user on how they can try again
        Serial.println("Flip if you want to try unlocking again");
        reset(3); // update position of accelerometer 
        delay(500);
        if (zCenter < 0){ // if accelerometer is flipped over
          Serial.println("");
          reset(0); // reset position of accelerometer
          break;
        }
      }
    }

    delay(100);
}
