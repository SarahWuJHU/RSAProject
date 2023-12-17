/* 
Sarah Wu, Ben Promisel, Chris Khoury
RSA Final Project
*/

// Include other classes from the project
#include "LightSense.h"
#include "MenuDisplay.h"
#include "BlindsMotor.h"
#include "TempSense.h"
#include "Encoder.h"
//eeprom
#include <EEPROM.h>
#include <EEWrap.h>
#define EEADDRESS 42
//OLED display settings
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_LTR329_LTR303.h"
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

// Declaration for SSD1306 display connected using software SPI (default case):
#define OLED_PICO 9
#define OLED_CLK 10
#define OLED_DC 11
#define OLED_CS 12
#define OLED_RESET 13

// Other settings for automatic control
#define MOTOR_TOLERANCE 10 // tolerance designating when the motor has reached its position
#define LIGHT_THRES 400 // boundary indicating when to consider 'light' or 'dark'
#define AUTO_TOLERANCE_TEMP 10 // tolerance for temperature sensor boundary in automatic mode
#define AUTO_TOLERANCE_LIGHT 50 // tolerance for light sensor boundary in automatic mode
#define BUTTON_BOUNCE 100 // tolerance to avoid button bouncing

// Declare states for the main control state machine
enum States {
  initializing, // setup state
  menu, // main menu
  calibrate, // calibrate menu
  automatic, // automatic mode
  manual // manual mode
};

// Declare states for motor control
enum Control {
  up,
  down
};

// Declare states to be used in the calibration menu
enum calibrationStates {
  menu_cali,
  open_position,
  close_position,
  half_position,
  temperature_cali
};

// Setting up displays for OLED screen
char calibrate_word[] = "calibrate";
char automatic_word[] = "automatic";
char manual_word[] = "manual";
char open_position_word[] = "open_position";
char close_position_word[] = "close_position";
char half_position_word[] = "half_position";
char temperature_word[] = "temperature";
const char* menu_items[] = { calibrate_word, automatic_word, manual_word };
const char* calibrate_items[] = { open_position_word, close_position_word, half_position_word, temperature_word };
static MenuDisplay menu_display("   MENU", menu_items, 3);
static MenuDisplay calibration_display("    CALIBRATION", calibrate_items, 4);

// Set pins for MenuDisplays Menu, Calibrate;
const int upButtonPin = A0;
const int downButtonPin = A1;
const int exitButtonPin = A2;
const int selectButtonPin = 4;
const int motorUpPin = 6;
const int motorDownPin = 7;
const int motorPulsePin = 5;
const int encoderUpPin = 3;
const int encoderDownPin = 2;
const int tempSensorPin = A3;

static BlindsMotor motor(motorUpPin, motorDownPin, motorPulsePin); // initialize motor class with pins
static LightSense lightSensor; // initialize light sensor class
static TempSense tempSensor(tempSensorPin); // initialize temp sensor class with pin
Adafruit_SSD1306 display(OLED_PICO, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS); // initialize OLED with pins
static States myState = initializing; // set default state to initializing
static calibrationStates myCalibrationState = menu_cali; // set original calibration state to the menu

// Calibration values to store in EEPROM
struct Settings {
  long open_pose;
  long closed_pose;
  long half_pose;
  long desired_temp;
};

// Set the default settings for calibration values
Settings settings = { 0, 0, 0, 60 };

// Function for pressing exit button
void handleExit() {
  myState = menu;
  myCalibrationState = menu_cali;
  menu_display.draw(display);
  delay(BUTTON_BOUNCE);
}

// Function for moving to half open position
void moveToHalf() {
  while (abs(getEncoderPos() - settings.half_pose) > MOTOR_TOLERANCE) {
    while (abs(getEncoderPos() - settings.half_pose) > MOTOR_TOLERANCE) {
      motor.moveToward(settings.half_pose,MOTOR_TOLERANCE);
    }
    motor.stopMoving();
    delay(10);
  }
}

// Function for moving to open position
void moveToOpen() {
  while (abs(getEncoderPos() - settings.open_pose) > MOTOR_TOLERANCE) {
    while (abs(getEncoderPos() - settings.open_pose) > MOTOR_TOLERANCE) {
      motor.moveToward(settings.open_pose,MOTOR_TOLERANCE);
    }
    motor.stopMoving();
    delay(10);
  }
}

void moveToZero() {
  while (abs(getEncoderPos() - 0) > MOTOR_TOLERANCE) {
    while (abs(getEncoderPos() - 0) > MOTOR_TOLERANCE) {
      motor.moveToward(0, MOTOR_TOLERANCE);
    }
    motor.stopMoving();
    delay(10);
  }
}

// Function for moving to closed position
void moveToClosed() {
  while (abs(getEncoderPos() - settings.closed_pose) > MOTOR_TOLERANCE) {
    while (abs(getEncoderPos() - settings.closed_pose) > MOTOR_TOLERANCE) {
      motor.moveToward(settings.closed_pose,MOTOR_TOLERANCE);
    }
    motor.stopMoving();
    delay(10);
  }
}

// Case for manually moving the motor up, down, or not moving
void moveMotorControl() {
  while (true) {
    delay(BUTTON_BOUNCE);
    if (digitalRead(upButtonPin) == LOW) {
      motor.moveUp();
    }
    if (digitalRead(downButtonPin) == LOW) {
      motor.moveDown();
    }
    if (digitalRead(upButtonPin) == HIGH && digitalRead(downButtonPin) == HIGH) {
      motor.stopMoving();
    }
    if (digitalRead(exitButtonPin) == LOW || digitalRead(selectButtonPin) == LOW) {
      handleExit();
      break;
    }
  }
}

// Function for scrolling on the OLED display
void handleDisplay(MenuDisplay* menu_dis, Control option) {
  switch (option) {
    case up:
      menu_dis->moveCursorUp();
      break;
    case down:
      menu_dis->moveCursorDown();
      break;
  }
}

// Initialize input pins, set up light sensor and motor, and enable the encoder
void setup() {
  pinMode(upButtonPin, INPUT_PULLUP);
  pinMode(downButtonPin, INPUT_PULLUP);
  pinMode(exitButtonPin, INPUT_PULLUP);
  pinMode(selectButtonPin, INPUT_PULLUP);

  lightSensor.setupLTR();
  motor.begin();

  display.begin(SSD1306_SWITCHCAPVCC);
  display.setTextSize(1);
  display.setTextColor(WHITE);

  enableEncoderInterrupts(encoderUpPin, encoderDownPin);
}

// Start the main loop
void loop() {
  bool higherThanTemp; // True when current temperature is higher than desired temperature
  bool lowerThanTemp; // True when current temperature is lower than desired temperature
  bool higherThanLight; // True when current luminosity is higher than desired luminosity
  bool lowerThanLight; // True when current luminosity is lower than desired luminosity

  // Back button handling
  if (digitalRead(exitButtonPin) == LOW) {
    handleExit();
  }

  // Main state machine
  switch (myState) {
    case initializing:
      //loading settings
      myState = menu;
      menu_display.draw(display);
      EEPROM.get(EEADDRESS, settings);

      break;

    // Main menu
    case menu:
      if (digitalRead(exitButtonPin) == LOW) {
        moveToZero();
      }
      if (digitalRead(upButtonPin) == LOW) { // scroll up
        handleDisplay(&menu_display, up);
        menu_display.draw(display);
        delay(BUTTON_BOUNCE);
      }
      if (digitalRead(downButtonPin) == LOW) { // scroll down
        handleDisplay(&menu_display, down);
        menu_display.draw(display);
        delay(BUTTON_BOUNCE);
      }
      if (digitalRead(selectButtonPin) == LOW) { // enter button is pressed
        int pos = menu_display.getCursorPos();
        switch (pos) {
          case 0:
            myState = calibrate; // go to calibration mode
            menu_display.resetCursorPos();
            calibration_display.resetCursorPos();
            break;
          case 1:
            myState = automatic; // go to automatic mode
            menu_display.resetCursorPos();
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("In automatic mode");
            display.display();
            break;
          case 2:
            myState = manual; // go to manual mode
            menu_display.resetCursorPos();
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("In manual mode");
            display.display();
            break;
        }
        delay(BUTTON_BOUNCE);
      }
      break;

    // Begin Calibration Menu
    case calibrate:
      calibration_display.draw(display);
      switch (myCalibrationState) { // which thing are we calibrating
        case menu_cali:
          if (digitalRead(upButtonPin) == LOW) { // scroll up
            handleDisplay(&calibration_display, up);
            calibration_display.draw(display);
            delay(BUTTON_BOUNCE);
          }
          if (digitalRead(downButtonPin) == LOW) { // scroll down
            handleDisplay(&calibration_display, down);
            calibration_display.draw(display);
            delay(BUTTON_BOUNCE);
          }
          if (digitalRead(selectButtonPin) == LOW) { // enter button is pressed
            int pos = calibration_display.getCursorPos();
            switch (pos) {
              case 0:
                myCalibrationState = open_position; // calibrate open position
                menu_display.resetCursorPos();
                calibration_display.resetCursorPos();
                break;
              case 1:
                myCalibrationState = close_position; // calibrate closed position
                menu_display.resetCursorPos();
                calibration_display.resetCursorPos();
                break;
              case 2:
                myCalibrationState = half_position; // calibrate half-open position
                menu_display.resetCursorPos();
                calibration_display.resetCursorPos();
                break;
              case 3:
                myCalibrationState = temperature_cali; // calibrate temperature
                menu_display.resetCursorPos();
                calibration_display.resetCursorPos();
                break;
            }
            delay(BUTTON_BOUNCE);
          }
          if (digitalRead(exitButtonPin) == LOW) { // back button is pressed
            handleExit();
          }
          break;

        // Calibrate the open position
        case open_position:
          display.clearDisplay();
          display.setCursor(0, 0);
          display.println("Calibrating open\nposition");
          display.display();

          moveToOpen();
          moveMotorControl();
          motor.openPos = getEncoderPos(); // after the motor has been moved, save the open position
          settings.open_pose = motor.openPos;
          EEPROM.put(EEADDRESS, settings);
          handleExit();
          break;

        // Calibrate the closed position
        case close_position:
          display.clearDisplay();
          display.setCursor(0, 0);
          display.println("Calibrating closed\nposition");
          display.display();

          moveToClosed();
          moveMotorControl();
          motor.closedPos = getEncoderPos(); // after the motor has been moved, save the closed position
          settings.closed_pose = motor.closedPos;
          EEPROM.put(EEADDRESS, settings);
          handleExit();
          break;

        // Calibrate the half-open position
        case half_position:
          display.clearDisplay();
          display.setCursor(0, 0);
          display.println("Calibrating half openposition");
          display.display();

          moveToHalf();
          moveMotorControl();
          motor.halfPos = getEncoderPos();
          settings.half_pose = motor.halfPos; // after the motor has been moved, save the half position
          if (digitalRead(exitButtonPin) == LOW) {
            handleExit();
          }
          break;

        // Calibrate the temperature
        case temperature_cali:
          while (true) {
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println(settings.desired_temp);
            display.display();
  
            if (digitalRead(upButtonPin) == LOW) { // increase the desired temperature on 'up' button
              settings.desired_temp++;
              delay(BUTTON_BOUNCE);
            }
            if (digitalRead(downButtonPin) == LOW) { // increase the desired temperature on 'down' button
              settings.desired_temp--;
              delay(BUTTON_BOUNCE);
            }
            if (digitalRead(exitButtonPin) == LOW || digitalRead(selectButtonPin) == LOW) {
              EEPROM.put(EEADDRESS, settings);
              handleExit();
              break;
            }
          }

          break;
      }
      if (digitalRead(exitButtonPin) == LOW) {
        handleExit();
      }
      break;

    // Enter the automatic mode 
    case automatic:
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("In automatic mode");
      display.display();
      
      // Evaluate each of the previously defined booleans based on sensor data
      higherThanTemp = tempSensor.getTemp() > settings.desired_temp + AUTO_TOLERANCE_TEMP;
      lowerThanTemp = tempSensor.getTemp() < settings.desired_temp - AUTO_TOLERANCE_TEMP;
      higherThanLight = lightSensor.getLux() > LIGHT_THRES + AUTO_TOLERANCE_LIGHT;
      lowerThanLight = lightSensor.getLux() < LIGHT_THRES - AUTO_TOLERANCE_LIGHT;
      if (higherThanTemp && higherThanLight) { // if it's too hot and there's light, close the blinds to cool down
        motor.moveTowardClosed(MOTOR_TOLERANCE);
      } else if (lowerThanTemp && higherThanLight) { // if it's too cold and there's light, open the blinds to warm up
        motor.moveTowardOpen(MOTOR_TOLERANCE);
      } else if (lowerThanLight) { // if there's no light, open the blinds anyways to get a view outside
        motor.moveTowardOpen(MOTOR_TOLERANCE);
      } else { // just maintain the current status by keeping the blinds half open
        motor.moveTowardHalf(MOTOR_TOLERANCE);
      }
      if (digitalRead(exitButtonPin) == LOW) { // exit if exit button is pressed
        handleExit();
      }
      break;
    
    // Enter the manual mode
    case manual:
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("In manual mode");
      display.display();
      
      if (digitalRead(upButtonPin) == HIGH && digitalRead(downButtonPin) == HIGH) { // if no buttons are pressed, don't move
        motor.stopMoving();
      }
      if (digitalRead(upButtonPin) == LOW) { // move up if up button is pressed
        motor.moveUp();
        delay(BUTTON_BOUNCE);
      }
      if (digitalRead(downButtonPin) == LOW) { // move down if down button is pressed
        motor.moveDown();
        delay(BUTTON_BOUNCE);
      }
      if (digitalRead(exitButtonPin) == LOW) { // exit
        handleExit();
      }
      break;
  }
}
