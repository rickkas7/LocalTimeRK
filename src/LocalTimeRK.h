#ifndef __LOCALTIMERK_H
#define __LOCALTIMERK_H

#include "Particle.h"

#include <time.h>

class LocalTimeHMS {
public:
    LocalTimeHMS();
    virtual ~LocalTimeHMS();

    LocalTimeHMS(const char *str);

    /**
     * @brief Sets the hour, minute, and second to 0
     */
    void clear();

    /**
     * @brief Parse a "H:MM:SS" string
     * 
     * @param str Input string
     * 
     * Multiple formats are supported, and parts are optional:
     * 
     * - H:MM:SS  (examples: "2:00:00" or "2:0:0")
     * - H:MM     (examples: "2:00" or "2:0")
     * - H        (examples: "2")
     * 
     * Hours are always 0 - 23 (24-hour clock). Can also be a negative hour -1 to -23.
     */
    void parse(const char *str);

    /**
     * @brief Turns the parsed data into a normalized string of the form: "H:MM:SS" (24-hour clock)
     */
    String toString() const;

    /**
     * @brief Convert hour minute second into a number of seconds (simple multiplication and addition)
     */
    int toSeconds() const;

    void fromTimeInfo(const struct tm *pTimeInfo);

    void toTimeInfo(struct tm *pTimeInfo) const;

    void adjustTimeInfo(struct tm *pTimeInfo, bool subtract = false) const;

    void adjustTimeInfoPreserveDate(struct tm *pTimeInfo, bool subtract = false) const;

    int8_t hour = 0;        //!< 0-23 hour (could also be negative)
    int8_t minute = 0;      //!< 0-59 minute
    int8_t second = 0;      //!< 0-59 second
    int8_t reserved = 0;    //!< reserved for future use (here for alignment)
};

/**
 * @brief Handles the time change part of the Posix timezone string like "M3.2.0/2:00:00"
 * 
 * Other formats with shortened time of day are also allowed like "M3.2.0/2" or even 
 * "M3.2.0" (midnight) are also allowed. Since the hour is local time, it can also be
 * negative "M3.2.0/-1".
 */
class LocalTimeChange {
public:
    LocalTimeChange();
    virtual ~LocalTimeChange();

    LocalTimeChange(const char *str);

    void clear();

    // M3.2.0/2:00:00
    void parse(const char *str);

    /**
     * @brief Turns the parsed data into a normalized string like "M3.2.0/2:00:00"
     */
    String toString() const;

    /**
     * @brief Calculate the time change in a given year
     * 
     * On input, pTimeInfo must have a valid tm_year (121 = 2021) set. 
     * 
     * On output, all struct tm values are set appropriately with UTC
     * values of when the time change occurs.
     */
    time_t calculate(struct tm *pTimeInfo, LocalTimeHMS tzAdjust) const;

    int8_t month = 0;       //!< 1-12, 1=January
    int8_t week = 0;        //!< 1-5, 1=first
    int8_t dayOfWeek = 0;   //!< 0-6, 0=Sunday, 1=Monday, ...
    int8_t valid = 0;       //!< true = valid
    LocalTimeHMS hms;
};

/**
 * @brief Parses a Posix timezone string into its component parts
 * 
 * For the Eastern US timezone, the string is: "EST5EDT,M3.2.0/2:00:00,M11.1.0/2:00:00"
 * 
 * - EST is the standard timezone name
 * - 5 is the offset in hours (the sign is backwards from the normal offset from UTC)
 * - EDT is the daylight saving timezone name
 * - M3 indicates that DST starts on the 3rd month (March)
 * - 2 is the week number (second week)
 * - 0 is the day of week (0 = Sunday)
 * - 2:00:00 at 2 AM local time, the transition occurs
 * - M11 indicates that standard time begins in the 11th month (November)
 * - 1 is the week number (first week)
 * - 0 is the day of week (0 = Sunday)
 * - 2:00:00 at 2 AM local time, the transition occurs
 * 
 * There are many other acceptable formats, including formats for locations that don't have DST.
 */
class LocalTimePosixTimezone {
public:
    LocalTimePosixTimezone();
    virtual ~LocalTimePosixTimezone();

    LocalTimePosixTimezone(const char *str);

    void clear();

    void parse(const char *str);

    String toString() const;

    bool hasDST() const { return dstStart.valid; };

    String dstName;
    LocalTimeHMS dstHMS;
    String standardName;
    LocalTimeHMS standardHMS;
    LocalTimeChange dstStart;
    LocalTimeChange standardStart;
};

/**
 * @brief Container for a local time value with accessors similar to the Wiring Time class
 * 
 * Really just a C++ wrapper around struct tm with adjustments for weekday and month being 
 * 0-based in struct tm and 1-based in Wiring. Also tm_year being weird in struct tm.
 */
class LocalTimeValue {
public:
    int hour() const { return localTimeInfo.tm_hour; };

    int hourFormat12() const;

    uint8_t isAM() const { return localTimeInfo.tm_hour < 12; };

    uint8_t isPM() const { return !isAM(); };

    int minute() const { return localTimeInfo.tm_min; };

    int second() const { return localTimeInfo.tm_sec; };

    int day() const { return localTimeInfo.tm_mday; };

    int weekday() const { return localTimeInfo.tm_wday + 1; };

    int month() const { return localTimeInfo.tm_mon + 1; };

    int year() const { return localTimeInfo.tm_year + 1900; };

    // TODO: add format()

    LocalTimeHMS hms() const;

    void setHMS(LocalTimeHMS hms);

    time_t toUTC(LocalTimePosixTimezone config) const;

    void fromString(const char *str);

    /**
     * @brief Returns which week of this day it is
     * 
     * For example, if this day is a Friday and it's the first Friday of the month, then
     * 1 is returned. If it's the second Friday, then 2 is returned.
     * 
     * (This is different than the week number of the month, which depends on which day
     * you begin the week on.)
     */
    int ordinal() const;

    struct tm localTimeInfo;
};

class LocalTimeConvert {
public:
    enum class Position {
        BEFORE_DST,
        IN_DST,
        AFTER_DST,
        NO_DST
    };

    LocalTimeConvert();
    virtual ~LocalTimeConvert();

    LocalTimeConvert &withConfig(LocalTimePosixTimezone config) { this->config = config; return *this; };

    /**
     * @brief Sets the UTC time to begin conversion from 
     * 
     * For the current time, you can instead use withCurrentTime();
     */
    LocalTimeConvert &withTime(time_t time) { this->time = time; return *this; };

    LocalTimeConvert &withCurrentTime() { this->time = Time.now(); return *this; };

    void convert();

    bool isDST() const { return position == Position::IN_DST; };
    bool isStandardTime() const { return !isDST(); };

    void nextDay();
    void nextDay(LocalTimeHMS hms);

    bool nextDayOfWeek(int dayOfWeek, LocalTimeHMS hms);

    void nextWeekday(LocalTimeHMS hms);
    void nextWeekendDay(LocalTimeHMS hms);

    /**
     * @brief Moves the date and time (local time) forward to the specified day of month and local time
     * 
     * This version will move to the closest forward time. It could be as close as 1 second later, but 
     * it will always advance at least once second. It could be as much as 1 month minus 1 second later.
     */
    bool nextDayOfMonth(int dayOfMonth, LocalTimeHMS hms);

    /**
     * @brief Moves the date and time (local time) forward to the specified day of month and local time
     * 
     * This version always picks the next month, even if the target day of month hasn't been reached
     * in this month yet. This will always more forward at least a month, and may be as much as 
     * two months minus one day.
     */
    void nextDayOfNextMonth(int dayOfMonth, LocalTimeHMS hms);

    bool nextDayOfWeekOrdinal(int dayOfWeek, int ordinal, LocalTimeHMS hms);

    //void nextDayOfMonth(int month, int dayOfMonth, LocalTimeHMS hms);

    // nextDayOfWeekInNextMonth(int dayOfWeek, int ordinal)
    // nextDayOfWeekInMonth(int month, int dayOfWeek, int ordinal)
    
    void nextLocalTime(LocalTimeHMS hms);

    /**
     * @brief Changes the time of day to the specified hms in local time on the same local day
     * 
     * @param hms A LocalTimeHMS object with hour, minute, and second
     * 
     * You can use the string constructor like this to set the time to 2:00 PM local time.
     * 
     * ```
     * converter.atLocalTime(LocalTimeHms("14:00:00"));
     * ```
     * 
     * It's possible that this will set the time to a time earlier than the object's current
     * time. To only set a time in the future, use nextLocalTime() instead.
     */
    void atLocalTime(LocalTimeHMS hms);


    Position position = Position::NO_DST;
    LocalTimePosixTimezone config;
    time_t time;
    LocalTimeValue localTimeValue;
    time_t dstStart;
    struct tm dstStartTimeInfo;
    time_t standardStart;
    struct tm standardStartTimeInfo;
};



class LocalTime {
public:
    LocalTime();
    virtual ~LocalTime();

    // POSIX timezone strings
    // https://developer.ibm.com/technologies/systems/articles/au-aix-posix/
    // Can use zdump for unit test!
    
    /**
     * @brief Converts a Unix time (seconds past Jan 1 1970) UTC value to a struct tm
     * 
     * @param time Unix time (seconds past Jan 1 1970) UTC
     * 
     * @param pTimeTime Pointer to a struct tm that is filled in with the time broken out
     * into components
     * 
     * The struct tm contains the following members:
     * - tm_sec seconds (0-59). Could in theory be up to 61 with leap seconds, but never is
     * - tm_min minute (0-59)
     * - tm_hour hour (0-23)
     * - tm_mday day of month (1-31)
     * - tm_mon month (0-11). This is 0-11 not 1-12! Beware!
     * - tm_year year since 1900. Note: 2021 is 121, not 2021 or 21! Beware!
     * - tm_wday Day of week (Sunday = 0, Monday = 1, Tuesday = 2, ..., Saturday = 6)
     * - tm_yday Day of year (0 - 365). Note: zero-based, January 1 = 0
     * - tm_isdst Daylight saving flag, always 0 on Particle devices
     * 
     * This basically a wrapper around localtime_r or gmtime_r.
     */
    static void timeToTm(time_t time, struct tm *pTimeInfo);

    /**
     * @brief Converts a struct tm to a Unix time (seconds past Jan 1 1970) UTC
     * 
     * @param pTimeTime Pointer to a struct tm
     * 
     * Note: tm_wday, tm_yday, and tm_isdst are ignored for calculating the result,
     * however tm_wday and tm_yday are filled in with the correct values based on
     * the date, which is why pTimeInfo is not const.
     * 
     * This basically a wrapper around mktime or timegm.
     */
    static time_t tmToTime(struct tm *pTimeInfo);

    /**
     * @brief Returns a human-readable string version of a struct tm
     */
    static String getTmString(struct tm *pTimeInfo);

    static time_t stringToTime(const char *str, struct tm *pTimeInfo = NULL);


    static String timeToString(time_t time, char separator = ' ');
};


#endif /* __LOCALTIMERK_H */
