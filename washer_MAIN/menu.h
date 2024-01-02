#pragma once

void resetMenuSelection()
{
    prevMenuSelection = -1;
}
void backToMain()
{
    currentMenu = MAIN_MENU;
    resetMenuSelection();
}
void display_MAIN()
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
void handle_MAIN()
{
    int menuSelection = abs(encoderPosition) % 2;
    if (menuSelection == 0)
    {
        currentMenu = WASHING_MENU;
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

void display_WASHING()
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

void handle_WASHING()
{
    int menuSelection = abs(encoderPosition) % 3;
    if (menuSelection == 0)
    {
        Serial.println("START");
        START_WASHING();
    }
    else if (menuSelection == 1)
    {
        Serial.println("SET TIME");
        SET_TIME();
    }
    else if (menuSelection == 2)
    {
        Serial.println("BACK");
        backToMain();
    }
    encoder.write(0);
    encoderPosition = 0;
}

void display_CONTROL()
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

void handle_CONTROL()
{
    int menuSelection = abs(encoderPosition) % 3;
    if (menuSelection == 0)
    {
        LIFT_UP();
        Serial.println("MOVE UP");
    }
    else if (menuSelection == 1)
    {
        Serial.println("MOVE DOWN");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("HOMING");
        HOME_VERTICAL();
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
