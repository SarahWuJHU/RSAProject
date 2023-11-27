#include "Encoder.h"

static void upIsr();
static void downIsr();

static volatile long pos = 0;

static int upPin;
static int downPin;

long getEncoderPos() {
  return pos;
}

void enableEncoderInterrupts(int upChannelPin, int downChannelPin) {
  upPin = upChannelPin;
  downPin = downChannelPin;
  
  attachInterrupt(digitalPinToInterrupt(upChannelPin), upIsr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(downChannelPin), downIsr, CHANGE);
}

static void upIsr() {
  int upChannel = digitalRead(upPin);
  int downChannel = digitalRead(downPin);

  if (upChannel == downChannel)
    pos++;
  else
    pos--;
}

static void downIsr() {
  int upChannel = digitalRead(upPin);
  int downChannel = digitalRead(downPin);

  if (upChannel != downChannel)
    pos++;
  else
    pos--;
}
