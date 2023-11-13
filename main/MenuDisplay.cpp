#include "MenuDisplay.h"

void MenuDisplay::moveCursorUp() {
  if (cursorPos > 0)
    cursorPos--;
}

void MenuDisplay::moveCursorDown() {
  if (cursorPos < numOptions - 1)
    cursorPos++;
}

void MenuDisplay::resetCursorPos() {
  cursorPos = 0;
}

void MenuDisplay::draw(Adafruit_SSD1306& oled) {
  oled.clearDisplay();
  oled.setCursor(0, 0);

  oled.setTextSize(2);
  oled.println(title);

  oled.setTextSize(1);
  for (int i = 0; i < numOptions; i++) {
    if (i == cursorPos)
      oled.print("*");
    oled.print(optionNames[i]);
    if (i == cursorPos)
      oled.print("*");
    oled.println("");
  }

  oled.display();
}

int MenuDisplay::getCursorPos() const {
  return cursorPos;
}

MenuDisplay::MenuDisplay(const char* title, const char** optionNames, int numOptions)
  : title(title), optionNames(optionNames), numOptions(numOptions), cursorPos(0) {}
