#include <Wire.h>
#include <hd44780.h>                        // main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h>  // i2c expander i/o class header

#define outputA 34
#define outputB 35
#define SW 26

hd44780_I2Cexp lcd;  // declare lcd object: auto locate & auto config expander chip

byte pijl[] = {
  B00000,
  B00100,
  B00110,
  B11111,
  B00110,
  B00100,
  B00000,
  B00000
};

// LCD geometry
const int LCD_COLS = 20;
const int LCD_ROWS = 4;

// encoder
int counter = 0;
int currentStateA;
int lastStateA;

// button
int buttonState = 0;
int lastButtonState = 0;

// Globals
int pulsenPerLiter = 630;
int menuStatus = 0;
int inwateren = 20;

void setup() {

  // Set encoder pins as inputs
  pinMode(outputA, INPUT);
  pinMode(outputB, INPUT);
  pinMode(SW, INPUT_PULLUP);
  // Setup Serial Monitor
  Serial.begin(9600);
  int status;

  status = lcd.begin(LCD_COLS, LCD_ROWS);
  if (status)  // non zero status means it was unsuccesful
  {
    hd44780::fatalError(status);  // does not return
  }
  // create cutom char
  lcd.createChar(0, pijl);
  // set up the encoder
  lastStateA = digitalRead(outputA);
  handleMenu();
}

void loop() {
  buttonState = digitalRead(SW);
  if (buttonState != lastButtonState) {
    if (buttonState == LOW) {
      handleButton();
    }
    lastButtonState = buttonState;
  }
  if (menuStatus != 201) {
    currentStateA = digitalRead(outputA);
    if (currentStateA != lastStateA && currentStateA == 1) {
      if (digitalRead(outputB) != currentStateA) {
        counter++;
        if (counter > 3) {
          counter = 1;
        }
      } else {
        // Encoder is rotating CW so increment
        counter--;
        if (counter < 1) {
          counter = 3;
        }
      }
      Serial.print("Counter: ");
      Serial.println(counter);
      Serial.print("Menustatus: ");
      Serial.println(menuStatus);
      updateCursor();
    }
    lastStateA = currentStateA;
  }

  else {
    currentStateA = digitalRead(outputA);
    if (currentStateA != lastStateA && currentStateA == 1) {
      if (digitalRead(outputB) != currentStateA) {
        pulsenPerLiter++;

      } else {
        // Encoder is rotating CW so increment
        pulsenPerLiter--;
      }
      Serial.print("Pulsen: ");
      Serial.println(pulsenPerLiter);
      Serial.print("Menustatus: ");
      Serial.println(menuStatus);
      lcd.setCursor(7, 1);
      lcd.print(pulsenPerLiter);
    }
    lastStateA = currentStateA;
  }
  // Put in a slight delay to help debounce the reading
  delay(10);
}

void handleMenu() {
  if (menuStatus == 0) {
    hoofdMenu();
  }
  if (menuStatus == 10) {
    startMenu();
  }
  if (menuStatus == 20) {
    settingMenu();
  }
  if (menuStatus == 30) {
    resetMenu();
  }
  if (menuStatus == 100) {
    trayMenu();
  }
  if (menuStatus == 200) {
    setPulsen();
  }
  if (menuStatus == 201) {
    pulsen();
  }
}

void handleButton() {
  Serial.println("Button pressed");
  if (menuStatus == 0 && counter == 1) {
    menuStatus = 10;
  } else if (menuStatus == 0 && counter == 2) {
    menuStatus = 20;
  } else if (menuStatus == 0 && counter == 3) {
    menuStatus = 30;
  } else if (menuStatus == 10 && counter == 3) {
    menuStatus = 0;
  } else if (menuStatus == 20 && counter == 3) {
    menuStatus = 0;
  } else if (menuStatus == 20 && counter == 1) {
    menuStatus = 100;
  } else if (menuStatus == 30 && counter == 3) {
    menuStatus = 0;
  } else if (menuStatus == 20 && counter == 2) {
    menuStatus = 200;
  } else if (menuStatus == 200 && counter == 2) {
    menuStatus = 201;
  } else if (menuStatus == 201) {
    menuStatus = 200;
  } else if (menuStatus == 100 && counter == 3) {
    menuStatus = 20;
  } else if (menuStatus == 200 && counter == 3) {
    menuStatus = 20;
  } else {
    menuStatus = 0;
  }
  handleMenu();
}

void updateCursor() {
  lcd.setCursor(0, 0);
  lcd.print(" ");
  lcd.setCursor(0, 1);
  lcd.print(" ");
  lcd.setCursor(0, 2);
  lcd.print(" ");
  lcd.setCursor(0, 3);
  lcd.print(" ");
  lcd.setCursor(0, counter);
  lcd.write(byte(0));
}

void hoofdMenu() {  // menuStatus 0
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("=== HOOFDMENU ====");
  lcd.setCursor(0, 1);
  lcd.write(byte(0));
  lcd.setCursor(1, 1);
  lcd.print("Starten");
  lcd.setCursor(1, 2);
  lcd.print("Instellingen");
  lcd.setCursor(1, 3);
  lcd.print("Resetten");
}

void startMenu() {  // menustatus 10
  // Hier start code
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("START MENU");
  lcd.setCursor(1, 3);
  lcd.print("Terug");
}

void settingMenu() {  // menustatus 20
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("SETTINGS MENU");
  lcd.setCursor(1, 1);
  lcd.print("Tray instellingen");
  lcd.setCursor(1, 2);
  lcd.print("Pulsen per liter");
  lcd.setCursor(1, 3);
  lcd.print("Terug");
}

void setPulsen() {  // menuStatus 200
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Pulsen per liter");
  lcd.setCursor(7, 2);
  lcd.print(pulsenPerLiter);
  lcd.setCursor(1, 3);
  lcd.print("Terug");
}

void pulsen() {  // menuStatus 201
  lcd.clear();
  lcd.setCursor(6, 1);
  lcd.print(">");
  lcd.print(pulsenPerLiter);
  lcd.print("<");
  lcd.setCursor(3, 3);
  lcd.print("Klik voor OK");
}

void trayMenu() {  // menustatus 100
  // Hier tray code
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("TRAY MENU");
  lcd.setCursor(1, 1);
  lcd.print("Tray selecteren");
  lcd.setCursor(1, 2);
  lcd.print("Tray settings");
  lcd.setCursor(1, 3);
  lcd.print("Terug");
}

void traySelectMenu() {  // menusatus 101
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("TRAY SELECTEREN");
  lcd.setCursor(1, 1);
  lcd.print("-");
  lcd.setCursor(1, 2);
  lcd.print("-");
  lcd.setCursor(1, 3);
  lcd.print("Terug");
}

void traySettingsMenu() {  // menusTatus 102
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("TRAY SETTINGS");
  lcd.setCursor(1, 0);
  lcd.print("Inwateren: ");
  lcd.setCursor(1, 0);
  lcd.print("Volume: ");
  lcd.setCursor(1, 3);
  lcd.print("Terug");
}
void resetMenu() {  // menustatus 30
  // Hier start code
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("RESET MENU");
  lcd.setCursor(1, 3);
  lcd.print("Terug");
}