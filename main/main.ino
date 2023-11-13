#include "MenuDisplay.h"
enum States = {
  initializing,
  menu,
  calibrate,
  automatic,
  manuel
};

char** menu_items = { "Calibrate", "Automatic", "Manuel" };
char** calibrate_items = { "Open Position", "Close Position", "Half Position", "Temperature" };

//MenuDisplays Menu, Calibrate;
int upButtonPin;
int downButtonPin;
int exitButtonPin;
int selectButtonPin;

enum CursorStates = {
  calibrate,
  automatic,
  maunuel,
  increase,
  decrease,
}

enum ControlOption = { up, down, select }

struct Settings {
  long open_pose;
  long closed_pose;
  long half_pose;
  long desired_temp;
};

const long lighting_thres;
//store in ee prom
Settings settings = { 0, 0, 0, 0 };
States myState = initializing;

void handleExit() {
  myState = menu;
}

void handleDisplay(MenuDisplay* menu, ControlOption option) {
  switch (option) {
    case up:
      break;
    case down:
      break;
    case select:
      break;
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(upButtonPin, INPUT_PULLUP);
  pinMode(downButtonPin, INPUT_PULLUP);
  pinMode(exitButtonPin, INPUT_PULLUP);
  pinMode(selectButtonPin, INPUT_PULLUP);
}

void loop() {
  // put your main code here, to run repeatedly:
  switch (myState) {
    case initializing:
      //loading settings
      break;
    case menu:
      if (digitalRead(upButtonPin) == LOW) {
      }
      if (digitalRead(downButtonPin) == LOW) {
      }
      if (digitalRead(selectButtonPin) == LOW) {
      }
      // if down button pressed
      break;
    case calibrate:
      // calibration start
      // open state, close state, half open
      // desired temperature
      if (digitalRead(upButtonPin) == LOW) {
      }
      if (digitalRead(downButtonPin) == LOW) {
      }
      if (digitalRead(selectButtonPin) == LOW) {
      }
      break;
    case automatic:
      if (digitalRead(upButtonPin) == LOW) {
      }
      if (digitalRead(downButtonPin) == LOW) {
      }
      if (digitalRead(selectButtonPin) == LOW) {
      }
      break;
    case manuel:
      if (digitalRead(upButtonPin) == LOW) {
      }
      if (digitalRead(downButtonPin) == LOW) {
      }
      if (digitalRead(selectButtonPin) == LOW) {
      }
      break;
  }
}

/*//counter updates with interrupt
volatile double counter = 0;
//interrupt numbers
const uint8_t chAInter = 0;
const uint8_t chBInter = 1;
//input pins
const uint8_t chAInput = 2;
const uint8_t chBInput = 3;

void changeA() {
  //update counter based on event
  if (digitalRead(chAInput) != digitalRead(chBInput)) {
    counter++;
  }
  else {
    counter--;
  }
}

void changeB() {
  //update counter based on event
  if (digitalRead(chAInput) == digitalRead(chBInput)) {
    counter++;
  }
  else {
    counter--;
  }
}

void setup() {
  // interupt on channel A and B hall effect sensors
  attachInterrupt(chAInter, changeA, CHANGE);
  attachInterrupt(chBInter, changeB, CHANGE);
  pinMode(chAInput, INPUT);
  pinMode(chBInput, INPUT);
  Serial.begin(9600);

}

void loop() {
  Serial.println(counter);
}
*/
/* LED display
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
// Declaration for SSD1306 display connected using software SPI (default case):
#define OLED_PICO 9
#define OLED_CLK 10
#define OLED_DC 11
#define OLED_CS 12
#define OLED_RESET 13

Adafruit_SSD1306 display(OLED_PICO, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
void setup() {
  pinMode(2, OUTPUT);
  pinMode(3, INPUT);
  display.begin(SSD1306_SWITCHCAPVCC);
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000);  // Pause for 2 seconds
  // Clear the buffer
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
}

void loop() {
  //set to low
  digitalWrite(2, LOW);
  //wait 10 microseconds
  delayMicroseconds(10);
  //set to high to send the signal
  digitalWrite(2, HIGH);
  delayMicroseconds(10);
  //signal sent
  digitalWrite(2, LOW);
  //wait for the echo signal
  while(digitalRead(3) < 1){}
  //receiving the return signal
  unsigned long curr_time = micros();
  while(digitalRead(3) > 0){}
  //calculate distance
  unsigned long t_roundtrip = micros() - curr_time;
  double distance = t_roundtrip * 0.01715;

  //displaying
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print(distance);
  display.display();
  delay(1000);
}
*/
