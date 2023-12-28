#include <LiquidCrystal_I2C.h>

#include "C:\customLibraries\cronos\cronos.h"
#include "C:\customLibraries\timer\timer.h"
#include "C:\customLibraries\cronos\stepperControl.h"

#define S1_DIR_PIN 15
#define S1_STEP_PIN 16
#define S1_EN_PIN 17

#define S2_DIR_PIN 5
#define S2_STEP_PIN 18
#define S2_EN_PIN 19

#define OUT_A 33
#define OUT_B 32
#define SW 23
#define LIMIT_PIN 39

//* LCD
int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

void setup()
{
  Serial.begin(115200);

  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);

  pinMode(OUT_A, INPUT);
  pinMode(OUT_B, INPUT);
  pinMode(SW, INPUT);

  turnOn();
  setDir(HIGH);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.clear();
}

void loop()
{
}

/*
void setup()
{
  Serial.begin(115200);

  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);

  pinMode(OUT_A, INPUT);
  pinMode(OUT_B, INPUT);
  pinMode(SW, INPUT);

  turnOn();
  setDir(HIGH);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.clear();
}



void makeStep(int delayMicros)
{
  digitalWrite(STEP_PIN, HIGH);
  delayMicroseconds(delayMicros);
  digitalWrite(STEP_PIN, LOW);
  delayMicroseconds(delayMicros);
}
void turnOn()
{
  digitalWrite(EN_PIN, LOW);
}
void setDir(bool newDir)
{
  digitalWrite(DIR_PIN, newDir);
}
void loop()
{
  Serial.println(digitalRead(SW));
  Serial.println(digitalRead(OUT_A));
  Serial.println(digitalRead(OUT_B));
  Serial.println(" ");
  
  lcd.setCursor(0, 0);
  lcd.print("jobbra forog");
  for (int i = 0; i < 1000; i++)
  {
    makeStep(1000);
  }
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ballra forog");
  setDir(LOW);
  for (int i = 0; i < 1000; i++)
  {
    makeStep(1000);
  }
  delay(1000);
  lcd.clear();
  setDir(HIGH);
}

*/