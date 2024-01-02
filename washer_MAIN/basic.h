#pragma once

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
