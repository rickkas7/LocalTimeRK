#include "LocalTimeRK.h"

SerialLogHandler logHandler;
SYSTEM_THREAD(ENABLED);

const unsigned long checkPeriodMs = 1000;
unsigned long checkLast = 0;

time_t nextMinutely = 0;
time_t nextHourly = 0;
time_t nextDaily = 0;

void setup() {
    // Set timezone to the Eastern United States
    LocalTime::instance().withConfig(LocalTimePosixTimezone("EST5EDT,M3.2.0/2:00:00,M11.1.0/2:00:00"));

}

void loop() {
    static unsigned long lastLog = 0;
    if (millis() - checkLast > checkPeriodMs && Time.isValid()) {
        checkLast = millis();

        time_t now = Time.now();

        if (!nextMinutely || nextMinutely <= now) {
            LocalTimeConvert conv;
            conv.withCurrentTime().convert();
            String currentTime = conv.timeStr().c_str();
            
            conv.nextMinute();
            nextMinutely = conv.time;
            Log.info("minutely current=%s next=%s", currentTime.c_str(), conv.timeStr().c_str());
        }
        if (!nextHourly || nextHourly <= now) {
            LocalTimeConvert conv;
            conv.withCurrentTime().convert();
            String currentTime = conv.timeStr().c_str();

            conv.nextHour();
            nextHourly = conv.time;
            Log.info("hourly current=%s next=%s", currentTime.c_str(), conv.timeStr().c_str());
        }
        if (!nextDaily || nextDaily <= now) {
            LocalTimeConvert conv;
            conv.withCurrentTime().convert();
            String currentTime = conv.timeStr().c_str();
            
            conv.nextDayMidnight();
            nextDaily = conv.time;

            Log.info("daily current=%s next=%s", currentTime.c_str(), conv.timeStr().c_str());
        }
    }
}
