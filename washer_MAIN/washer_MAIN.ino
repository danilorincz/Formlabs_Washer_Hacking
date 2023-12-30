#define homeWifi
#include <LiquidCrystal_I2C.h>

#include <WiFi.h>
#include <WiFiUDP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#define ENCODER_DO_NOT_USE_INTERRUPTS
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
int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

//* ENCODER
Encoder encoder(OUT_A, OUT_B);
long oldPosition = -999;

//* STEPPER
Stepper liftingStepper(S1_STEP_PIN, S1_DIR_PIN, S1_EN_PIN);
Stepper washingStepper(S2_STEP_PIN, S2_DIR_PIN, S2_EN_PIN);

//* WIFI
const int udpPort = 4210;
#ifdef work
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
  liftingStepper.off();
  liftingStepper.setSpeed(100);
}

int getEncoderValue()
{
  return encoder.read() / 4;
}
bool getButton()
{
  if (digitalRead(SW))
    return false;
  else
    return true;
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

void loop()
{
  AsyncElegantOTA.loop();
  int newPosition = getEncoderValue();
  static Timer displayLCD(200);
  if (displayLCD.timeElapsed())
  {
    if (newPosition != oldPosition)
    {
      oldPosition = newPosition;

      long newWashTime = getWashTime(newPosition);
      String newWashTimeString = cronos::millis_to_string(newWashTime * 1000);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(newWashTimeString);
      lcd.setCursor(0, 1);
      lcd.print(newPosition);
    }
  }
  if (getButton())
  {

    homeLift();
    /*
    liftingStepper.changeDirection();
    liftingStepper.on();
    liftingStepper.runSteps(newPosition * 100, returnFalse);
    liftingStepper.off();*/
    encoder.write(0);
  }
  lcd.setCursor(10, 1);
  lcd.print(readLimitSwitch());
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
  liftingStepper.setSpeed(20);
  liftingStepper.setLeft();
  liftingStepper.runSteps(10000, readLimitSwitch);
  liftingStepper.off();
}
