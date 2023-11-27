class BlindsMotor {
public:
  BlindsMotor(int upPin, int downPin);
  
  void moveUp() const;
  void moveDown() const;
  void stopMoving() const;

  void moveToward(int pos, int tolerance) const;

private:
  int upPin;
  int downPin;
};
