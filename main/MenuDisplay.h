#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

class MenuDisplay {
private:
  const char* title;
  const char** optionNames;
  int numOptions;
  
  int cursorPos;
  int scrollPos;

public:
  MenuDisplay(const char* title, const char** optionNames, int numOptions);

  void moveCursorUp();
  void moveCursorDown();
  void resetCursorPos();
  void draw(Adafruit_SSD1306& oled);
  int getCursorPos() const;
};
