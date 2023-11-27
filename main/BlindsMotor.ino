#include "BlindsMotor.h"
#include "Encoder.h"

BlindsMotor::BlindsMotor(int upPin, int downPin)
  : upPin(upPin), downPin(downPin) {}

void BlindsMotor::moveUp() const {
  digitalWrite(downPin, LOW);
  digitalWrite(upPin, HIGH);
}

void BlindsMotor::moveDown() const {
  digitalWrite(upPin, LOW);
  digitalWrite(downPin, HIGH);
}

void BlindsMotor::stopMoving() const {
  digitalWrite(upPin, LOW);
  digitalWrite(downPin, LOW);
}

void BlindsMotor::moveToward(int pos, int tollerance) const {
  int current_pos = getEncoderPos();
  
  if (current_pos < pos - tollerance)
    moveUp();
  else if (getEncoderPos() > pos + tollerance)
    moveDown();
  else
    stopMoving();
}
