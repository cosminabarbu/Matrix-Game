
#include "LedControl.h"

const int fillPercent = 65;
const int dinPin = 12;
const int clockPin = 11;
const int loadPin = 10;

const int xPin = A0;
const int yPin = A1;
const int buttonPin = 2;
const int buzzerPin = 3;

LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);

byte matrixBrightness = 2;
byte xPos = 0;
byte yPos = 0;
byte xLastPos = 0;
byte yLastPos = 0;
byte xBomb = 0;
byte yBomb = 0;

const int minThreshold = 200;
const int maxThreshold = 600;

const byte moveInterval = 100;
unsigned long lastMoved = 0;
const int bombDuration = 2000;

const byte matrixSize = 8;
bool matrixChanged = true;

byte matrix[matrixSize][matrixSize] = {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0}
};

unsigned long bombTime = 0;
unsigned long lastPlayerBlink = 0;
unsigned long lastBombBlink = 0;
byte playerBlinkState = 1;
byte bombBlinkState = 1;
const int blinkTimePlayer = 400;
const int blinkTimeBomb = 50;
byte buttonState = 0;
byte bombPlanted = 0;
byte bombActivated = 0;
bool exploded = false;
bool firstBombPlanted = true;

unsigned int lastDebounceTime = 0;
unsigned int debounceDelay = 50;
byte reading = 1;
byte lastReading = 1;

void blinkPlayer() {
  if (millis() > lastPlayerBlink + blinkTimePlayer) {
    playerBlinkState = !playerBlinkState;
    setMatrixElement(xPos, yPos, playerBlinkState);
    // matrix[xPos][yPos] = playerBlinkState;
    lastPlayerBlink = millis();
    matrixChanged = true;
  }
}

void blinkBomb() {
  if(millis() - bombTime < bombDuration){

    if(millis() > lastBombBlink + blinkTimeBomb){
      bombBlinkState = !bombBlinkState; 
     // matrix[xBomb][yBomb] = bombBlinkState;
      setMatrixElement(xBomb, yBomb, bombBlinkState);
      lastBombBlink = millis();
      matrixChanged = true;
    }
}
else {
  explode();
  tone(buzzerPin, 1000); 
  delay(500); 
  noTone(buzzerPin);
  }
}


void plantBomb(int x, int y){
  xBomb = x;
  yBomb = y;
  bombPlanted = true;
  // set bombActivated to true immediately for the first bomb planted
  if (firstBombPlanted) {
    bombActivated = true;
    firstBombPlanted = false;
  }

  // initialize bombTime to ensure the bomb starts blinking immediately
  bombTime = millis() ;
}

void explode(){
  exploded = true;
  if(bombPlanted){
    //explode the neighbouring walls of the bomb
    for ( int i = -1; i <= 1; i++){
      for( int j = -1; j <= 1; j++){
        int explodeX = xBomb + i;
        int explodeY = yBomb + j;

        if(explodeX >= 0 && explodeX < matrixSize && explodeY >= 0 && explodeY < matrixSize){
          if(matrix[explodeX][explodeY] == 1){
            matrix[explodeX][explodeY] = 0; // detonate the wall
          }
        }
      }
    }

    matrix[xBomb][yBomb] = 0; // clear the bomb position
    bombPlanted = false;
    matrixChanged = true;
  }
}

void buttonDebounce(){
  reading = digitalRead(buttonPin);
  if(reading != lastReading){
    lastDebounceTime = millis();
  }

  if((millis() - lastDebounceTime ) > debounceDelay){
    if(reading != buttonState){
      buttonState = reading;
      if(buttonState == LOW){
        plantBomb(xPos, yPos);
      }
    }
  }

  lastReading = reading;

  //check for bomb activation

  if(bombPlanted && !bombActivated ){
    if(millis() - bombTime >= bombDuration){
      bombActivated = true;
    }
  }

  if(bombActivated){
    blinkBomb();
    // if(xBomb == xPos || yBomb == yPos || (xBomb - 1) == xPos || (yBomb - 1) == yPos || (xBomb + 1) == xPos || (yBomb + 1) == yPos){
    //   gameOver();
    // }
  }

    if (bombActivated && (xBomb == xPos || yBomb == yPos 
  //   // || (xBomb - 1) == xPos || (yBomb - 1) == yPos || (xBomb + 1) == xPos || (yBomb + 1) == yPos 
  //   // || ( ( (xBomb -1 ) == xPos) && ( (yBomb - 1) == yPos ) ) || ( ( (xBomb -1 ) == xPos) && ( (yBomb + 1) == yPos ) ) ||
  //   //  ( ( (xBomb + 1 ) == xPos) && ( (yBomb - 1) == yPos ) ) || ( ( (xBomb + 1 ) == xPos) && ( (yBomb + 1) == yPos ) ) 
      ) ) {
  //   // player loses the game if the bomb catches them
    // gameOver();
    // tone(buzzerPin, 1200);
    // delay(500);
    // noTone(buzzerPin);
  // }

}
}

void initializeMatrix() {
  lc.shutdown(0, false);
  lc.setIntensity(0, matrixBrightness);
  lc.clearDisplay(0);
}

void clearMatrix() {
  for (int row = 0; row < matrixSize; row++) {
    for (int col = 0; col < matrixSize; col++) {
      matrix[row][col] = 0;
    }
  }
}

void setMatrixElement(int row, int col, int value) {
  matrix[row][col] = value;
}

void updateMatrixDisplay() {
  for (int row = 0; row < matrixSize; row++) {
    for (int col = 0; col < matrixSize; col++) {
      lc.setLed(0, row, col, matrix[row][col]);
    }
  }
}

void generateRandomMap() {
  clearMatrix();

  int totalElements = matrixSize * matrixSize;
  int elementsToFill = (fillPercent * totalElements) / 100;

  for (int i = 0; i < elementsToFill; i++) {
    int row = random(matrixSize);
    int col = random(matrixSize);
    setMatrixElement(row, col, 1);
  }
}


void updatePositions() {
  int xValue = analogRead(xPin);
  int yValue = analogRead(yPin);

  xLastPos = xPos;
  yLastPos = yPos;

  byte newXPos = xPos;
  byte newYPos = yPos;

  if (xValue < minThreshold) {
    newXPos = (xPos + 1) % matrixSize;
  } else if (xValue > maxThreshold) {
    newXPos = (xPos > 0) ? xPos - 1 : matrixSize - 1;
  }

  if (yValue < minThreshold) {
    newYPos = (yPos > 0) ? yPos - 1 : matrixSize - 1;
  } else if (yValue > maxThreshold) {
    newYPos = (yPos + 1) % matrixSize;
  }

  // check if the new position is an obstacle (1 in the matrix)
  if (matrix[newXPos][newYPos] == 0) {
    // update the position only if it's not an obstacle
    xPos = newXPos;
    yPos = newYPos;
  }

  if (xPos != xLastPos || yPos != yLastPos) {
    matrixChanged = true;
    matrix[xLastPos][yLastPos] = 0;
    matrix[xPos][yPos] = 1;
  }
}

// void gameOver() {
//   for (int row = 0; row < matrixSize; row++) {
//     for (int col = 0; col < matrixSize; col++) {
//       lc.setLed(0, row, col, true);  // turns on LED at col, row
//       delay(25);
//       // lc.setLed(0, row, col, false);
//     }
//   }
// }

void setup() {
  Serial.begin(9600);
  // pinMode(buzzerPin, OUTPUT);
  // noTone(buzzerPin); // ensure the buzzer is initially silent
  pinMode(buttonPin, INPUT_PULLUP);
  initializeMatrix();
  // use analogRead to set the random seed based on an unconnected pin
  randomSeed(analogRead(A2));
  generateRandomMap();
  updateMatrixDisplay();

  //  // initialize bombTime to ensure the bomb starts blinking immediately
  // bombTime = millis() - bombDuration + blinkTimeBomb;
}


void loop() {
  blinkPlayer();
  buttonDebounce();
  if (millis() - lastMoved > moveInterval) {
    updatePositions();
    lastMoved = millis();
  }

  if (matrixChanged) {
    updateMatrixDisplay();
    matrixChanged = false;
  }
}
