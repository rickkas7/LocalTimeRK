#include "LocalTimeRK.h"

SerialLogHandler logHandler;
SYSTEM_THREAD(ENABLED);

LocalTimeSchedule publishSchedule;

void setup() {
    // Set timezone to the Eastern United States
    LocalTime::instance().withConfig(LocalTimePosixTimezone("EST5EDT,M3.2.0/2:00:00,M11.1.0/2:00:00"));

    // Publish every 5 minutes at :00, :05, :10, ...
    publishSchedule.withMinuteOfHour(5);
}

void loop() {
    if (publishSchedule.isScheduledTime() && Particle.connected()) {
        Particle.publish("testEvent", "scheduled publish!");
    }
}
