#include "LocalTimeRK.h"

LocalTime *LocalTime::_instance;

//
// LocalTimeYMD
//
void LocalTimeYMD::fromTimeInfo(const struct tm *pTimeInfo) {
    ymd.year = pTimeInfo->tm_year;
    ymd.month = pTimeInfo->tm_mon + 1;
    ymd.day = pTimeInfo->tm_mday;
}

void LocalTimeYMD::fromLocalTimeValue(const LocalTimeValue &value) {
    *this = value.ymd();
}

int LocalTimeYMD::getDayOfWeek() const {
    struct tm timeInfo = {0};

    timeInfo.tm_year = ymd.year;
    timeInfo.tm_mon = ymd.month - 1;
    timeInfo.tm_mday = ymd.day;
    
    time_t time = LocalTime::tmToTime(&timeInfo);
    LocalTime::timeToTm(time, &timeInfo);

    return timeInfo.tm_wday;
}

int LocalTimeYMD::compareTo(const LocalTimeYMD other) const {
    int cmp;

    if (ymd.year < other.ymd.year) {
        cmp = -1;
    }
    else
    if (ymd.year > other.ymd.year) {
        cmp = +1;
    }
    else {
        if (ymd.month < other.ymd.month) {
            return -1;
        }
        else
        if (ymd.month > other.ymd.month) {
            return +1;
        }
        else {
            if (ymd.day < other.ymd.day) {
                cmp = -1;
            }  
            else
            if (ymd.day > other.ymd.day) {
                cmp = +1;
            }
            else {
                cmp = 0;
            }
        }
    }

    return cmp;
}

bool LocalTimeYMD::parse(const char *s) {
    int year, month, day;
    if (sscanf(s, "%d-%d-%d", &year, &month, &day) == 3) {
        setYear(year);
        setMonth(month);
        setDay(day);
        return true;
    }
    else {
        return false;
    }
}


//
// LocalTimeHMS
//
LocalTimeHMS::LocalTimeHMS() {
}

LocalTimeHMS::~LocalTimeHMS() {
}

LocalTimeHMS::LocalTimeHMS(const char *str) {
    parse(str);
}

LocalTimeHMS::LocalTimeHMS(const LocalTimeValue &value) {
    *this = value.hms();
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
    if (hour < 0) {
        return - (((int)hour) * -3600 + ((int)minute) * 60 + (int) second);
    }
    else {
        return ((int)hour) * 3600 + ((int)minute) * 60 + (int) second;
    }
}

void LocalTimeHMS::fromTimeInfo(const struct tm *pTimeInfo) {
    hour = (int8_t) pTimeInfo->tm_hour;
    minute = (int8_t) pTimeInfo->tm_min;
    second = (int8_t) pTimeInfo->tm_sec;
}

void LocalTimeHMS::fromLocalTimeValue(const LocalTimeValue &value) {
    *this = value.hms();
}


void LocalTimeHMS::toTimeInfo(struct tm *pTimeInfo) const {
    if (!ignore) {
        pTimeInfo->tm_hour = hour;
        pTimeInfo->tm_min = minute;
        pTimeInfo->tm_sec = second;
    }
}

void LocalTimeHMS::adjustTimeInfo(struct tm *pTimeInfo) const {
    if (!ignore) {
        if (hour < 0) {
            pTimeInfo->tm_hour += hour;
            pTimeInfo->tm_min -= minute;
            pTimeInfo->tm_sec -= second;
        }
        else {
            pTimeInfo->tm_hour += hour;
            pTimeInfo->tm_min += minute;
            pTimeInfo->tm_sec += second;
        }
        
    }
}

//
// LocalTimeRestrictedDate
//
LocalTimeRestrictedDate &LocalTimeRestrictedDate::withOnlyOnDays(LocalTimeDayOfWeek value) { 
    onlyOnDays = value; 
    return *this;
};

LocalTimeRestrictedDate &LocalTimeRestrictedDate::withOnlyOnDates(std::initializer_list<const char *> dates) {
    for(auto it = dates.begin(); it != dates.end(); ++it) {
        onlyOnDates.push_back(LocalTimeYMD(*it));    
    }
    return *this;
}

LocalTimeRestrictedDate &LocalTimeRestrictedDate::withOnlyOnDates(std::initializer_list<LocalTimeYMD> dates) {
    onlyOnDates.insert(onlyOnDates.end(), dates.begin(), dates.end());
    return *this;
}

LocalTimeRestrictedDate &LocalTimeRestrictedDate::withExceptDates(std::initializer_list<const char *> dates) {
    for(auto it = dates.begin(); it != dates.end(); ++it) {
        exceptDates.push_back(LocalTimeYMD(*it));    
    }
    return *this;
}

LocalTimeRestrictedDate &LocalTimeRestrictedDate::withExceptDates(std::initializer_list<LocalTimeYMD> dates) {
    exceptDates.insert(exceptDates.end(), dates.begin(), dates.end());
    return *this;
}

bool LocalTimeRestrictedDate::isEmpty() const {
    return onlyOnDays.isEmpty() && onlyOnDates.empty() && exceptDates.empty();
}

bool LocalTimeRestrictedDate::isValid(LocalTimeValue localTimeValue) const {
    return isValid(localTimeValue.ymd());
}


bool LocalTimeRestrictedDate::isValid(LocalTimeYMD ymd) const {
    bool result = false;

    // Is it in the except days list?
    if (inExceptDates(ymd)) {
        result = false;
    }
    else {
        result = onlyOnDays.isSet(ymd) || inOnlyOnDates(ymd);
    }

    return result;
}

bool LocalTimeRestrictedDate::inOnlyOnDates(LocalTimeYMD ymd) const {
    for(auto it = onlyOnDates.begin(); it != onlyOnDates.end(); ++it) {
        if (*it == ymd) {
            return true;
        }
    }
    return false;
}

bool LocalTimeRestrictedDate::inExceptDates(LocalTimeYMD ymd) const {
    for(auto it = exceptDates.begin(); it != exceptDates.end(); ++it) {
        if (*it == ymd) {
            return true;
        }
    }
    return false;
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
        if (pTimeInfo->tm_mday > LocalTime::lastDayOfMonth(pTimeInfo->tm_year + 1900, month)) {
            // 5 means the last week of the month, even if there is no 5th week
            pTimeInfo->tm_mday -= 7;
        }

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


//
// LocalTimePosixTimezone
//

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

bool LocalTimePosixTimezone::parse(const char *str) {
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
                valid = true;

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
        valid = false;
    }


    free(mutableCopy);

    return valid;
}

//
// LocalTimeValue
//

int LocalTimeValue::hourFormat12() const {
    if (tm_hour == 0) {
        return 12;
    }
    else
    if (tm_hour < 12) {
        return tm_hour;
    }
    else {
        return tm_hour - 12;
    }
}

LocalTimeHMS LocalTimeValue::hms() const {
    LocalTimeHMS result;
    result.fromTimeInfo(this);
    return result;
}

void LocalTimeValue::setHMS(LocalTimeHMS hms) {
    if (!hms.ignore) {
        tm_hour = hms.hour;
        tm_min = hms.minute;
        tm_sec = hms.second;
    }
}

LocalTimeYMD LocalTimeValue::ymd() const {
    LocalTimeYMD result;
    result.fromTimeInfo(this);
    return result;
}


time_t LocalTimeValue::toUTC(LocalTimePosixTimezone config) const {
    struct tm mutableTimeInfo = *this;
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
    (void) LocalTime::stringToTime(str, this);
}

int LocalTimeValue::ordinal() const {
    int ordinal = 1;
    int tempDayOfMonth = tm_mday;
    while((tempDayOfMonth - 7) >= 1) {
        ordinal++;
        tempDayOfMonth -= 7;
    }
    return ordinal;
}




//
// LocalTimeConvert
// 

void LocalTimeConvert::convert() {
    if (!config.isValid()) {
        config = LocalTime::instance().getConfig();
    }

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

        if (dstStart < standardStart) {
            // Northern Hemisphere, DST is in summer
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
            // Southern Hemisphere: DST runs from October/November through the 
            // turn of the year, into March/April. There's a different set of
            // position variables for this.
            if (time < standardStart) {
                // Before the start of standard time this year
                position = Position::BEFORE_STANDARD;
            }
            else if (time < dstStart) {
                // 
                position = Position::IN_STANDARD;
            }
            else {
                position = Position::AFTER_STANDARD;
            }

        }

    }
    else {
        // Just timezones, no DST
        position = Position::NO_DST;
    }
    if (!isDST()) {
        LocalTime::timeToTm(time - config.standardHMS.toSeconds(), &localTimeValue);
    }
    else {
        LocalTime::timeToTm(time - config.dstHMS.toSeconds(), &localTimeValue);
    }

}
void LocalTimeConvert::addSeconds(int seconds) {
    time += seconds;
    convert();
}

void LocalTimeConvert::nextMinuteMultiple(int minuteMultiple, int startingModulo) {
    time += minuteMultiple * 60;

    struct tm timeInfo;

    LocalTime::timeToTm(time, &timeInfo);

    timeInfo.tm_min -= ((timeInfo.tm_min - startingModulo) % minuteMultiple);
    timeInfo.tm_sec = 0;

    time = LocalTime::tmToTime(&timeInfo);

    convert();
}


void LocalTimeConvert::nextTimeList(std::initializer_list<LocalTimeHMS> _hmsList) {
    std::vector<LocalTimeHMS> hmsList = _hmsList;

    time_t origTime = time;
    time_t resultTime = origTime + 86400;

    for(auto it = hmsList.begin(); it != hmsList.end(); ++it) {
        LocalTimeHMS hms = *it;

        nextTime(hms);
        if (time > origTime && time < resultTime) {
            resultTime = time;
        }

        // Restore original time
        time = origTime;
        convert();
    }

    time = resultTime;
    convert();
}


void LocalTimeConvert::nextTime(LocalTimeHMS hms) {
    time_t origTime = time;

    atLocalTime(hms);
    if (time <= origTime) {
        time += 86400;
        convert();
        atLocalTime(hms);
    }
}



void LocalTimeConvert::nextDay(LocalTimeHMS hms) {
    time += 86400;
    convert();

    atLocalTime(hms);
}

void LocalTimeConvert::nextDayOrTimeChange(LocalTimeHMS hms) {
    time_t timeOrig = time;
    time_t dstStartOrig = dstStart;
    time_t standardStartOrig = standardStart;

    nextDay(hms);

    if (dstStartOrig > timeOrig && dstStartOrig < time) {
        // printf("timeOrig=%ld dstStartOrig=%ld time=%ld dstStart=%ld\n", timeOrig, dstStartOrig, time, dstStart);
        time = dstStartOrig;
        convert();
    }
    if (standardStartOrig > timeOrig && standardStartOrig < time) {
        time = standardStartOrig;
        convert();
    }
}   


bool LocalTimeConvert::nextDayOfWeek(int dayOfWeek, LocalTimeHMS hms) {
    if (dayOfWeek < 0 || dayOfWeek > 6) {
        return false;
    }

    do {
        nextDay(hms);
    } while(localTimeValue.tm_wday != dayOfWeek);

    return true;
}

void LocalTimeConvert::nextWeekday(LocalTimeHMS hms) {
    do {
        nextDay(hms);
    } while(localTimeValue.tm_wday == 0 || localTimeValue.tm_wday == 6);
}

void LocalTimeConvert::nextWeekendDay(LocalTimeHMS hms) {
    do {
        nextDay(hms);
    } while(localTimeValue.tm_wday != 0 && localTimeValue.tm_wday != 6);
}

bool LocalTimeConvert::nextDayOfMonth(int dayOfMonth, LocalTimeHMS hms) {
    time_t origTime = time;

    localTimeValue.tm_mday = (dayOfMonth > 0) ? dayOfMonth : (lastDayOfMonth() + dayOfMonth);
    localTimeValue.setHMS(hms);
    time = localTimeValue.toUTC(config);
    convert();

    if (time <= origTime) {
        // The target dayOfMonth and time is before the original time, so move to next month
        localTimeValue.tm_mon++;
        time = localTimeValue.toUTC(config);
        convert();
    }
    return true;
}


bool LocalTimeConvert::nextDayOfNextMonth(int dayOfMonth, LocalTimeHMS hms) {
    localTimeValue.tm_mon++;
    localTimeValue.tm_mday = (dayOfMonth > 0) ? dayOfMonth : (lastDayOfMonth() + dayOfMonth);
    localTimeValue.setHMS(hms);
    time = localTimeValue.toUTC(config);
    convert();
    return true;
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
    convert();

    if (time <= origTime) {
        // Need to move to tomorrow
        nextDay(hms);
    }
}

bool LocalTimeConvert::nextSchedule(const Schedule &schedule) {
    time_t origTime = time;

    for(int tries = 0; tries < 2; tries++) {
        // Check the minute multiples
        ScheduleItemMinuteMultiple foundItem;

        // 86400 + 3600 for fall back on time change = 90000, plus some extra
        time_t smallestTimeSpan = 100000; 

        time_t closestTime = 0;


        for(auto it = schedule.minuteMultipleItems.begin(); it != schedule.minuteMultipleItems.end(); ++it) {
            ScheduleItemMinuteMultiple item = *it;
            if (inLocalTimeRange(item.timeRange)) {
                // This minute multiple applies
                time_t timeSpan = item.getTimeSpan(*this);
                //printf("found minute multiple timeSpan=%d\n", (int)timeSpan);
                if (timeSpan < smallestTimeSpan) {
                    foundItem = item;
                    smallestTimeSpan = timeSpan;
                }
            }
        }

        if (foundItem.isValid()) {
            LocalTimeConvert tmpConvert(*this);
            tmpConvert.nextMinuteMultiple(foundItem.minuteMultiple, foundItem.timeRange.hmsStart.minute % foundItem.minuteMultiple);
            //printf("converting with multiple %d was %d now %d\n", foundItem.minuteMultiple, (int)origTime, (int)tmpConvert.time);
            if (closestTime == 0 || tmpConvert.time < closestTime) {
                closestTime = tmpConvert.time;
            }
        } 
        else {
            // Not in a time range. Is there a time range after the current time?
            for(auto it = schedule.minuteMultipleItems.begin(); it != schedule.minuteMultipleItems.end(); ++it) {
                ScheduleItemMinuteMultiple item = *it;
                LocalTimeConvert tmpConvert(*this);
                tmpConvert.atLocalTime(item.timeRange.hmsStart);
                if (time < tmpConvert.time) {
                    time_t timeSpan = (tmpConvert.time - time);
                    if (timeSpan < smallestTimeSpan) {
                        foundItem = item;
                        smallestTimeSpan = timeSpan;
                    }
                }
            }
            if (foundItem.isValid()) {
                LocalTimeConvert tmpConvert(*this);
                tmpConvert.atLocalTime(foundItem.timeRange.hmsStart);
                if (closestTime == 0 || tmpConvert.time < closestTime) {
                    closestTime = tmpConvert.time;
                }
            } 
        }

        // Check the times
        for(auto it = schedule.times.begin(); it != schedule.times.end(); ++it) {
            LocalTimeHMS tmpHMS = *it;

            LocalTimeConvert tmpConvert(*this);
            tmpConvert.nextTime(tmpHMS);
            if (closestTime == 0 || tmpConvert.time < closestTime) {
                closestTime = tmpConvert.time;
            }        
        }

        if (closestTime != 0) {
            time = closestTime;
            convert();
            return true;
        }

        // Try times in the next day starting at midnight
        nextDay(LocalTimeHMS("00:00:00"));
    }

    time = origTime;
    convert();

    return false;
}


void LocalTimeConvert::atLocalTime(LocalTimeHMS hms) {
    if (!hms.ignore) {
        localTimeValue.setHMS(hms);
        time = localTimeValue.toUTC(config);
        convert();
    }
}

bool LocalTimeConvert::inLocalTimeRange(TimeRange localTimeRange) {
    time_t origTime = time;


    localTimeValue.setHMS(localTimeRange.hmsStart);
    time_t minTime = localTimeValue.toUTC(config);

    localTimeValue.setHMS(localTimeRange.hmsEnd);
    time_t maxTime = localTimeValue.toUTC(config);

    time = origTime;
    convert();

    return (minTime <= origTime) && (origTime <= maxTime);
}

bool LocalTimeConvert::beforeLocalTimeRange(TimeRange localTimeRange) {
    time_t origTime = time;

    localTimeValue.setHMS(localTimeRange.hmsStart);
    time_t minTime = localTimeValue.toUTC(config);

    time = origTime;
    convert();

    return (origTime < minTime);
}


String LocalTimeConvert::timeStr() {
    char ascstr[26];
    asctime_r(&localTimeValue, ascstr);
    int len = strlen(ascstr);
    ascstr[len-1] = 0; // remove final newline
    return String(ascstr);
}

String LocalTimeConvert::format(const char* format_spec) {

    if (!format_spec || !strcmp(format_spec, TIME_FORMAT_DEFAULT)) {
        return timeStr();
    }

    // This implementation is from spark_wiring_time.cpp

    char format_str[64];
    // only copy up to n-1 to dest if no null terminator found
    strncpy(format_str, format_spec, sizeof(format_str) - 1); // Flawfinder: ignore (ch42318)
    format_str[sizeof(format_str) - 1] = '\0'; // ensure null termination
    size_t len = strlen(format_str); // Flawfinder: ignore (ch42318)

    // while we are not using stdlib for managing the timezone, we have to do this manually
    String zoneNameStr = zoneName();

    char time_zone_str[16];
    if (config.isZ()) {
        strcpy(time_zone_str, "Z");
    }
    else {
        int time_zone = isDST() ? config.dstHMS.toSeconds() : config.standardHMS.toSeconds();

        snprintf(time_zone_str, sizeof(time_zone_str), "%+03d:%02u", -time_zone/3600, abs(time_zone/60)%60);
    }

    // replace %z with the timezone
    for (size_t i=0; i<len-1; i++)
    {
        if (format_str[i]=='%' && format_str[i+1]=='z')
        {
            size_t tzlen = strlen(time_zone_str);
            memcpy(format_str+i+tzlen, format_str+i+2, len-i-1);    // +1 include the 0 char
            memcpy(format_str+i, time_zone_str, tzlen);
            len = strlen(format_str);
        }
        else
        if (format_str[i]=='%' && format_str[i+1]=='Z')
        {
            size_t tzlen = zoneNameStr.length();
            memcpy(format_str+i+tzlen, format_str+i+2, len-i-1);    // +1 include the 0 char
            memcpy(format_str+i, zoneNameStr.c_str(), tzlen);
            len = strlen(format_str);
        }
    }

    char buf[50] = {};
    strftime(buf, sizeof(buf), format_str, &localTimeValue);
    return String(buf);    
}

String LocalTimeConvert::zoneName() const { 
    if (config.isZ()) {
        return "Z";
    }
    else
    if (isDST()) {
        return config.dstName;
    }
    else {
        return config.standardName;
    }
};

int LocalTimeConvert::lastDayOfMonth() const {
    return LocalTime::lastDayOfMonth(localTimeValue.tm_year + 1900, (localTimeValue.tm_mon % 12) + 1);
}


//
// LocalTime
//
LocalTime &LocalTime::instance() {
    if (!_instance) {
        _instance = new LocalTime();
    }
    return *_instance;
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
        timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);
}


// [static]
int LocalTime::lastDayOfMonth(int year, int month) {
    switch(month) {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
            return 31;

        case 2:
            if ((year % 4) == 0) {
                if ((year % 100) == 0) {
                    return 28;
                }
                else {
                    return 29;
                }
            }
            else {
                return 28;
            }

        case 4:
        case 6:
        case 9:
        case 11:
            return 30;
    }
    return 0;
}