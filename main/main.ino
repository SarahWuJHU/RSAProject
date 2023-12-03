#include "LightSense.h"
#include "MenuDisplay.h"
#include "BlindsMotor.h"
#include "TempSense.h"
#include "Encoder.h"

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

// other settings for automatic control
#define MOTOR_TOLERANCE 1
#define LIGHT_THRES 300
#define AUTO_TOLERANCE_TEMP 10
#define AUTO_TOLERANCE_LIGHT 50

enum States {
  initializing,
  menu,
  calibrate,
  automatic,
  manual
};
enum Control {
  up,
  down
};
enum calibrationStates {
  menu_cali,
  open_position,
  close_position,
  half_position,
  temperature_cali
};

//Setting up displays
char calibrate_word[] = "calibrate";
char automatic_word[] = "automatic";
char manual_word[] = "manual";
char open_position_word[] = "open_position";
char close_position_word[] = "close_position";
char half_position_word[] = "half_position";
char temperature_word[] = "temperature";
const char* menu_items[] = { calibrate_word, automatic_word, manual_word };
const char* calibrate_items[] = { open_position_word, close_position_word, half_position_word, temperature_word };
static MenuDisplay menu_display("Menu", menu_items, 3);
static MenuDisplay calibration_display("Calibration", calibrate_items, 4);

//MenuDisplays Menu, Calibrate;
const int upButtonPin = 10;
const int downButtonPin = 11;
const int exitButtonPin = 12;
const int selectButtonPin = 13;
const int motorUpPin = 0;
const int motorDownPin = 0;
const int motorPulsePin = 0;
const int tempSensorPin = 0;

static BlindsMotor motor(motorUpPin, motorDownPin, motorPulsePin);
static LightSense lightSensor;
static TempSense tempSensor(tempSensorPin);
Adafruit_SSD1306 display(OLED_PICO, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
static States myState = initializing;
static calibrationStates myCalibrationState = menu_cali;

//store in ee prom
struct Settings {
  long open_pose;
  long closed_pose;
  long half_pose;
  long desired_temp;
};

Settings settings = { 0, 0, 0, 60};


void handleExit() {
  myState = menu;
  myCalibrationState = menu_cali;
}

void moveToHalf() {
  while (abs(getEncoderPos() - settings.half_pose) < MOTOR_TOLERANCE) {
    motor.moveTowardHalf(MOTOR_TOLERANCE);
  }
  motor.stopMoving();
}

void moveToOpen() {
  while (abs(getEncoderPos() - settings.open_pose) < MOTOR_TOLERANCE) {
    motor.moveTowardOpen(MOTOR_TOLERANCE);
  }
  motor.stopMoving();
}

void moveToClosed() {
  while (abs(getEncoderPos() - settings.closed_pose) < MOTOR_TOLERANCE) {
    motor.moveTowardClosed(MOTOR_TOLERANCE);
  }
  motor.stopMoving();
}

void moveMotorControl() {
  while (digitalRead(selectButtonPin) == HIGH) {
    if (digitalRead(upButtonPin) == LOW) {
      motor.moveUp();
    }
    if (digitalRead(downButtonPin) == LOW) {
      motor.moveDown();
    }
    if (digitalRead(upButtonPin) == HIGH && digitalRead(downButtonPin) == HIGH) {
      motor.stopMoving();
    }
  }
}

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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(upButtonPin, INPUT_PULLUP);
  pinMode(downButtonPin, INPUT_PULLUP);
  pinMode(exitButtonPin, INPUT_PULLUP);
  pinMode(selectButtonPin, INPUT_PULLUP);

  lightSensor.setupLTR();
}

void loop() {
  bool higherThanTemp;
  bool lowerThanTemp;
  bool higherThanLight;
  bool lowerThanLight;
  
  // put your main code here, to run repeatedly:
  // escape button logic
  if (digitalRead(exitButtonPin) == LOW) {
    handleExit();
  }
  switch (myState) {
    case initializing:
      //loading settings
      myState = menu;
      break;
    case menu:
      menu_display.draw(display);
      if (digitalRead(exitButtonPin) == LOW) {
        handleExit();
      }
      if (digitalRead(upButtonPin) == LOW) {
        handleDisplay(&menu_display, up);
        menu_display.draw(display);
      }
      if (digitalRead(downButtonPin) == LOW) {
        handleDisplay(&menu_display, down);
        menu_display.draw(display);
      }
      if (digitalRead(selectButtonPin) == LOW) {
        int pos = menu_display.getCursorPos();
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

      break;
    case calibrate:
      // calibration start
      // open state, close state, half open
      // desired temperature
      calibration_display.draw(display);
      switch (myCalibrationState) {
        case menu_cali:
          if (digitalRead(upButtonPin) == LOW) {
            handleDisplay(&calibration_display, up);
            calibration_display.draw(display);
          }
          if (digitalRead(downButtonPin) == LOW) {
            handleDisplay(&calibration_display, down);
            calibration_display.draw(display);
          }
          if (digitalRead(selectButtonPin) == LOW) {
            int pos = calibration_display.getCursorPos();
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
                myCalibrationState = temperature_cali;
                menu_display.resetCursorPos();
                break;
            }
          }
          if (digitalRead(exitButtonPin) == LOW) {
            handleExit();
          }
          break;
        case open_position:
          moveToOpen();
          moveMotorControl();
          motor.openPos = getEncoderPos();
          settings.open_pose = motor.openPos;
          if (digitalRead(exitButtonPin) == LOW) {
            handleExit();
          }
          break;
        case close_position:
          moveToClosed();
          moveMotorControl();
          motor.closedPos = getEncoderPos();
          settings.closed_pose = motor.closedPos;
          if (digitalRead(exitButtonPin) == LOW) {
            handleExit();
          }
          break;
        case half_position:
          moveToHalf();
          moveMotorControl();
          motor.halfPos = getEncoderPos();
          settings.half_pose = motor.halfPos;
          if (digitalRead(exitButtonPin) == LOW) {
            handleExit();
          }
          break;
        case temperature_cali:
          display.clearDisplay();
          display.println(settings.desired_temp);
          while (digitalRead(selectButtonPin) == HIGH) {
            if (digitalRead(upButtonPin) == LOW) {
              settings.desired_temp++;
              display.clearDisplay();
              display.println(settings.desired_temp);
            }
            if (digitalRead(downButtonPin) == LOW) {
              settings.desired_temp--;
              display.clearDisplay();
              display.println(settings.desired_temp);
            }
          }
          if (digitalRead(exitButtonPin) == LOW) {
            handleExit();
          }
          break;
      }
      if (digitalRead(exitButtonPin) == LOW) {
        handleExit();
      }
      break;
    case automatic:
      //sensor
      higherThanTemp = tempSensor.getTemp() > settings.desired_temp + AUTO_TOLERANCE_TEMP;
      lowerThanTemp = tempSensor.getTemp() < settings.desired_temp - AUTO_TOLERANCE_TEMP;
      higherThanLight = lightSensor.getLux() > LIGHT_THRES + AUTO_TOLERANCE_LIGHT;
      lowerThanLight = lightSensor.getLux() < LIGHT_THRES - AUTO_TOLERANCE_LIGHT;
      if (higherThanTemp && higherThanLight) {
        motor.moveTowardClosed(MOTOR_TOLERANCE);
      } else if (lowerThanTemp && higherThanLight) {
        motor.moveTowardOpen(MOTOR_TOLERANCE);
      } else if (lowerThanLight) {
        motor.moveTowardOpen(MOTOR_TOLERANCE);
      } else {
        motor.moveTowardHalf(MOTOR_TOLERANCE);
      }
      if (digitalRead(exitButtonPin) == LOW) {
        handleExit();
      }
      break;
    case manual:
      if (digitalRead(upButtonPin) == HIGH && digitalRead(downButtonPin) == HIGH) {
        motor.stopMoving();
      }
      if (digitalRead(upButtonPin) == LOW) {
        motor.moveUp();
      }
      if (digitalRead(downButtonPin) == LOW) {
        motor.moveDown();
      }
      if (digitalRead(exitButtonPin) == LOW) {
        handleExit();
      }
      break;
  }
}
