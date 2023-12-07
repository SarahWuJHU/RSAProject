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

// other settings for automatic control
#define MOTOR_TOLERANCE 10
#define LIGHT_THRES 400
#define AUTO_TOLERANCE_TEMP 10
#define AUTO_TOLERANCE_LIGHT 50
#define BUTTON_BOUNCE 100

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
static MenuDisplay menu_display("   MENU", menu_items, 3);
static MenuDisplay calibration_display("    CALIBRATION", calibrate_items, 4);

//MenuDisplays Menu, Calibrate;
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

Settings settings = { 0, 0, 0, 60 };


void handleExit() {
  myState = menu;
  myCalibrationState = menu_cali;
  menu_display.draw(display);
  delay(BUTTON_BOUNCE);
}

void moveToHalf() {
  while (abs(getEncoderPos() - settings.half_pose) > MOTOR_TOLERANCE) {
    while (abs(getEncoderPos() - settings.half_pose) > MOTOR_TOLERANCE) {
      motor.moveToward(settings.half_pose,MOTOR_TOLERANCE);
    }
    motor.stopMoving();
    delay(10);
  }
}

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

void moveToClosed() {
  while (abs(getEncoderPos() - settings.closed_pose) > MOTOR_TOLERANCE) {
    while (abs(getEncoderPos() - settings.closed_pose) > MOTOR_TOLERANCE) {
      motor.moveToward(settings.closed_pose,MOTOR_TOLERANCE);
    }
    motor.stopMoving();
    delay(10);
  }
}

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

void loop() {
  bool higherThanTemp;
  bool lowerThanTemp;
  bool higherThanLight;
  bool lowerThanLight;

  // put your main code here, to run repeatedly:
  // escape button logic
  switch (myState) {
    case initializing:
      //loading settings
      myState = menu;
      menu_display.draw(display);
      EEPROM.get(EEADDRESS, settings);

      break;
    case menu:
      if (digitalRead(exitButtonPin) == LOW) {
        moveToZero();
      }
      if (digitalRead(upButtonPin) == LOW) {
        handleDisplay(&menu_display, up);
        menu_display.draw(display);
        delay(BUTTON_BOUNCE);
      }
      if (digitalRead(downButtonPin) == LOW) {
        handleDisplay(&menu_display, down);
        menu_display.draw(display);
        delay(BUTTON_BOUNCE);
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
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("In automatic mode");
            display.display();
            break;
          case 2:
            myState = manual;
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
            delay(BUTTON_BOUNCE);
          }
          if (digitalRead(downButtonPin) == LOW) {
            handleDisplay(&calibration_display, down);
            calibration_display.draw(display);
            delay(BUTTON_BOUNCE);
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
                calibration_display.resetCursorPos();
                break;
              case 2:
                myCalibrationState = half_position;
                menu_display.resetCursorPos();
                calibration_display.resetCursorPos();
                break;
              case 3:
                myCalibrationState = temperature_cali;
                menu_display.resetCursorPos();
                calibration_display.resetCursorPos();
                break;
            }
            delay(BUTTON_BOUNCE);
          }
          if (digitalRead(exitButtonPin) == LOW) {
            handleExit();
          }
          break;
        case open_position:
          display.clearDisplay();
          display.setCursor(0, 0);
          display.println("Calibrating open\nposition");
          display.display();

          moveToOpen();
          moveMotorControl();
          motor.openPos = getEncoderPos();
          settings.open_pose = motor.openPos;
          EEPROM.put(EEADDRESS, settings);
          handleExit();
          break;
        case close_position:
          display.clearDisplay();
          display.setCursor(0, 0);
          display.println("Calibrating closed\nposition");
          display.display();

          moveToClosed();
          moveMotorControl();
          motor.closedPos = getEncoderPos();
          settings.closed_pose = motor.closedPos;
          EEPROM.put(EEADDRESS, settings);
          handleExit();
          break;
        case half_position:
          display.clearDisplay();
          display.setCursor(0, 0);
          display.println("Calibrating half openposition");
          display.display();

          moveToHalf();
          moveMotorControl();
          motor.halfPos = getEncoderPos();
          settings.half_pose = motor.halfPos;
          EEPROM.put(EEADDRESS, settings);
          handleExit();
          break;
        case temperature_cali:
          while (true) {
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println(settings.desired_temp);
            display.display();

            if (digitalRead(upButtonPin) == LOW) {
              settings.desired_temp++;
              delay(BUTTON_BOUNCE);
            }
            if (digitalRead(downButtonPin) == LOW) {
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
        delay(BUTTON_BOUNCE);
      }
      if (digitalRead(downButtonPin) == LOW) {
        motor.moveDown();
        delay(BUTTON_BOUNCE);
      }
      if (digitalRead(exitButtonPin) == LOW) {
        handleExit();
      }
      break;
  }
}
