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
        LocalTime::timeToTm(time - config.standardHMS.toSeconds(), &localTimeValue.localTimeInfo);
    }
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
