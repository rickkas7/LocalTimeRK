#include "LocalTimeRK.h"

LocalTimeHMS::LocalTimeHMS() {

}
LocalTimeHMS::~LocalTimeHMS() {

}

LocalTimeHMS::LocalTimeHMS(const char *str) {
    parse(str);
}

void LocalTimeHMS::clear() {
    hour = minute = second = 0;
}

void LocalTimeHMS::parse(const char *str) {
    clear();

    int values[3];
    if (sscanf(str, "%d:%d:%d", &values[0], &values[1], &values[2]) == 3) {
        hour = (int8_t) values[0];    
        minute = (int8_t) values[1];    
        second = (int8_t) values[2];    
    }
    else
    if (sscanf(str, "%d:%d", &values[0], &values[1]) == 2) {
        hour = (int8_t) values[0];    
        minute = (int8_t) values[1];    
    }
    else
    if (sscanf(str, "%d", &values[0]) == 1) {
        hour = (int8_t) values[0];    
    }
}

String LocalTimeHMS::toString() const {
    return String::format("%d:%02d:%02d", (int)hour, (int)minute, (int)second);
}

int LocalTimeHMS::toSeconds() const {
    return ((int)hour) * 3600 + ((int)minute) * 60 + (int) second;
}

void LocalTimeHMS::fromTimeInfo(const struct tm *pTimeInfo) {
    hour = (int8_t) pTimeInfo->tm_hour;
    minute = (int8_t) pTimeInfo->tm_min;
    second = (int8_t) pTimeInfo->tm_sec;
}

void LocalTimeHMS::toTimeInfo(struct tm *pTimeInfo) const {
    pTimeInfo->tm_hour = hour;
    pTimeInfo->tm_min = minute;
    pTimeInfo->tm_sec = second;
}

void LocalTimeHMS::adjustTimeInfo(struct tm *pTimeInfo, bool subtract) const {
    if (subtract) {
        pTimeInfo->tm_hour -= hour;
        pTimeInfo->tm_min -= minute;
        pTimeInfo->tm_sec -= second;
    }
    else {
        pTimeInfo->tm_hour += hour;
        pTimeInfo->tm_min += minute;
        pTimeInfo->tm_sec += second;
    }
}


//
// LocalTimeChange
//
LocalTimeChange::LocalTimeChange() {
}
LocalTimeChange::~LocalTimeChange() {
}

LocalTimeChange::LocalTimeChange(const char *str) {
    parse(str);
}

void LocalTimeChange::clear() {
    month = week = dayOfWeek = valid = 0;
    hms.clear();
}


void LocalTimeChange::parse(const char *str) {
    // M3.2.0/2:00:00
    if (!str || str[0] != 'M') {
        clear();
        return;
    }

    int values[3];
    if (sscanf(str, "M%d.%d.%d", &values[0], &values[1], &values[2]) == 3) {
        month = (int8_t) values[0];
        week = (int8_t) values[1];
        dayOfWeek = (int8_t) values[2];

        const char *cp = strchr(str, '/');
        if (cp) {
            cp++;
            hms.parse(cp);
        }
        else {
            hms.clear();
        }
        valid = true;
    }
    else {
        clear();
    }
}

String LocalTimeChange::toString() const {
    if (valid) {
        return String::format("M%d.%d.%d/%d:%02d:%02d", (int)month, (int)week, (int)dayOfWeek, (int)hms.hour, (int)hms.minute, (int)hms.second);
    }
    else {
        return "";
    }
}

time_t LocalTimeChange::calculate(struct tm *pTimeInfo, LocalTimeHMS tzAdjust) const {
    // Start with the first of the month
    pTimeInfo->tm_mday = 1;
    pTimeInfo->tm_mon = month - 1; // tm_mon is zero-based!
    pTimeInfo->tm_hour = pTimeInfo->tm_min = pTimeInfo->tm_sec = 0;
    LocalTime::tmToTime(pTimeInfo);

    while(pTimeInfo->tm_wday != dayOfWeek) {
        pTimeInfo->tm_mday++;
        LocalTime::tmToTime(pTimeInfo);
    }
    if (week != 1) {
        pTimeInfo->tm_mday += (week - 1) * 7;
        LocalTime::tmToTime(pTimeInfo);        
    }

    // We now know the date of time change in local time

    // Set the time of the time change in local time (for example, 2:00:00 in the United States)
    hms.toTimeInfo(pTimeInfo);

    // Handle timezone conversion (also DST if necessary)
    // The tzAdjust values are positive in the US, so to convert local time to UTC we need to add
    // to the hour values
    tzAdjust.adjustTimeInfo(pTimeInfo);

    return LocalTime::tmToTime(pTimeInfo);
}




LocalTimePosixTimezone::LocalTimePosixTimezone() {
}
LocalTimePosixTimezone::~LocalTimePosixTimezone() {
}

LocalTimePosixTimezone::LocalTimePosixTimezone(const char *str) {
    parse(str);
}

void LocalTimePosixTimezone::clear() {
    dstStart.clear();
    dstName = "";
    dstHMS.clear();
    standardStart.clear();
    standardName = "";
    standardHMS.clear();
}

void LocalTimePosixTimezone::parse(const char *str) {
    char *mutableCopy = strdup(str);

    char *token, *save = mutableCopy;
    size_t ii = 0;
    while((token = strtok_r(save, ",", &save)) != 0) {
        switch(ii++) {
            case 0: {
                // Timezone specifier
                char *cp = token, *start = token, save2;
                while(*cp >= 'A') {
                    cp++;
                }
                save2 = *cp;
                *cp = 0;
                standardName = start;
                *cp = save2;

                if (*cp) {
                    start = cp;
                    while(*cp && *cp < 'A') {
                        cp++;
                    }
                    save2 = *cp;
                    *cp = 0;
                    standardHMS.parse(start);
                    *cp = save2;

                    if (*cp) {
                        start = cp;
                        while(*cp && *cp >= 'A') {
                            cp++;
                        }
                        save2 = *cp;
                        *cp = 0;
                        dstName = start;
                        *cp = save2;

                        if (*cp) {
                            start = cp;
                            dstHMS.parse(start);
                        }
                        else {
                            // Default dst is 1 hour later
                            dstHMS = standardHMS;
                            dstHMS.hour--;
                        }
                    }
                }
                break;
            }
            case 1: {
                dstStart.parse(token);
                break;
            }
            case 2: {
                standardStart.parse(token);
                break;
            }
        }
    }

    if (dstStart.valid && !standardStart.valid) {
        // If DST start is specified, standard start must also be specified
        dstStart.clear();
    }


    free(mutableCopy);
}

String LocalTimePosixTimezone::toString() const {
    return "";
}

//
// LocalTimeValue
//

int LocalTimeValue::hourFormat12() const {
    if (localTimeInfo.tm_hour == 0) {
        return 12;
    }
    else
    if (localTimeInfo.tm_hour < 12) {
        return localTimeInfo.tm_hour;
    }
    else {
        return localTimeInfo.tm_hour - 12;
    }
}

LocalTimeHMS LocalTimeValue::hms() const {
    LocalTimeHMS result;
    result.fromTimeInfo(&localTimeInfo);
    return result;
}

void LocalTimeValue::setHMS(LocalTimeHMS hms) {
    localTimeInfo.tm_hour = hms.hour;
    localTimeInfo.tm_min = hms.minute;
    localTimeInfo.tm_sec = hms.second;
}


time_t LocalTimeValue::toUTC(LocalTimePosixTimezone config) const {
    struct tm mutableTimeInfo = localTimeInfo;
    time_t standardTime, dstTime;
    
    standardTime = dstTime = LocalTime::tmToTime(&mutableTimeInfo);
    standardTime += config.standardHMS.toSeconds();

    if (config.hasDST()) {
        LocalTimeConvert convert;
        convert.withConfig(config).withTime(standardTime).convert();

        if (convert.isDST()) {
            // The time is in DST, so return that instead
            dstTime += config.dstHMS.toSeconds();
            return dstTime;
        }
    }
    return standardTime;
}

void LocalTimeValue::fromString(const char *str) {
    (void) LocalTime::stringToTime(str, &localTimeInfo);
}

int LocalTimeValue::ordinal() const {
    int ordinal = 1;
    int tempDayOfMonth = localTimeInfo.tm_mday;
    while((tempDayOfMonth - 7) >= 1) {
        ordinal++;
        tempDayOfMonth -= 7;
    }
    return ordinal;
}




//
// LocalTimeConvert
// 
LocalTimeConvert::LocalTimeConvert() {
}

LocalTimeConvert::~LocalTimeConvert() {
}

void LocalTimeConvert::convert() {
    if (config.hasDST()) {
        // We need to worry about daylight saving time
        LocalTime::timeToTm(time, &dstStartTimeInfo);
        standardStartTimeInfo = dstStartTimeInfo;

        // Calculate start of DST. Note that the second parameter is standardHMS because when you enter DST at 
        // a local standard time; you have not yet entered DST.
        dstStart = config.dstStart.calculate(&dstStartTimeInfo, config.standardHMS);

        // Calculate start of standard time. Same for the second parameter here, when entering standard time
        // you are leaving DST. For example you leave DST at 2 AM EDT (-0400) so that's the adjustment to UTC.
        standardStart = config.standardStart.calculate(&standardStartTimeInfo, config.dstHMS);

        if (time < dstStart) {
            // Before the start of DST this year
            position = Position::BEFORE_DST;
        }
        else if (time < standardStart) {
            // In DST, before the end of DST in this year
            position = Position::IN_DST;
        }
        else {
            // After the end of DST in this year
            position = Position::AFTER_DST;
        }
    }
    else {
        // Just timezones, no DST
        position = Position::NO_DST;
    }
    if (!isDST()) {
        LocalTime::timeToTm(time - config.standardHMS.toSeconds(), &localTimeValue.localTimeInfo);
    }
    else {
        LocalTime::timeToTm(time - config.dstHMS.toSeconds(), &localTimeValue.localTimeInfo);
    }

}


void LocalTimeConvert::nextDay() {
    time += 86400;
    convert();
}

void LocalTimeConvert::nextDay(LocalTimeHMS hms) {
    time += 86400;
    convert();

    atLocalTime(hms);
}

bool LocalTimeConvert::nextDayOfWeek(int dayOfWeek, LocalTimeHMS hms) {
    if (dayOfWeek < 0 || dayOfWeek > 6) {
        return false;
    }

    do {
        nextDay(hms);
    } while(localTimeValue.localTimeInfo.tm_wday != dayOfWeek);

    return true;
}

void LocalTimeConvert::nextWeekday(LocalTimeHMS hms) {
    do {
        nextDay(hms);
    } while(localTimeValue.localTimeInfo.tm_wday == 0 || localTimeValue.localTimeInfo.tm_wday == 6);
}

void LocalTimeConvert::nextWeekendDay(LocalTimeHMS hms) {
    do {
        nextDay(hms);
    } while(localTimeValue.localTimeInfo.tm_wday != 0 && localTimeValue.localTimeInfo.tm_wday != 6);
}

bool LocalTimeConvert::nextDayOfMonth(int dayOfMonth, LocalTimeHMS hms) {
    if (dayOfMonth < 1 || dayOfMonth > 31) {
        return false;
    }

    time_t origTime = time;

    localTimeValue.localTimeInfo.tm_mday = dayOfMonth;
    localTimeValue.setHMS(hms);
    time = localTimeValue.toUTC(config);
    convert();

    if (time <= origTime) {
        // The target dayOfMonth and time is before the original time, so move to next month
        localTimeValue.localTimeInfo.tm_mon++;
        time = localTimeValue.toUTC(config);
        convert();
    }
    return true;
}


void LocalTimeConvert::nextDayOfNextMonth(int dayOfMonth, LocalTimeHMS hms) {
    localTimeValue.localTimeInfo.tm_mday = dayOfMonth;
    localTimeValue.localTimeInfo.tm_mon++;
    localTimeValue.setHMS(hms);
    time = localTimeValue.toUTC(config);
    convert();
}

bool LocalTimeConvert::nextDayOfWeekOrdinal(int dayOfWeek, int ordinal, LocalTimeHMS hms) {
    for(int tries = 0; tries < 52; tries++) {
        nextDayOfWeek(dayOfWeek, hms);
        if (localTimeValue.ordinal() == ordinal) {
            return true;
        }
    }
    return false;
}


void LocalTimeConvert::nextLocalTime(LocalTimeHMS hms) {
    time_t origTime = time;
    localTimeValue.setHMS(hms);
    time = localTimeValue.toUTC(config);
    if (time <= origTime) {
        // Need to move to tomorrow
        time += 86400;
    }
    convert();
}


void LocalTimeConvert::atLocalTime(LocalTimeHMS hms) {

    localTimeValue.setHMS(hms);
    time = localTimeValue.toUTC(config);
    convert();
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

// [static]
String LocalTime::getTmString(struct tm *pTimeInfo) {
    return String::format("tm_year=%d tm_mon=%d tm_mday=%d tm_hour=%d tm_min=%d tm_sec=%d tm_wday=%d", 
        pTimeInfo->tm_year, pTimeInfo->tm_mon, pTimeInfo->tm_mday,
        pTimeInfo->tm_hour, pTimeInfo->tm_min, pTimeInfo->tm_sec,
        pTimeInfo->tm_wday);
}

// [static]
time_t LocalTime::stringToTime(const char *str, struct tm *pTimeInfo) {
    struct tm timeInfo;
    int parts[6];
    if (sscanf(str, "%d-%d-%d%*c%d:%d:%d", &parts[0], &parts[1], &parts[2], &parts[3], &parts[4], &parts[5]) == 6) {
        timeInfo.tm_year = parts[0] - 1900;
        timeInfo.tm_mon = parts[1] - 1;
        timeInfo.tm_mday = parts[2];
        timeInfo.tm_hour = parts[3];
        timeInfo.tm_min = parts[4];
        timeInfo.tm_sec = parts[5];
        if (pTimeInfo) {
            *pTimeInfo = timeInfo;
        }
        return tmToTime(&timeInfo);
    }
    else {
        return 0;
    }
}


// [static]
String LocalTime::timeToString(time_t time, char separator) {
    struct tm timeInfo;

    timeToTm(time, &timeInfo);

    return String::format("%04d-%02d-%02d%c%02d:%02d:%02d", 
        timeInfo.tm_year + 1900, timeInfo.tm_mon + 1, timeInfo.tm_mday,
        separator,
        timeInfo.tm_hour + timeInfo.tm_min, timeInfo.tm_sec);
}


