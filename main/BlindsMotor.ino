#include "BlindsMotor.h"
#include "Encoder.h"

#define MOTOR_K 0.1f

BlindsMotor::BlindsMotor(int upPin, int downPin, int pulsePin)
  : upPin(upPin), downPin(downPin), pulsePin {}

void BlindsMotor::moveUp() const {
  digitalWrite(downPin, LOW);
  digitalWrite(upPin, HIGH);
  digitalWrite(pulsePin, HIGH);
}

void BlindsMotor::moveDown() const {
  digitalWrite(upPin, LOW);
  digitalWrite(downPin, HIGH);
  digitalWrite(pulsePin, HIGH);
}

void BlindsMotor::stopMoving() const {
  digitalWrite(upPin, LOW);
  digitalWrite(downPin, LOW);
  digitalWrite(pulsePin, LOW);
}

void BlindsMotor::moveToward(long pos, int tolerance) const {
  long currentPos = getEncoderPos();
  
  if (currentPos < pos - tolerance)
    moveUp();
  else if (currentPos > pos + tolerance)
    moveDown();
  else
    stopMoving();

  long diff = abs(pos - currentPos);
  int power = 255 * (int)max(1.0, MOTOR_K * diff);
  analogWrite(pulsePin, power);
}

void BlindsMotor::moveTowardClosed(int tolerance) {
  moveToward(closedPos, tolerance);
}

void BlindsMotor::moveTowardHalf(int tolerance) {
  moveToward(halfPos, tolerance);
}

void BlindsMotor::moveTowardOpen(int tolerance) {
  moveToward(openPos, tolerance);
}
