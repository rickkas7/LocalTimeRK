#include "Particle.h"

#include "LocalTimeRK.h"
#include "oled-wing-adafruit.h"


SerialLogHandler logHandler;

SYSTEM_THREAD(ENABLED);

OledWingAdafruit display;

void setup() {
    // Set timezone to the Eastern United States
    LocalTime::instance().withConfig(LocalTimePosixTimezone("EST5EDT,M3.2.0/2:00:00,M11.1.0/2:00:00"));

	display.setup();

	display.clearDisplay();
	display.display();
}

void loop() {
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate >= 1000 && Time.isValid()) {
        lastUpdate = millis();

        LocalTimeConvert conv;
        conv.withCurrentTime().convert();

        // Log.info("local time: %s", conv.format(TIME_FORMAT_ISO8601_FULL).c_str());
        
        // Room for 11 characters at text size 2
        display.clearDisplay();
        display.setTextColor(WHITE);
        display.setTextSize(2);
        display.setFont(NULL);
        display.setCursor(0, 1);

        String msg;

        msg = conv.format("%Y-%m-%d"); // 2021-06-07
        display.println(msg);

        msg = conv.format("%I:%M:%S%p"); // 10:00:00AM
        display.println(msg);

        display.display();


    }
}

