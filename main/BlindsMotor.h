#ifndef BLINDS_MOTOR_INC
#define BLINDS_MOTOR_INC

class BlindsMotor {
public:
  long closedPos = 0;
  long halfPos = 0;
  long openPos = 0;

  BlindsMotor(int upPin, int downPin, int pulsePin);

  void begin();
  
  void moveUp() const;
  void moveDown() const;
  void stopMoving() const;

  void moveToward(long pos, int tolerance) const;

  void moveTowardClosed(int tolerance);
  void moveTowardHalf(int tolerance);
  void moveTowardOpen(int tolerance);

private:
  int upPin;
  int downPin;
  int pulsePin;
};

#endif
