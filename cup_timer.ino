// include the library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// these constants describe the pins. They won't change:
const int groundpin = 18;             // analog input pin 4 -- ground
const int powerpin = 19;              // analog input pin 5 -- voltage
const int xpin = A3;                  // x-axis of the accelerometer
const int ypin = A2;                  // y-axis
const int zpin = A1;                  // z-axis (only on 3-axis models)

// Delay between chugs
const int chug_delay = 100;

// How many readings to store
const int numReadings = 10;
int xReadings[numReadings];
int yReadings[numReadings];
int zReadings[numReadings];

// Make sure we dont change too quickly
int periods_since_change = 0;

// Whether or not currently chugging
boolean is_chugging = false;

// This is for a timeout to rest
int time_since_last_chug = 0;

void setup()
{
  // initialize the serial communications:
  Serial.begin(9600);
  
  // Provide ground and power by using the analog inputs as normal
  // digital pins.  This makes it possible to directly connect the
  // breakout board to the Arduino.  If you use the normal 5V and
  // GND pins on the Arduino, you can remove these lines.
  pinMode(groundpin, OUTPUT);
  pinMode(powerpin, OUTPUT);
  digitalWrite(groundpin, LOW); 
  digitalWrite(powerpin, HIGH);
  
  // Fill the initial data
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    xReadings[thisReading] = analogRead(xpin);
    yReadings[thisReading] = analogRead(ypin);
    zReadings[thisReading] = analogRead(zpin);
  }
  
  lcd.begin(16, 2);
  lcd.print("Calibrating: ");
}

void loop()
{
  if (! is_chugging && time_since_last_chug > chug_delay) {
    lcd.begin(16, 2);
    lcd.print("Ready: ");
    lcd.setCursor(0, 1);
    lcd.print("                ");
  }
  
  periods_since_change += 1;
  int xval = analogRead(xpin);
  int yval = analogRead(ypin);
  int zval = analogRead(zpin);
  
  // Update the lists
  for (int thisReading = 1; thisReading < numReadings; thisReading++) {
    xReadings[thisReading] = xReadings[thisReading - 1];
    yReadings[thisReading] = yReadings[thisReading - 1];
    zReadings[thisReading] = zReadings[thisReading - 1];
  }

  xReadings[0] = xval;
  yReadings[0] = yval;  
  zReadings[0] = zval;
  
  // Get the average
  int xSum = 0;
  int ySum = 0;
  int zSum = 0;
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    xSum += xReadings[thisReading];
    ySum += yReadings[thisReading];
    zSum += zReadings[thisReading];
  }
  int xAvg = xSum / numReadings;
  int yAvg = ySum / numReadings;
  int zAvg = zSum / numReadings;
  
    // Get the average
  int xDev = 0;
  int yDev = 0;
  int zDev = 0;
  for (int thisReading = 1; thisReading < numReadings; thisReading++) {
    xDev += sq(xReadings[thisReading] - xAvg);
    yDev += sq(yReadings[thisReading] - yAvg);
    zDev += sq(zReadings[thisReading] - zAvg);
  }
  xDev /= numReadings;
  yDev /= numReadings;
  zDev /= numReadings;
  xDev = sqrt(xDev);
  yDev = sqrt(yDev);
  zDev = sqrt(zDev);
  
  if ((abs(zAvg - zval ) > max(25, zDev) || 
      ((zDev == 0 && xDev == 0) && (abs(zAvg - zval) > 5) 
        && max(abs(xAvg - xval), abs(yAvg - yval)) > 5))
        && periods_since_change > 4) {
    Serial.println("MOTION");
    Serial.println(periods_since_change);
    periods_since_change = 0;
          
    // This is where we end the chug
    if (is_chugging) {  
      lcd.begin(16, 2);
      lcd.print("Your time is: "); 
      time_since_last_chug = 0;
        
      // Clear the screen
      lcd.setCursor(0, 1);
      lcd.print("                ");
      // print the number of seconds since reset:
      lcd.setCursor(0, 1);
      lcd.print(periods_since_change / 10);
      lcd.print(".");
      lcd.print(periods_since_change % 10);
      lcd.print("0 seconds");

      is_chugging = false;
    } else {
      lcd.begin(16, 2);
      lcd.print("Chugging: "); 
      is_chugging = true;
      time_since_last_chug = 0;
    }
  }
  
  if (time_since_last_chug > chug_delay && is_chugging) {
    is_chugging = false; 
    lcd.begin(16, 2);
    lcd.print("Ready: ");
    lcd.setCursor(0, 1);
    lcd.print("                ");
  }
  
  if (!is_chugging) {
    time_since_last_chug += 1; 
  }
  
  // delay before next reading:
  delay(100);
}
