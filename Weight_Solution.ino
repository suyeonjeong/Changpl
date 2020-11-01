#include "HX711.h"
#include <Keypad.h>
#define DOUT  3
#define CLK  2

HX711 scale;

float calibration_factor = 390; //-7050 worked for my 440lb max scale setup

int numReadings = 10;
float readings[10];   
int readIndex = 0;              // the index of the current reading
float total = 0;                  // the running total
float average = 0;                // the average
float targetWeight = 0;
float difference = 0;

float firstWarn = 0;
float secondWarn = 0;
float thirdWarn = 0;

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'0','1','2','3'},
  {'4','5','6','7'},
  {'8','9','A','B'},
  {'C','D','E','F'}
};
byte rowPins[ROWS] = {9, 8, 7, 6}; // 행(Raw)가 연결된 아두이노 핀 번호
byte colPins[COLS] = {10, 11, 12, 13}; // 열(column)가 연결된 아두이노 핀 번호

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

int speaker = 4;

void setup() {
  // put your setup code here, to run once:
    Serial.begin(9600);
  
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
    
  }
  Serial.println("Readings Initialized.");

  char customKey = 1;
  String result = "";
  Serial.print("Input: ");
  while (true) {
    
    customKey = customKeypad.waitForKey();
    if (customKey != 'F') {
      result += customKey;
      Serial.print(customKey);
    }
    else{
      Serial.print("\n");
      break;
    }
  }

  targetWeight = result.toFloat();
  firstWarn = targetWeight * 0.2;
  secondWarn = targetWeight * 0.1;
  thirdWarn = targetWeight * 0.05;
  
  Serial.print("Target weight set to ");
  Serial.print(targetWeight);
  Serial.println("");

  pinMode(speaker, OUTPUT);
  Serial.println("Speaker setup complete.");
  
  Serial.println("Entering scale mode.");
  scale.begin(DOUT, CLK);
  scale.set_scale();
  scale.tare(); //Reset the scale to 0

  long zero_factor = scale.read_average(); //Get a baseline reading
  Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  Serial.println(zero_factor);
  
}

void loop() {
  
  // put your main code here, to run repeatedly:
  scale.set_scale(calibration_factor); //Adjust to this calibration factor
  
  total = total - readings[readIndex];
  readings[readIndex] = scale.get_units();
  total = total + readings[readIndex];
  readIndex = readIndex + 1;
  
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  average = total / numReadings;
  difference = targetWeight - average;
  Serial.print("Average: ");
  Serial.print(average);
  Serial.print(" g. "); //Change this to kg and re-adjust the calibration factor if you follow SI units like a sane person
  Serial.print("Difference: ");
  Serial.print(difference);
  Serial.println();
  
  if(difference < thirdWarn) {
    tone(speaker, 196, 100);
  }
  else if(difference < secondWarn) {
    tone(speaker, 262, 100);
  }
  else if(difference < firstWarn) {
    tone(speaker, 330, 100);
  }
}
