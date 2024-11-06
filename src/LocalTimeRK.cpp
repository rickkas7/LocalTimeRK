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

void LocalTimeYMD::addDay(int numberOfDays) {
    struct tm timeInfo = {0};

    timeInfo.tm_year = ymd.year;
    timeInfo.tm_mon = ymd.month - 1;
    timeInfo.tm_mday = ymd.day;

    timeInfo.tm_mday += numberOfDays;

    time_t time = LocalTime::tmToTime(&timeInfo);
    LocalTime::timeToTm(time, &timeInfo);

    fromTimeInfo(&timeInfo);
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
const LocalTimeHMS LocalTimeHMS::startOfDay = LocalTimeHMS("00:00:00");
const LocalTimeHMS LocalTimeHMS::endOfDay = LocalTimeHMS("23:59:59");


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
    return String::format("%02d:%02d:%02d", (int)hour, (int)minute, (int)second);
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

void LocalTimeHMS::fromJson(JSONValue jsonObj) {
    parse(jsonObj.toString().data());
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

void LocalTimeRestrictedDate::clear() {
    onlyOnDays.setMask(0);
    onlyOnDates.clear();
    exceptDates.clear();
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
        bool isValidDays = onlyOnDays.isSet(ymd);
        bool isValidDates = inOnlyOnDates(ymd);
        result = isValidDays || isValidDates;
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

LocalTimeYMD LocalTimeRestrictedDate::getExpirationDate() const {
    LocalTimeYMD result;

    for(auto it = onlyOnDates.begin(); it != onlyOnDates.end(); ++it) {
        if (result.isEmpty() || *it > result) {
            result = *it;
        }
    }
    return result;
}


void LocalTimeRestrictedDate::fromJson(JSONValue jsonObj) {
    JSONObjectIterator iter(jsonObj);
    while(iter.next()) {
        String key = (const char *)iter.name();

        if (key == "y") {
            onlyOnDays.setMask((uint8_t)iter.value().toInt());
        }
        else
        if (key == "a" || key == "x") {
            JSONArrayIterator iter2(iter.value());
            while(iter2.next()){
                LocalTimeYMD ymd(iter2.value().toString().data());
                if (key == "a") {
                    onlyOnDates.push_back(ymd);
                }
                else {
                    exceptDates.push_back(ymd);
                }
            }
        }
    }
    if (isEmpty()) {
        // If there are no restrictions, set to all days
        onlyOnDays.setMask(LocalTimeDayOfWeek::MASK_ALL);
    }
}

// 
// LocalTimeHMSRestricted
//
void LocalTimeHMSRestricted::fromJson(JSONValue jsonObj) {
    // Clearing is necessary in case there is no t key so the fi
    LocalTimeHMS::clear();

    JSONObjectIterator iter(jsonObj);
    while(iter.next()) {
        String key = (const char *) iter.name();
        if (key == "t") {
            LocalTimeHMS::fromJson(iter.value());
        }
    }

    LocalTimeRestrictedDate::fromJson(jsonObj);
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
// LocalTimeScheduleItem
//
bool LocalTimeScheduleItem::getNextScheduledTime(LocalTimeConvert &conv) const {

    LocalTimeConvert tempConv(conv);
    
    LocalTimeYMD endYMD;
    
    LocalTimeYMD expirationDate = getExpirationDate();
    if (expirationDate.isEmpty()) {
        endYMD = tempConv.getLocalTimeYMD();

        // Maximum number of days to look ahead in the schedule for the next scheduled time (default: 32 days)
        endYMD.addDay(LocalTime::instance().getScheduleLookaheadDays());
    }
    else {
        endYMD = expirationDate;
    }
    
    for(;; tempConv.nextDay(LocalTimeHMS::startOfDay)) {
        LocalTimeYMD curYMD = tempConv.getLocalTimeYMD();
        if (curYMD > endYMD) {
            break;
        }

        if (!timeRange.isValidDate(curYMD)) {
            // This is a time range restricted that excludes this date, so skip this date
            continue;
        }

        switch(scheduleItemType) {
        case ScheduleItemType::NONE:
            break;

        case ScheduleItemType::HOUR_OF_DAY:
        case ScheduleItemType::MINUTE_OF_HOUR:
            {
                bool bResult = false;

                int cmp = timeRange.compareTo(tempConv.localTimeValue.hms());
                if (cmp < 0) {
                    // Before time range, return beginning of time range
                    tempConv.atLocalTime(timeRange.hmsStart);
                    conv.time = tempConv.time;
                    conv.convert();
                    return true;
                }
                else 
                if (cmp == 0) {
                    // In time range hmsStart <= hms <= hmsEnd
                    // Handle multiples here
                    struct tm timeInfo;
                    int startingModulo;
                     
                    switch(scheduleItemType) {
                    case ScheduleItemType::HOUR_OF_DAY:
                        // Loop here instead of doing the modulo math to correctly handle timezone and daylight saving switch
                        for(LocalTimeHMS tempHMS = timeRange.hmsStart; tempHMS <= timeRange.hmsEnd; tempHMS.hour += increment) {
                            tempConv.atLocalTime(tempHMS);
                            if (tempConv.time > conv.time) {
                                // Found match
                                bResult = true;
                                break;
                            }
                        }
                        break;

                    case ScheduleItemType::MINUTE_OF_HOUR:
                        // TODO: I think this is wrong for timezones with a minute offset
                        startingModulo = timeRange.hmsStart.minute % increment;

                        tempConv.time += increment * 60;
                        tempConv.convert();

                        LocalTime::timeToTm(tempConv.time, &timeInfo);
                        timeInfo.tm_min -= ((tempConv.localTimeValue.minute() - startingModulo) % increment);
                        timeInfo.tm_sec = timeRange.hmsStart.second;
                        tempConv.time = LocalTime::tmToTime(&timeInfo);
                        tempConv.convert();
                        if (tempConv.getLocalTimeHMS() < timeRange.hmsEnd) {
                            bResult = true;
                        }
                        break;

                    default:
                        break;
                    }

                    if (bResult) {
                        if (!timeRange.isValidDate(tempConv.getLocalTimeYMD())) {
                            bResult = false;
                        }
                    }

                    if (bResult) {
                        conv.time = tempConv.time;
                        conv.convert();
                        return true;
                    }
                }
                else {
                    // cmp > 0, after time range, don't check and try next day
                }
            }
            break;            
            
        case ScheduleItemType::DAY_OF_WEEK_OF_MONTH:
            // "dayOfWeek" specifies the day of the week (0 = Sunday, 1 = Monday, ...)
            // "increment" specifies which one (1 = first, 2 = second, ... or -1 = last, -2 = second to last, ...)
            // Time is at the HMS of the hmsStart (local time)
            {
                int day = LocalTime::dayOfWeekOfMonth(tempConv.localTimeValue.year(), tempConv.localTimeValue.month(), dayOfWeek, increment);
                if (day == tempConv.localTimeValue.day()) {
                    tempConv.atLocalTime(timeRange.hmsStart);
                    if (tempConv.time > conv.time) {
                        conv.time = tempConv.time;
                        conv.convert();
                        return true;
                    }
                }
            }
            break;
            
        case ScheduleItemType::DAY_OF_MONTH:
            {
                int tempIncrement = increment;
                if (tempIncrement < 0) {
                    tempIncrement = LocalTime::lastDayOfMonth(tempConv.localTimeValue.year(), tempConv.localTimeValue.month()) + tempIncrement + 1;
                }
                // "increment" specifies which day of month (1, 2, 3, ...) or 
                // Time is at the HMS of the hmsStart (local time)
                if (tempConv.localTimeValue.ymd().getDay() == tempIncrement) {
                    int cmp = timeRange.compareTo(tempConv.localTimeValue.hms());
                    if (cmp <= 0) {
                        // Before the beginning of time range, return beginning of time range
                        tempConv.atLocalTime(timeRange.hmsStart);
                        if (tempConv.time > conv.time) {
                            conv.time = tempConv.time;
                            conv.convert();
                            return true;
                        }
                    }
                }

            }
            break;

        case ScheduleItemType::TIME: 
            // At a specific time, optionally with day of week or date restrictions            
            // The first test must be <= otherwise you can't schedule at midnight.
            // The second test for conv.time must be > to advance to the next schedule.
            if (tempConv.localTimeValue.hms() <= timeRange.hmsStart) {
                tempConv.atLocalTime(timeRange.hmsStart);
                if (tempConv.time > conv.time) {
                    conv.time = tempConv.time;
                    conv.convert();
                    return true;
                }
            }
            break;

        }

        
    }

    // No next time found (no schedule, or all days excluded within the next getScheduleLookaheadDays() days)
    return false;
}


void LocalTimeScheduleItem::fromJson(JSONValue jsonObj) {
    JSONObjectIterator iter(jsonObj);
    while(iter.next()) {
        String key = (const char *) iter.name();
        if (key == "m") {
            scheduleItemType = (ScheduleItemType) iter.value().toInt();
        }
        else
        if (key == "i") {
            increment = iter.value().toInt();
        }
        else
        if (key == "d") {
            dayOfWeek = iter.value().toInt();
        }
        else
        if (key == "f") {
            flags = iter.value().toInt();
        }
        else
        if (key == "n") {
            name = iter.value().toString().data();
        }
        else
        if (key == "mh") {
            // Shortcut for setting item
            scheduleItemType = ScheduleItemType::MINUTE_OF_HOUR;
            increment = iter.value().toInt();
        }
        else
        if (key == "hd") {
            // Shortcut for setting item
            scheduleItemType = ScheduleItemType::HOUR_OF_DAY;
            increment = iter.value().toInt();
        }
        else
        if (key == "dw") {
            // Shortcut for setting item
            scheduleItemType = ScheduleItemType::DAY_OF_WEEK_OF_MONTH;
            increment = iter.value().toInt();
        }
        else
        if (key == "dm") {
            // Shortcut for setting item
            scheduleItemType = ScheduleItemType::DAY_OF_MONTH;
            increment = iter.value().toInt();
        }
        else
        if (key == "tm") {
            // Shortcut for setting time items
            scheduleItemType = ScheduleItemType::TIME;
            timeRange.hmsStart = LocalTimeHMS(iter.value().toString().data());
            timeRange.onlyOnDays = LocalTimeDayOfWeek::MASK_ALL;
        }
    }

    timeRange.fromJson(jsonObj);
}

//
// LocalTimeSchedule
//


LocalTimeSchedule &LocalTimeSchedule::withMinuteOfHour(int increment, LocalTimeRange timeRange) {
    LocalTimeScheduleItem item;
    item.scheduleItemType = LocalTimeScheduleItem::ScheduleItemType::MINUTE_OF_HOUR;
    item.increment = increment;
    item.timeRange = timeRange;
    scheduleItems.push_back(item);
    return *this;
}


LocalTimeSchedule &LocalTimeSchedule::withHourOfDay(int hourMultiple, LocalTimeRange timeRange) {
    LocalTimeScheduleItem item;
    item.scheduleItemType = LocalTimeScheduleItem::ScheduleItemType::HOUR_OF_DAY;
    item.increment = hourMultiple;
    item.timeRange = timeRange;
    scheduleItems.push_back(item);
    return *this;
}

LocalTimeSchedule &LocalTimeSchedule::withDayOfWeekOfMonth(int dayOfWeek, int instance, LocalTimeRange timeRange) {
    LocalTimeScheduleItem item;
    item.scheduleItemType = LocalTimeScheduleItem::ScheduleItemType::DAY_OF_WEEK_OF_MONTH;
    item.dayOfWeek = dayOfWeek;
    item.increment = instance;
    item.timeRange = timeRange;
    scheduleItems.push_back(item);
    return *this;
}

LocalTimeSchedule &LocalTimeSchedule::withDayOfMonth(int dayOfMonth, LocalTimeRange timeRange) {
    LocalTimeScheduleItem item;
    item.scheduleItemType = LocalTimeScheduleItem::ScheduleItemType::DAY_OF_MONTH;
    item.increment = dayOfMonth;
    item.timeRange = timeRange;
    scheduleItems.push_back(item);
    return *this;
}


LocalTimeSchedule &LocalTimeSchedule::withTime(LocalTimeHMSRestricted hms) {
    LocalTimeScheduleItem item;
    item.scheduleItemType = LocalTimeScheduleItem::ScheduleItemType::TIME;
    item.timeRange.fromTime(hms);
    scheduleItems.push_back(item);
    
    return *this;
}


LocalTimeSchedule &LocalTimeSchedule::withTimes(std::initializer_list<LocalTimeHMSRestricted> timesParam) {

    for(auto it = timesParam.begin(); it != timesParam.end(); ++it) {
        withTime(*it);
    }

    return *this;
}


void LocalTimeSchedule::fromJson(const char *jsonStr) {
    JSONValue outerObj = JSONValue::parseCopy(jsonStr);

    fromJson(outerObj);
}

void LocalTimeSchedule::fromJson(JSONValue jsonArray) {
    JSONArrayIterator iter(jsonArray);
    while(iter.next()) {
        LocalTimeScheduleItem item;
        item.fromJson(iter.value());
        scheduleItems.push_back(item);
    }
}


bool LocalTimeSchedule::getNextScheduledTime(LocalTimeConvert &conv) const {

    return getNextScheduledTime(conv, [](LocalTimeScheduleItem &item) {
        return true;
    });
}

bool LocalTimeSchedule::getNextScheduledTime(LocalTimeConvert &conv, std::function<bool(LocalTimeScheduleItem &item)> filter) const {
    time_t closestTime = 0;

    for(auto it = scheduleItems.begin(); it != scheduleItems.end(); ++it) {
        LocalTimeScheduleItem item = *it;
        if (filter(item)) {
            LocalTimeConvert tmpConvert(conv);
            bool bResult = item.getNextScheduledTime(tmpConvert);
            if (bResult && closestTime == 0 || tmpConvert.time < closestTime) {
                closestTime = tmpConvert.time;
            }
        }
    }
    
    if (closestTime != 0) {
        conv.time = closestTime;
        conv.convert();
        return true;
    }
    else {
        return false;
    }

}


bool LocalTimeSchedule::isScheduledTime() {
    if (!Time.isValid()) {
        return false;
    }

    LocalTimeConvert conv;
    conv.withCurrentTime().convert();
    return isScheduledTime(conv, Time.now());
}

bool LocalTimeSchedule::isScheduledTime(LocalTimeConvert &conv, time_t timeNow) {
    bool result = false;

    if (nextTime != 0 && nextTime <= timeNow) {
        result = true;
        nextTime = 0;
    }

    if (getNextScheduledTime(conv)) {
        nextTime = conv.time;
    }
    
    return result;
}
//
// LocalTimeScheduleManager
//

time_t LocalTimeScheduleManager::getNextTimeByName(const char *name, const LocalTimeConvert &conv) {
    for(auto it = schedules.begin(); it != schedules.end(); ++it) {
        if (it->name.equals(name)) {
            LocalTimeConvert tempConv(conv);
            if (it->getNextScheduledTime(tempConv)) {

            }
        }
    }
    return 0;
}

time_t LocalTimeScheduleManager::getNextWake(const LocalTimeConvert &conv) const {
    time_t nextTime = 0;

    for(auto it = schedules.begin(); it != schedules.end(); ++it) {
        if ((it->flags & LocalTimeSchedule::FLAG_ANY_WAKE) != 0) {
            LocalTimeConvert tempConv(conv);
            if (it->getNextScheduledTime(tempConv)) {
                if (nextTime == 0 || tempConv.time < nextTime) {
                    nextTime = tempConv.time;
                }
            }
        }
    }
    return nextTime;
}

time_t LocalTimeScheduleManager::getNextFullWake(const LocalTimeConvert &conv) const {
    time_t nextTime = 0;

    for(auto it = schedules.begin(); it != schedules.end(); ++it) {
        if ((it->flags & LocalTimeSchedule::FLAG_FULL_WAKE) != 0) {
            LocalTimeConvert tempConv(conv);
            if (it->getNextScheduledTime(tempConv)) {
                if (nextTime == 0 || tempConv.time < nextTime) {
                    nextTime = tempConv.time;
                }
            }
        }
    }
    return nextTime;
}

time_t LocalTimeScheduleManager::getNextDataCapture(const LocalTimeConvert &conv) const {
    time_t nextTime = 0;

    for(auto it = schedules.begin(); it != schedules.end(); ++it) {
        if (it->name.equals("data")) {
            LocalTimeConvert tempConv(conv);
            if (it->getNextScheduledTime(tempConv)) {
                if (nextTime == 0 || tempConv.time < nextTime) {
                    nextTime = tempConv.time;
                }
            }
        }
    }
    return nextTime;
}


void LocalTimeScheduleManager::forEach(std::function<void(LocalTimeSchedule &schedule)> callback) {
    for(auto it = schedules.begin(); it != schedules.end(); ++it) {
        callback(*it);
    }
}

LocalTimeSchedule &LocalTimeScheduleManager::getScheduleByName(const char *name) {

    for(auto it = schedules.begin(); it != schedules.end(); ++it) {
        if (it->name.equals(name)) {
            return *it;
        }
    }

    LocalTimeSchedule sch;
    sch.name = name;
    schedules.push_back(sch);

    return getScheduleByName(name);
}


void LocalTimeScheduleManager::setFromJsonObject(const JSONValue &jsonObj) {
    JSONObjectIterator iter(jsonObj);
    while(iter.next()) {
        String key = (const char *)iter.name();

        for(auto it = schedules.begin(); it != schedules.end(); ++it) {
            if (it->name.equals(key)) {
                it->fromJson(iter.value());
            }
        }
    }
}


//
// LocalTimeRange
// 

time_t LocalTimeRange::getTimeSpan(const LocalTimeConvert &conv) const {
    
    LocalTimeConvert convStart(conv);
    convStart.atLocalTime(hmsStart);

    LocalTimeConvert convEnd(conv);
    convEnd.atLocalTime(hmsEnd);

    if (!rangeCrossesMidnight()) {
        return convEnd.time - convStart.time;
    }
    else {        
        convEnd.nextDay();
        return convEnd.time - convStart.time;
    }
}


void LocalTimeRange::fromJson(JSONValue jsonObj) {    
    JSONObjectIterator iter(jsonObj);
    while(iter.next()) {
        String key = (const char *)iter.name();

        if (key == "s" || key == "e") {
            String hmsStr = iter.value().toString().data();

            if (key == "s") {
                hmsStart = LocalTimeHMS(hmsStr);
            }
            else
            if (key == "e") {
                hmsEnd = LocalTimeHMS(hmsStr);
            }
        }
    }
    LocalTimeRestrictedDate::fromJson(jsonObj);
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

void LocalTimeConvert::nextMinuteMultiple(int increment, int startingModulo) {
    time += increment * 60;

    struct tm timeInfo;

    LocalTime::timeToTm(time, &timeInfo);

    timeInfo.tm_min -= ((timeInfo.tm_min - startingModulo) % increment);
    timeInfo.tm_sec = 0;

    time = LocalTime::tmToTime(&timeInfo);

    convert();
}


void LocalTimeConvert::nextTimeList(std::initializer_list<LocalTimeHMS> _hmsList) {
    std::vector<LocalTimeHMS> hmsList = _hmsList;

    time_t origTime = time;

    nextDay();
    time_t resultTime = time;
    time = origTime;
    convert();

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

// nextTime is used by nextDayMidnight with a HMS of 00:00:00

void LocalTimeConvert::nextTime(LocalTimeHMS hms) {
    time_t origTime = time;

    localTimeValue.setHMS(hms);
    time = localTimeValue.toUTC(config);
    convert();

    if (time <= origTime) {
        // Day rolled backwards, so move back forward
        localTimeValue.tm_mday++;
        time = localTimeValue.toUTC(config);
        convert();
    }
}

void LocalTimeConvert::prevDay(LocalTimeHMS hms) {
    time_t origTime = time;

    localTimeValue.setHMS(hms);
    localTimeValue.tm_mday--;

    time = localTimeValue.toUTC(config);
    convert();
}


void LocalTimeConvert::nextDay(LocalTimeHMS hms) {
    time_t origTime = time;

    localTimeValue.setHMS(hms);
    localTimeValue.tm_mday++;

    time = localTimeValue.toUTC(config);
    convert();
}

void LocalTimeConvert::nextDayOrTimeChange(LocalTimeHMS hms) {
    time_t timeOrig = time;
    time_t dstStartOrig = dstStart;
    time_t standardStartOrig = standardStart;

    nextDay(hms);

    if (dstStartOrig > timeOrig && dstStartOrig < time) {
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

bool LocalTimeConvert::nextSchedule(const LocalTimeSchedule &schedule) {

    return schedule.getNextScheduledTime(*this);
}


void LocalTimeConvert::atLocalTime(LocalTimeHMS hms) {
    if (!hms.ignore) {
        localTimeValue.setHMS(hms);
        time = localTimeValue.toUTC(config);
        convert();
    }
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

// [static]
int LocalTime::dayOfWeekOfMonth(int year, int month, int dayOfWeek, int ordinal) {
    struct tm timeInfo = {0};

    if (dayOfWeek < 0 || dayOfWeek >= 7) {
        // Invalid values of dayOfWeek can cause infinite loop below
        return 0;
    }

    int lastDay = lastDayOfMonth(year, month);

    if (ordinal > 0) {
        timeInfo.tm_year = year - 1900;
        timeInfo.tm_mon = month - 1;
        timeInfo.tm_mday = 1;
        tmToTime(&timeInfo);

        while(timeInfo.tm_wday != dayOfWeek) {
            timeInfo.tm_mday++;
            tmToTime(&timeInfo);
        }

        for(int loops = 1; loops <= 5; loops++) {
            if (loops >= ordinal) {
                return timeInfo.tm_mday;
            }
            timeInfo.tm_mday += 7;
            if (timeInfo.tm_mday > lastDay) {
                // This ordinal does not exist
                return 0;
            }
        }
    }
    else
    if (ordinal < 0) {
        timeInfo.tm_year = year - 1900;
        timeInfo.tm_mon = month - 1;
        timeInfo.tm_mday = lastDay;
        tmToTime(&timeInfo);

        while(timeInfo.tm_wday != dayOfWeek) {
            timeInfo.tm_mday--;
            tmToTime(&timeInfo);
        }
        for(int loops = 1; loops <= 5; loops++) {
            if (loops >= -ordinal) {
                return timeInfo.tm_mday;
            }
            timeInfo.tm_mday -= 7;
            if (timeInfo.tm_mday < 1) {
                // This ordinal does not exist
                return 0;
            }
        }
    }

    
    return 0;
}
