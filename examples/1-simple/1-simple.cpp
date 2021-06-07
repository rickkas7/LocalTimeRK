#include "LocalTimeRK.h"

SerialLogHandler logHandler;
SYSTEM_THREAD(ENABLED);

void setup() {
    // Set timezone to the Eastern United States
    LocalTime::instance().withConfig(LocalTimePosixTimezone("EST5EDT,M3.2.0/2:00:00,M11.1.0/2:00:00"));
}

void loop() {
    static unsigned long lastLog = 0;
    if (millis() - lastLog > 10000 && Time.isValid()) {
        lastLog = millis();
        
        LocalTimeConvert conv;
        conv.withCurrentTime().convert();

        Log.info("local time: %s", conv.format(TIME_FORMAT_ISO8601_FULL).c_str());
    }
}
