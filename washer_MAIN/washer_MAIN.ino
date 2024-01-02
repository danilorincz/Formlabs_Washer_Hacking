//* DEFINE NETWORK
#define homeWifi
//#define workWifi

//* INCLUDE LIBRARIES
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WiFiUDP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <Encoder.h>
#include "C:\customLibraries\cronos\cronos.h"
#include "C:\customLibraries\timer\timer.h"
#include "C:\customLibraries\stepperControl\stepperControl_blocking.h"

//* DEFINE PINS
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

//* LCD
unsigned long refreshTime = 120;
int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

//* MENU
enum MenuState
{
  EMPTY,
  MAIN_MENU,
  WASHING_MENU,
  MANUAL_CONTROL
};
MenuState currentMenu = MAIN_MENU;
MenuState prevMenu = EMPTY;
long washTime = 300; // Default washing time

//* ENCODER
Encoder encoder(OUT_A, OUT_B);

//* STEPPER
Stepper verticalStepper(S1_STEP_PIN, S1_DIR_PIN, S1_EN_PIN);
Stepper rotationalStepper(S2_STEP_PIN, S2_DIR_PIN, S2_EN_PIN);

//* NTP TIME
const char *NTP_SERVER = "ch.pool.ntp.org";
const char *TZ_INFO = "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00";
tm timeinfo;
time_t now;
long unsigned lastNTPtime;
unsigned long lastEntryTime;
TimeStruct nowTime;

//* GLOBAL
unsigned long millisWhenStartWash = 0;
bool verticalHomeDone = false;
int encoderPosition = 0;
long oldPosition = -999;
int prevMenuSelection = -1;

#include "webpage.h"
#include "webInterface.h"
#include "basic.h"


void setup()
{
  //? BEGIN
  Serial.begin(115200);
  lcd.init();
  rotationalStepper.begin();
  verticalStepper.begin();
  pinMode(LIMIT_PIN, INPUT);

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

  rotationalStepper.setSpeed(70);
  verticalStepper.setSpeed(100);
  rotationalStepper.off();
  verticalStepper.off();
  lcd.backlight();
  lcd.setCursor(0, 2);
  lcd.clear();
  lcd.print("DENT-ART WASHER");
}

void HOME_VERTICAL()
{
  verticalStepper.on();
  verticalStepper.setSpeed(90);
  verticalStepper.setLeft();
  verticalStepper.runSteps(100000, readLimitSwitch);
  verticalStepper.off();
  verticalHomeDone = true;
}

void LIFT_UP()
{
  if (verticalHomeDone || readLimitSwitch())
  {
    verticalStepper.on();
    verticalStepper.setSpeed(90);
    verticalStepper.setRight();
    verticalStepper.runSteps(25000, getButton);
    verticalStepper.off();
  }
}

void SET_TIME()
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
void START_WASHING()
{
  rotationalStepper.on();
  if (!readLimitSwitch())
  {
    HOME_VERTICAL();
  }
  //LIFT_UP();
  millisWhenStartWash = millis();
  unsigned long timeLeft = millisWhenStartWash - washTime * 1000;
  while (timeLeft > 0)
  {
    timeLeft = washTime * 1000 - (millis() - millisWhenStartWash);
    rotationalStepper.changeDirection();
    rotationalStepper.runSteps(30000, emergencyStopWash);
    delay(300);
    static Timer displayTimeLeft(1000);
    if (displayTimeLeft.timeElapsed())
    {
      lcd.clear();
      String timeLeftString = cronos::millis_to_string(timeLeft * 1000);
      lcd.print(timeLeftString);
    }
  }
  rotationalStepper.off();
  LIFT_UP();
}
bool emergencyStopWash()
{
  if (millisWhenStartWash - washTime * 1000 > 0)
    return false;
  else
    return true;
}
#include "menu.h"
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
    display_MAIN();
    if (buttonPressed)
      handle_MAIN();
    break;

  case WASHING_MENU:
    display_WASHING();
    if (buttonPressed)
      handle_WASHING();
    break;

  case MANUAL_CONTROL:
    display_CONTROL();
    if (buttonPressed)
      handle_CONTROL();
    break;
  }
}
