#include "LocalTimeRK.h"

LocalTimeChange::LocalTimeChange() {
}
LocalTimeChange::~LocalTimeChange() {
}

LocalTimeChange::LocalTimeChange(const char *str) {
    parse(str);
}

void LocalTimeChange::clear() {
    month = week = dayOfWeek = hour = minute = second = valid = 0;
}


void LocalTimeChange::parse(const char *str) {
    // M3.2.0/2:00:00
    if (!str || str[0] != 'M') {
        clear();
        return;
    }

    int values[6];
    if (sscanf(str, "M%d.%d.%d/%d:%d:%d", &values[0], &values[1], &values[2], &values[3], &values[4], &values[5]) == 6) {
        month = (int8_t) values[0];
        week = (int8_t) values[1];
        dayOfWeek = (int8_t) values[2];
        hour = (int8_t) values[3];
        minute = (int8_t) values[4];
        second = (int8_t) values[5];
        valid = true;
    }
    else {
        clear();
    }
}

String LocalTimeChange::toString() const {
    if (valid) {
        return String::format("M%d.%d.%d/%d:%02d:%02d", (int)month, (int)week, (int)dayOfWeek, (int)hour, (int)minute, (int)second);
    }
    else {
        return "";
    }
}


/*
    int8_t month; // 1-12, 1=January
    int8_t week; // 1-5, 1=first
    int8_t dayOfWeek; // 0-6, 0=Sunday, 1=Monday, ...
    int8_t hour; // 0-23 hour to make change
    int8_t minute; // 0-59 minute
    int8_t second; // 0-59 second
    int8_t valid; // true = valid
*/

LocalTimePosixTimezone::LocalTimePosixTimezone() {
}
LocalTimePosixTimezone::~LocalTimePosixTimezone() {
}

LocalTimePosixTimezone::LocalTimePosixTimezone(const char *str) {
    parse(str);
}

void LocalTimePosixTimezone::parse(const char *str) {

}


// [static]
void LocalTime::timeToTm(time_t time, struct tm *pTimeInfo) {
#ifndef UNITTEST
    // Particle C standard library does not implement gmtime_r, however the C
    // library is always set to UTC, so this works
    localtime_r(&time, pTimeInfo);
#else
    gmtime_r(&time, pTimeInfo);     
#endif
    }

// [static]
time_t LocalTime::tmToTime(struct tm *pTimeInfo) {
#ifndef UNITTEST
    // Particle C standard library does not implement timegm, however the C
    // library is always set to UTC, so this works
    return mktime(pTimeInfo);
#else
    return timegm(pTimeInfo);        
#endif
}