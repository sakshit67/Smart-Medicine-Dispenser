/* automatic pill dispenser example code
for parts list, circuit diagram, and instructions see:

https://www.sciencebuddies.org/science-fair-projects/project-ideas/Elec_p105/electricity-electronics/automatic-pill-dispenser

This code assumes you are using the built-in RTC on an Arduino UNO R4. To modify the code for an external RTC, see:

https://www.sciencebuddies.org/science-fair-projects/references/how-to-use-an-arduino#step36

*/

// include libraries
#include <Servo.h>  
#include <LiquidCrystal.h>
#include <Wire.h>
#include "RTClib.h"


RTC_DS3231 rtc;
Servo servo; // create servo object

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// constant variables for pins
const int servoPin = 6;
const int buttonPin = 9;
const int ledPin = 7;
const int buzzerPin = 8;

// other variables
int angle = 0;
int angleIncrement = 45;  // default 45 degrees for 4 compartments, change for different number of compartments
int newAngle;
int buttonState;
int movementDelay = 50;
int debounceDelay = 1000;

// time variables
int year;
int month;
int day; 
int hour;
int minutes;
int seconds;

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup(){ // setup code that only runs once
  pinMode(buttonPin, INPUT); // set button pin as input
  pinMode(ledPin,OUTPUT);    // set LED pin as output
  pinMode(buzzerPin,OUTPUT); // set buzzer pin as output
  digitalWrite(ledPin,LOW);  // make sure LED is off
  digitalWrite(buzzerPin,LOW);  // make sure buzzer is off
  servo.attach(servoPin);    // attach servo object to servoPin
  servo.write(angle);        // set servo to initial angle
  Serial.begin(9600);        // initialize serial for debugging


  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  #ifndef ESP8266
    while (!Serial); // wait for serial port to connect. Needed for native USB
  #endif

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
}


void loop(){  // code that loops forever
// get the current time from RTC
  DateTime now = rtc.now();
 
  // store current time variables
  year = now.year();
  month = now.month();
  day = now.day();
  hour = now.hour();
  minutes = now.minute();
  seconds = now.second();

  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 0);
  // print month/day/year
  // add leading spaces if month or day are less than 10 to keep spacing consistent
  // (always use 2 characters for month and day)
  if(month<10){
    lcd.print(" ");
  }
  lcd.print(month);
  lcd.print("/");
  if(day<10){
    lcd.print(" ");
  }
  lcd.print(day);
  lcd.print("/");
  lcd.print(year);

  // print time in hh:mm:ss format
  // add leading zeroes if minutes or seconds are less than ten to keep spacing consistent
  lcd.setCursor(0, 1);
  if(hour<10){
    lcd.print(" ");
  }
  lcd.print(hour);
  lcd.print(":");
  if(minutes<10){
    lcd.print("0");
  }
  lcd.print(minutes);
  lcd.print(":");
  if(seconds<10){
    lcd.print("0");
  }
  lcd.print(seconds);

  delay(2000);

  // control LEDs based on time
  // if(seconds == 0){
  //   digitalWrite(led1,HIGH);
  //   digitalWrite(led2,LOW);
  //   digitalWrite(led3,LOW);
  // }
  // if(seconds == 20){
  //   digitalWrite(led1,LOW);
  //   digitalWrite(led2,HIGH);
  //   digitalWrite(led3,LOW);
  // }
  // if(seconds == 40){
  //   digitalWrite(led1,LOW);
  //   digitalWrite(led2,LOW);
  //   digitalWrite(led3,HIGH);
  // }

  updateLCD();  // display the current date and time on the screen (see function below)
  
  buttonState = digitalRead(buttonPin); // read button state (you can edit the code to advance the servo when you press the button, useful for debugging)
  Serial.println(angle);                // print the servo angle

  // detect certain times and rotate the servo mechanism ahead by one compartment. 
  // the example code rotates the servo once per minute when the seconds variable equals 0.
  // change to detect different times of day, for example (hours == 9 && minutes == 0 && seconds == 0) would detect 9:00:00 AM
  // use additional "else if" conditions to detect more than one time

  if(seconds == 0){               // check for seconds = 0 (one minute intervals)
    newAngle = angle + angleIncrement; // increase angle by increment
    if (newAngle<=180){           // if the new angle is less than or equal to 180, increase angle
    	while(angle < newAngle){    // increase angle until it reaches the new angle
      	angle = angle + 1;        // increase angle by 1
      	servo.write(angle);       // move the servo
        Serial.println(angle);    // print the angle
      	delay(movementDelay);     // delay to slow down movement
    	}
      // flash LED and buzzer
      flashLED(4,150);    // flashLED(number of flashes, delay in milliseconds), see function below
    }
    else{ // if the new angle is greater than 180, reset angle to 0
      while(angle>0){         // decrease angle until it reaches 0
        angle = angle - 1;    // decrease angle by 1
        servo.write(angle);   // move the servo
        Serial.println(angle);// print the angle
        delay(movementDelay); // delay to slow down movement
      }
    }
  }
}

void flashLED(int numFlashes, int flashDelay){  // alarm function to flash LED and sound buzzer
  lcd.clear();              // clear the LCD screen
  lcd.setCursor(0, 0);      // set cursor to top left
  lcd.print("Take medicine!");  // display message
  for (int i = 0; i<numFlashes; i++){  // loop to flash LED/buzzer numFlashes times
    digitalWrite(ledPin,HIGH);         // turn LED on
    digitalWrite(buzzerPin,HIGH);      // turn buzzer on
    delay(flashDelay);                 // delay
    digitalWrite(ledPin,LOW);          // turn LED off
    digitalWrite(buzzerPin,LOW);       // turn buzzer off
    delay(flashDelay);                 // delay
  }
  // wait for button press - the code will get stuck in this loop until you press the button
  while(digitalRead(buttonPin) == LOW){}; 
  delay(1000);    // delay before clearing screen
  lcd.clear();    // clear screen
}

void updateLCD(){

  DateTime now = rtc.now();

  year    = now.year();
  month   = now.month();
  day     = now.day();
  hour    = now.hour();
  minutes = now.minute();
  seconds = now.second();

  lcd.setCursor(0, 0);

  if(month < 10) lcd.print(" ");
  lcd.print(month);
  lcd.print("/");
  if(day < 10) lcd.print(" ");
  lcd.print(day);
  lcd.print("/");
  lcd.print(year);

  lcd.setCursor(0, 1);

  if(hour < 10) lcd.print(" ");
  lcd.print(hour);
  lcd.print(":");
  if(minutes < 10) lcd.print("0");
  lcd.print(minutes);
  lcd.print(":");
  if(seconds < 10) lcd.print("0");
  lcd.print(seconds);
}

  
      
