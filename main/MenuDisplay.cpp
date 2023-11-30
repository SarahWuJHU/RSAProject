#include "MenuDisplay.h"

void MenuDisplay::moveCursorUp() {
  if (cursorPos > 0)
    cursorPos--;
  while (cursorPos < scrollPos)
    scrollPos--;
}

void MenuDisplay::moveCursorDown() {
  if (cursorPos < numOptions - 1)
    cursorPos++;
  while (cursorPos >= scrollPos + 3)
    scrollPos++;
}

void MenuDisplay::resetCursorPos() {
  cursorPos = 0;
  scrollPos = 0;
}

void MenuDisplay::draw(Adafruit_SSD1306& oled) {
  oled.clearDisplay();
  oled.setCursor(0, 0);
  oled.setTextSize(1);
  
  oled.println(title);
  for (int i = scrollPos; i < min(scrollPos + 3, numOptions); i++) {
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
  : title(title), optionNames(optionNames), numOptions(numOptions), cursorPos(0), scrollPos(0) {}
