#define homeWifi
//#define workWifi
#include <LiquidCrystal_I2C.h>

#include <WiFi.h>
#include <WiFiUDP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <ArduinoJson.h>
#include <Preferences.h>
//#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Encoder.h>
#include "C:\customLibraries\cronos\cronos.h"
#include "C:\customLibraries\timer\timer.h"
#include "C:\customLibraries\stepperControl\stepperControl_blocking.h"

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
unsigned long refreshTime = 120;
int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);
enum MenuState
{
  EMPTY,
  MAIN_MENU,
  START_WASHING,
  MANUAL_CONTROL,
  SET_TIME,
  WASH,
  MOVE_UP,
  MOVE_DOWN
};

MenuState currentMenu = MAIN_MENU;
MenuState prevMenu = EMPTY;
int encoderPosition = 0;
long washTime = 10; // Default washing time

//* ENCODER
Encoder encoder(OUT_A, OUT_B);
long oldPosition = -999;
int prevMenuSelection = -1;
//* STEPPER
Stepper liftingStepper(S1_STEP_PIN, S1_DIR_PIN, S1_EN_PIN);
Stepper washingStepper(S2_STEP_PIN, S2_DIR_PIN, S2_EN_PIN);
bool liftHomeDone = false;
//* WIFI
const int udpPort = 4210;
#ifdef workWifi
const char *ssid = "dentartwork";
const char *password = "tutititkos444";
IPAddress staticIP(192, 168, 5, 4);
IPAddress subnet(255, 255, 255, 0);
IPAddress broadcast(192, 168, 5, 255);
IPAddress gateway(192, 168, 5, 1);
#endif
#ifdef homeWifi
const char *ssid = "Lorincz";
const char *password = "ccLorincz2020cc";
IPAddress staticIP(192, 168, 22, 4);
IPAddress subnet(255, 255, 255, 0);
IPAddress broadcast(192, 168, 22, 255);
IPAddress gateway(192, 168, 22, 1);
#endif
WiFiUDP udp;
AsyncWebServer server(80);

//? NTP TIME
const char *NTP_SERVER = "ch.pool.ntp.org";
const char *TZ_INFO = "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00";
tm timeinfo;
time_t now;
long unsigned lastNTPtime;
unsigned long lastEntryTime;
TimeStruct nowTime;

//#include "webpage.h"
#include "webInterface.h"

void setup()

{
  Serial.begin(115200);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.clear();
  pinMode(LIMIT_PIN, INPUT);

  washingStepper.begin();
  liftingStepper.begin();
  //? NEWTWROK
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(200);
  }

  //? NTP TIME
  configTime(0, 0, NTP_SERVER);
  setenv("TZ", TZ_INFO, 1);
  getNTPtime(10);
  lastNTPtime = time(&now);
  lastEntryTime = millis();

  //? SERVER
  WiFi.config(staticIP, staticIP, subnet);
  serverOn();
  Serial.println("Connected with this IP: " + String(staticIP));
  AsyncElegantOTA.begin(&server);
  server.begin();
  udp.begin(udpPort);

  washingStepper.off();
  washingStepper.setSpeed(70);
  liftingStepper.off();
  liftingStepper.setSpeed(100);

}

int getEncoderValue()
{
  return -encoder.read() / 4;
}

bool getButton()
{
  static bool pressed = false;

  if (!digitalRead(SW) && !pressed)
  {
    pressed = true;
    delay(200);
    return true;
  }
  else
  {
    pressed = false;
    return false;
  }
}

long getWashTime(int position)
{
  int washTime = 0;
  if (position < 0)
    return washTime;

  static const int initialValue = 0;
  static const int LIMIT_RATE_1 = 30;
  static const int LIMIT_RATE_5 = 5 * 60;
  static const int LIMIT_RATE_30 = 20 * 60;
  static const int LIMIT_RATE_60 = 60 * 60;

  washTime = initialValue;
  for (long i = 0; i < position; ++i)
  {
    if (washTime < LIMIT_RATE_1)
      washTime += 1;
    else if (washTime < LIMIT_RATE_5)
      washTime += 5;
    else if (washTime < LIMIT_RATE_30)
      washTime += 30;
    else if (washTime < LIMIT_RATE_60)
      washTime += 60;
    else
      washTime += 300;
  }

  return washTime;
}
bool getNTPtime(int sec)
{
  uint32_t start = millis();
  do
  {
    time(&now);
    localtime_r(&now, &timeinfo);

    delay(10);
  } while (((millis() - start) <= (1000 * sec)) && (timeinfo.tm_year < (2016 - 1900)));

  if (timeinfo.tm_year <= (2016 - 1900))
    return false;

  char time_output[30];
  strftime(time_output, 30, "%a  %d-%m-%y %T", localtime(&now));

  return true;
}
bool returnFalse()
{
  return false;
}

bool readLimitSwitch()
{
  return digitalRead(LIMIT_PIN);
}

void homeLift()
{
  liftingStepper.on();
  liftingStepper.setSpeed(90);
  liftingStepper.setLeft();
  liftingStepper.runSteps(100000, readLimitSwitch);
  liftingStepper.off();
  liftHomeDone = true;
}

void displayMainMenu()
{
  int menuSelection = abs(encoderPosition) % 2;

  if (prevMenuSelection != menuSelection)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(menuSelection == 0 ? ">WASHING" : " WASHING");
    lcd.setCursor(0, 1);
    lcd.print(menuSelection == 1 ? ">CONTROL" : " CONTROL");
    prevMenuSelection = menuSelection;
  }
}
void handleMainMenuSelection()
{
  int menuSelection = abs(encoderPosition) % 2;
  if (menuSelection == 0)
  {
    currentMenu = START_WASHING;
    Serial.println("WASHING MENU");
    resetMenuSelection();
  }
  else if (menuSelection == 1)
  {
    Serial.println("CONTROL MENU");
    currentMenu = MANUAL_CONTROL;
    resetMenuSelection();
  }
  encoder.write(0);
  encoderPosition = 0;
}

void displayWashingMenu()
{
  int menuSelection = abs(encoderPosition) % 3;

  if (prevMenuSelection != menuSelection)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(menuSelection == 0 ? ">START" : " START");
    lcd.print(menuSelection == 1 ? ">SET" : " SET");
    lcd.print(menuSelection == 2 ? ">Back" : " Back");
    lcd.setCursor(0, 1);
    String washTimeString = cronos::millis_to_string(washTime * 1000);
    lcd.print(washTimeString);
    prevMenuSelection = menuSelection;
  }
}

void handleWashingMenuSelection()
{
  int menuSelection = abs(encoderPosition) % 3;
  if (menuSelection == 0)
  {
    Serial.println("START");
    startWashing();
  }
  else if (menuSelection == 1)
  {
    Serial.println("SET TIME");
    setTimeMenu();
  }
  else if (menuSelection == 2)
  {
    Serial.println("BACK");
    backToMain();
  }
  encoder.write(0);
  encoderPosition = 0;
}

void displayManualControlMenu()
{
  int menuSelection = abs(encoderPosition) % 3;

  if (prevMenuSelection != menuSelection)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(menuSelection == 0 ? ">UP" : " UP");
    lcd.setCursor(0, 1);
    lcd.print(menuSelection == 1 ? ">DOWN" : " DOWN");
    lcd.print(menuSelection == 2 ? ">Back" : " Back");
    prevMenuSelection = menuSelection;
  }
}
void liftUp()
{
  if (liftHomeDone || readLimitSwitch())
  {
    liftingStepper.on();
    liftingStepper.setSpeed(90);
    liftingStepper.setRight();
    liftingStepper.runSteps(25000, getButton);
    liftingStepper.off();
  }
}
void handleManualControlSelection()
{
  int menuSelection = abs(encoderPosition) % 3;
  if (menuSelection == 0)
  {
    liftUp();
    Serial.println("MOVE UP");
  }
  else if (menuSelection == 1)
  {
    Serial.println("MOVE DOWN");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("HOMING");
    homeLift();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("HOMING DONE");
    delay(1000);
  }
  else if (menuSelection == 2)
  {
    Serial.println("BACK");
    backToMain();
  }
  encoder.write(0);
  encoderPosition = 0;
}

void backToMain()
{
  currentMenu = MAIN_MENU;
  resetMenuSelection();
}
void resetMenuSelection()
{
  prevMenuSelection = -1;
}
void setTimeMenu()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  String washTimeString = cronos::millis_to_string(washTime * 1000);
  lcd.print(washTimeString);

  while (!getButton())
  {
    encoderPosition = getEncoderValue();
    static int prevEncoderPosition = encoderPosition;
    if (encoderPosition != prevEncoderPosition)
    {
      prevEncoderPosition = encoderPosition;
      washTime = getWashTime(encoderPosition);
      String washTimeString = cronos::millis_to_string(washTime * 1000);
      lcd.setCursor(0, 0);
      lcd.print(washTimeString);
    }
  }
}
unsigned long millisWhenStartWash = 0;
void startWashing()
{
  washingStepper.on();
  if (!readLimitSwitch())
  {
    homeLift();
  }
  //liftUp();
  millisWhenStartWash = millis();
  unsigned long timeLeft = millisWhenStartWash - washTime * 1000;
  while (timeLeft > 0)
  {
    timeLeft = millisWhenStartWash - washTime * 1000;
    washingStepper.changeDirection();
    washingStepper.runSteps(30000, emergencyStopWash);
    delay(300);
    static Timer displayTimeLeft(1000);
    if (displayTimeLeft.timeElapsed())
    {
      lcd.clear();
      String timeLeftString = cronos::millis_to_string(timeLeft * 1000);
      lcd.print(timeLeftString);
    }
  }
  washingStepper.off();
}
bool emergencyStopWash()
{
  if (millisWhenStartWash - washTime * 1000 > 0)
    return false;
  else
    return true;
}
void loop()
{
  encoderPosition = getEncoderValue();
  bool buttonPressed = getButton();

  if (currentMenu != prevMenu)
  {
    resetMenuSelection();
    prevMenu = currentMenu;
  }
  switch (currentMenu)
  {
  case MAIN_MENU:
    displayMainMenu();
    if (buttonPressed)
      handleMainMenuSelection();
    break;
  case START_WASHING:
    displayWashingMenu();
    if (buttonPressed)
      handleWashingMenuSelection();
    break;
  case MANUAL_CONTROL:
    displayManualControlMenu();
    if (buttonPressed)
      handleManualControlSelection();
    break;
    // Add other cases as needed
  }
}
