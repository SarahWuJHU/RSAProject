#include "MenuDisplay.h"

enum States = {
  initializing,
  menu,
  calibrate,
  automatic,
  manual
};

enum calibrationStates = {
  menu_cali,
  open_position,
  close_position,
  half_position,
  temperature
}

char** menu_items = { "Calibrate", "Automatic", "manual" };
char** calibrate_items = { "Open Position", "Close Position", "Half Position", "Temperature" };

//MenuDisplays Menu, Calibrate;
int upButtonPin;
int downButtonPin;
int exitButtonPin;
int selectButtonPin;

static MenuDisplay menu_display("Menu", menu_items, 3);
static MenuDisplay calibration_display("Calibration", calibrate_items, 4);
static MotorControl motor();
static SensorControl sensor();

enum ControlOption { up,
                     down,
                     select };

struct Settings {
  long open_pose;
  long closed_pose;
  long half_pose;
  long desired_temp;
};

const long lighting_thres;
//store in ee prom
Settings settings = { 0, 0, 0, 0 };
static States myState = initializing;
static calibrationStates myCalibrationState = menu_cali;

void handleExit() {
  myState = menu;
  myCalibrationState = menu_cali;
  menu_display.resetCursorPos();
  calibration_display.resetCursorPos();
}

void handleDisplay(MenuDisplay* menu_dis, ControlOption option) {
  switch (option) {
    case up:
      menu_dis->moveCursorUp();
      break;
    case down:
      menu_dis->moveCursorDown();
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
        handleDisplay(&menu_display, up);
      }
      if (digitalRead(downButtonPin) == LOW) {
        handleDisplay(&menu_display, down);
      }
      if (digitalRead(selectButtonPin) == LOW) {
        pos = menu_display.getCursorPos();
        switch (pos) {
          case 0:
            myState = calibrate;
            menu_display.resetCursorPos();
            calibration_display.resetCursorPos();
            break;
          case 1:
            myState = automatic;
            menu_display.resetCursorPos();
            break;
          case 2:
            myState = manual;
            menu_display.resetCursorPos();
            break;
        }
      }
      // if down button pressed
      break;
    case calibrate:
      // calibration start
      // open state, close state, half open
      // desired temperature
      switch (myCalibrationState) {
        case menu_cali:
          if (digitalRead(upButtonPin) == LOW) {
            handleDisplay(&calibration_display, up);
          }
          if (digitalRead(downButtonPin) == LOW) {
            handleDisplay(&calibration_display, down);
          }
          if (digitalRead(selectButtonPin) == LOW) {
            pos = calibration_display.getCursorPos();
            switch (pos) {
              case 0:
                myCalibrationState = open_position;
                menu_display.resetCursorPos();
                calibration_display.resetCursorPos();
                break;
              case 1:
                myCalibrationState = close_position;
                menu_display.resetCursorPos();
                break;
              case 2:
                myCalibrationState = half_position;
                menu_display.resetCursorPos();
                break;
              case 3:
                myCalibrationState = temperature;
                menu_display.resetCursorPos();
                break;
            }
          }
          break;
        case open_position:
          motor.move_to_open();
          while (digitalRead(selectButtonPin) == HIGH){
            if (digitalRead(upButtonPin) == LOW){
              motor.move_up();
            }
            if (digitalRead(downButtonPin) == LOW){
              motor.move_down();
            }
          }
          motor.set_open_position(motor.position());
          settings.open_pose = motor.open_position;          
          break;
        case close_position:
          motor.move_to_close();
          while (digitalRead(selectButtonPin) == HIGH){
            if (digitalRead(upButtonPin) == LOW){
              motor.move_up();
            }
            if (digitalRead(downButtonPin) == LOW){
              motor.move_down();
            }
          }
          motor.set_close_position(motor.position());
          settings.close_pose = motor.close_position;
          break;
        case half_position:
          motor.move_to_half();
          while (digitalRead(selectButtonPin) == HIGH){
            if (digitalRead(upButtonPin) == LOW){
              motor.move_up();
            }
            if (digitalRead(downButtonPin) == LOW){
              motor.move_down();
            }
          }
          motor.set_half_position(motor.position());
          settings.half_pose = motor.half_position;
          break;
        case temperature:
          //display temperature
          
          break
      }

      break;
    case automatic:
      //sensor
      break;
    case manual:
      if (digitalRead(upButtonPin) == LOW) {
        motor.move_up();
      }
      if (digitalRead(downButtonPin) == LOW) {
        motor.move_down();
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
