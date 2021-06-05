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

    /**
     * @brief Sets the hour, minute, and second fields from a struct tm
     */
    void fromTimeInfo(const struct tm *pTimeInfo);

    /**
     * @brief Fill in the tm_hour, tm_min, and tm_sec fields of a struct tm from the values in this object
     * 
     * @param pTimeInfo The struct tm to modify
     */
    void toTimeInfo(struct tm *pTimeInfo) const;

    /**
     * @brief Adjust the values in a struct tm from the values in this object
     * 
     * @param pTimeInfo The struct tm to modify
     * 
     * @param subtract If false, the values in hour, minute, second are added to the struct tm. 
     * If true, subtracted
     * 
     * After calling this, the values in the struct tm may be out of range, for example tm_hour > 23. 
     * This is fine, as calling mktime/gmtime normalizes this case and carries out-of-range values
     * into the other fields as necessary.
     */
    void adjustTimeInfo(struct tm *pTimeInfo, bool subtract = false) const;

    int8_t hour = 0;        //!< 0-23 hour (could also be negative)
    int8_t minute = 0;      //!< 0-59 minute
    int8_t second = 0;      //!< 0-59 second
    int8_t ignore = 0;      //!< Special case
};

class LocalTimeIgnoreHMS : public LocalTimeHMS {
public:
    LocalTimeIgnoreHMS() {
        ignore = true;
    }
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
     * @brief Returns the last day of the month in a given month and year
     * 
     * For example, If the month in the current object is January, returns 31.
     * The year is required to handle leap years when the month is February.
     */
    int lastDayOfMonth(int year) const;

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

    bool isValid() const { return valid; };

    bool isZ() const { return !valid || (!hasDST() && standardHMS.toSeconds() == 0); };

    String dstName;
    LocalTimeHMS dstHMS;
    String standardName;
    LocalTimeHMS standardHMS;
    LocalTimeChange dstStart;
    LocalTimeChange standardStart;
    bool valid = false;
};

/**
 * @brief Container for a local time value with accessors similar to the Wiring Time class
 * 
 * Really just a C++ wrapper around struct tm with adjustments for weekday and month being 
 * 0-based in struct tm and 1-based in Wiring. Also tm_year being weird in struct tm.
 * 
 * If you want to format a time string, use the methods in LocalTimeConvert. The reason is
 * that the LocalTimeValue is only the value container and doesn't know the current timezone
 * offset for the local time.
 */
class LocalTimeValue : public ::tm {
public:
    int hour() const { return tm_hour; };

    int hourFormat12() const;

    uint8_t isAM() const { return tm_hour < 12; };

    uint8_t isPM() const { return !isAM(); };

    int minute() const { return tm_min; };

    int second() const { return tm_sec; };

    int day() const { return tm_mday; };

    /**
     * @brief Returns the day of week 1 - 7 (Sunday = 1, Monday = 2, ..., Saturday = 7)
     * 
     * Note: the underlying struct tm tm_wday is 0 - 6 (Sunday = 0, Monday = 1, ..., Saturday = 6)
     * but Wiring uses 1 - 7 instead of 0 - 6.
     */
    int weekday() const { return tm_wday + 1; };

    /**
     * @brief Returns the month of the year 1 - 12 (1 = January, 2 = February, ...)
     * 
     * Note: the underlying struct tm tm_mon is 0 - 11, but this returns the more common 1 - 12.
     */
    int month() const { return tm_mon + 1; };

    /**
     * @brief Returns the 4-digit year
     */
    int year() const { return tm_year + 1900; };

    /**
     * @brief Gets the local time as a LocalTimeHMS object
     */
    LocalTimeHMS hms() const;

    /**
     * @brief Sets the local time from a LocalTimeHMS object
     */
    void setHMS(LocalTimeHMS hms);

    /**
     * @brief Converts the specified local time into a UTC time
     * 
     * There are some caveats to this that occur on when the time change
     * occurs. On spring forward, there is an hour that doesn't technically
     * map to a UTC time. For example, in the United States, 2:00 AM to 3:00 AM
     * local time doesn't exist because at 2:00 AM local time, the local time
     * springs forward to 3:00 AM. 
     * 
     * When falling back, the hour from 1:00 AM to 2:00 AM is not unique, 
     * because it happens twice, once in DST before falling back, and a second
     * time after falling back. The toUTC() function returns the second one
     * that occurs in standard time. 
     */
    time_t toUTC(LocalTimePosixTimezone config) const;

    /**
     * @brief Converts time from ISO-8601 format, ignoring the timezone 
     * 
     * @param str The string to convert
     * 
     * The string should be of the form:
     * 
     * YYYY-MM-DDTHH:MM:SS
     * 
     * The T can be any single character, such as a space. For example: 
     * 
     * 2021-04-01 10:00:00
     * 
     * Any characters after the seconds are ignored.
     */
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

};

/**
 * @brief Perform time conversions. This is the main class you will need.
 */
class LocalTimeConvert {
public:
    /**
     * @brief Whether the specified time is DST or not. See also isDST().
     */
    enum class Position {
        BEFORE_DST,      //!< This time is before the start of DST (northern hemisphere)
        IN_DST,          //!< This time is in daylight saving time (northern hemisphere)
        AFTER_DST,       //!< This time is after the end of DST (northern hemisphere)
        BEFORE_STANDARD, //!< This time is before the start of standard time (southern hemisphere)
        IN_STANDARD,     //!< This time is in standard saving time (southern hemisphere)
        AFTER_STANDARD,  //!< This time is after the end of standard time (southern hemisphere)
        NO_DST,          //!< This config does not use daylight saving
    };

    /**
     * @brief Sets the timezone configuration to use for time conversion
     * 
     * If you do not use withConfig() the global default set in the LocalTime class is used.
     * If neither are set, the local time is UTC (with no DST).
     */
    LocalTimeConvert &withConfig(LocalTimePosixTimezone config) { this->config = config; return *this; };

    /**
     * @brief Sets the UTC time to begin conversion from 
     * 
     * @param time The time (UTC) to set. This is the Unix timestamp (seconds since January 1, 1970) UTC
     * such as returned by Time.now().
     * 
     * This does not start the conversion; you must also call the convert() method after setting
     * all of the settings you want to use.
     * 
     * For the current time, you can instead use withCurrentTime();
     */
    LocalTimeConvert &withTime(time_t time) { this->time = time; return *this; };

    /**
     * @brief Use the current time as the time to start with
     * 
     * This does not start the conversion; you must also call the convert() method after setting
     * all of the settings you want to use.
     */
    LocalTimeConvert &withCurrentTime() { this->time = Time.now(); return *this; };

    /**
     * @brief Do the time conversion
     * 
     * You must call this after changing the configuration or the time using withTime() or withCurrentTime()
     */
    void convert();

    /**
     * @brief Returns true if the current time is in daylight saving time
     */
    bool isDST() const { return position == Position::IN_DST || position == Position::BEFORE_STANDARD || position == Position::AFTER_STANDARD; };

    /**
     * @brief Returns true of the current time in in standard time
     */
    bool isStandardTime() const { return !isDST(); };

    /**
     * @brief Moves the current time to the next day
     * 
     * @param hms If specified, moves to that time of day (local time). If omitted, leaves the current time and only changes the date.
     * 
     * Upon completion, all fields are updated appropriately. For example:
     * - time specifies the time_t of the new time at UTC
     * - localTimeValue contains the broken-out values for the local time
     * - isDST() return true if the new time is in daylight saving time
     */
    void nextDay(LocalTimeHMS hms = LocalTimeIgnoreHMS());

    /**
     * @brief Moves the current time to the next of the specified day of week
     * 
     * @param dayOfWeek The day of week (0 - 6, 0 = Sunday, 1 = Monday, ..., 6 = Saturday)
     * 
     * @param hms If specified, moves to that time of day (local time). If omitted, leaves the current time and only changes the date.
     * 
     * Upon completion, all fields are updated appropriately. For example:
     * - time specifies the time_t of the new time at UTC
     * - localTimeValue contains the broken-out values for the local time
     * - isDST() return true if the new time is in daylight saving time
     */
    bool nextDayOfWeek(int dayOfWeek, LocalTimeHMS hms = LocalTimeIgnoreHMS());

    /**
     * @brief Returns the next day that is a weekday (Monday - Friday)
     * 
     * @param hms If specified, moves to that time of day (local time). If omitted, leaves the current time and only changes the date.
     * 
     * Upon completion, all fields are updated appropriately. For example:
     * - time specifies the time_t of the new time at UTC
     * - localTimeValue contains the broken-out values for the local time
     * - isDST() return true if the new time is in daylight saving time
     */
    void nextWeekday(LocalTimeHMS hms = LocalTimeIgnoreHMS());

    /**
     * @brief Returns the next day that is a weekend day (Saturday or Sunday)
     * 
     * @param hms If specified, moves to that time of day (local time). If omitted, leaves the current time and only changes the date.
     * 
     * Upon completion, all fields are updated appropriately. For example:
     * - time specifies the time_t of the new time at UTC
     * - localTimeValue contains the broken-out values for the local time
     * - isDST() return true if the new time is in daylight saving time
     */
    void nextWeekendDay(LocalTimeHMS hms = LocalTimeIgnoreHMS());

    /**
     * @brief Moves the date and time (local time) forward to the specified day of month and local time
     * 
     * @param hms If specified, moves to that time of day (local time). If omitted, leaves the current time and only changes the date.
     * 
     * This version will move to the closest forward time. It could be as close as 1 second later, but 
     * it will always advance at least once second. It could be as much as 1 month minus 1 second later.
     * 
     * Upon completion, all fields are updated appropriately. For example:
     * - time specifies the time_t of the new time at UTC
     * - localTimeValue contains the broken-out values for the local time
     * - isDST() return true if the new time is in daylight saving time
     */
    bool nextDayOfMonth(int dayOfMonth, LocalTimeHMS hms = LocalTimeIgnoreHMS());

    /**
     * @brief Moves the date and time (local time) forward to the specified day of month and local time
     * 
     * @param hms If specified, moves to that time of day (local time). If omitted, leaves the current time and only changes the date.
     * 
     * This version always picks the next month, even if the target day of month hasn't been reached
     * in this month yet. This will always more forward at least a month, and may be as much as 
     * two months minus one day.
     * 
     * Upon completion, all fields are updated appropriately. For example:
     * - time specifies the time_t of the new time at UTC
     * - localTimeValue contains the broken-out values for the local time
     * - isDST() return true if the new time is in daylight saving time
     */
    void nextDayOfNextMonth(int dayOfMonth, LocalTimeHMS hms = LocalTimeIgnoreHMS());


    /**
     * @brief Moves the date and time (local time) forward to the specified day of month and local time
     * 
     * @param dayOfWeek The day of week (0 - 6, 0 = Sunday, 1 = Monday, ..., 6 = Saturday)
     * 
     * @param ordinal 1 = first of that day of week in month, 2 = second, ...
     * 
     * @param hms If specified, moves to that time of day (local time). If omitted, leaves the current time and only changes the date.
     * 
     * If you specify an ordinal that does not exist (for example, there may not be a 5th ordinal for certain days
     * in certain months), returns false and leaves the date unchanged.
     * 
     * Upon successful completion, all fields are updated appropriately. For example:
     * - time specifies the time_t of the new time at UTC
     * - localTimeValue contains the broken-out values for the local time
     * - isDST() return true if the new time is in daylight saving time
     */
    bool nextDayOfWeekOrdinal(int dayOfWeek, int ordinal, LocalTimeHMS hms = LocalTimeIgnoreHMS());

    /**
     * @brief Sets the time to the nearest hms in local time in the future.
     * 
     * @param hms The time of day to set in local time
     * 
     * Moves the time forward to the next instance of hms in local time. If hms has not
     * occurred yet, it will select the one today, but it will always move the time forward.
     * 
     * There is a weird special case on the beginning of daylight saving (spring forward in
     * the northern hemisphere). If you select a hms between 2:00 AM and 2:59:59 AM local time,
     * this time does not exist on spring forward day because it's the hour that is skipped.
     * In order to preserve the requirement that the time will always be advanced by this
     * call, it will jump forward to the next day when the 2:00 hour occurs next. (The hour
     * may be different in other locations, for example it's 1:00 AM in the UK.)
     * 
     * In the case of fall back, if you specify a hms in the repeated hour (1:00:00 to 1:59:59)
     * the time returned will be the second time this time occurs, in standard time in the US.
     * 
     * Upon completion, all fields are updated appropriately. For example:
     * - time specifies the time_t of the new time at UTC
     * - localTimeValue contains the broken-out values for the local time
     * - isDST() return true if the new time is in daylight saving time
     */    
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
     * 
     * Upon completion, all fields are updated appropriately. For example:
     * - time specifies the time_t of the new time at UTC
     * - localTimeValue contains the broken-out values for the local time
     * - isDST() return true if the new time is in daylight saving time
     */
    void atLocalTime(LocalTimeHMS hms);

    /**
     * @brief Works like Time.timeStr() to generate a readable string of the local time
     * 
     * Uses asctime formatting, which looks like "Fri Jan  1 18:45:56 2021". The strings
     * are not localized; they're always in English.
     */
    String timeStr();

    /**
     * @brief Works like Time.format()
     * 
     * @param formatSpec the format specifies, which can be 
     * 
     * - TIME_FORMAT_DEFAULT (example: "Thu Apr  1 12:00:00 2021")
     * - TIME_FORMAT_ISO8601_FULL (example: "2021-04-01T12:00:00-04:00")
     * - custom format based on strftime()
     * 
     * There are many options to strftime described here: 
     * https://www.cplusplus.com/reference/ctime/strftime/?kw=strftime
     * 
     * Unlike Time.format(), you can use %Z to output the timezone abbreviation,
     * for example "EDT" for the Eastern United States, daylight saving instead 
     * of -04:00.
     * 
     * The %z formatting matches that of Time.format(), which is wrong. The
     * correct output should be "-400" but the output will be "-04:00" 
     * for compatibility.
     */
    String format(const char* formatSpec);

    /**
     * @brief Returns the abbreviated time zone name for the current time
     * 
     * For example, for the United States east coast, EST or EDT depending on
     * whether the current time is DST or not. See also isDST().
     * 
     * This string comes from the LocalTimePosixTimezone object.
     */
    String zoneName() const;

    /**
     * @brief Where time is relative to DST
     */
    Position position = Position::NO_DST;

    /**
     * @brief Timezone configuration for this time conversion
     * 
     * If you don't specify this using withConfig then the global setting is retrieved
     * from the LocalTime singleton instance.
     */
    LocalTimePosixTimezone config;

    /**
     * @brief The time that is being converted. This is always Unix time at UTC
     * 
     * Seconds after January 1, 1970, UTC. Using methods like nextDay() increment
     * this value.
     */
    time_t time;

    /**
     * @brief The local time that corresponds to time
     * 
     * The convert() method sets this, as do methods like nextDay(). The local time
     * will depend on the timezone set in config, as well as the date which will
     * determine whether it's daylight saving or not.
     */
    LocalTimeValue localTimeValue;

    /**
     * @brief The time that daylight saving starts, Unix time, UTC
     * 
     * The config specifies the rule (2nd Sunday in March, for example), and the
     * local time that the change occurs. This is the UTC value that corresponds
     * to that rule in the year specified by time.
     * 
     * Note in the southern hemisphere, dstStart is after standardStart.
     */
    time_t dstStart;

    /**
     * @brief The struct tm that corresponds to dstStart (UTC)
     */
    struct tm dstStartTimeInfo;

    /**
     * @brief The time that standard time starts, Unix time, UTC
     * 
     * The config specifies the rule (1st Sunday in November, for example), and the
     * local time that the change occurs. This is the UTC value that corresponds
     * to that rule in the year specified by time.
     * 
     * Note in the southern hemisphere, dstStart is after standardStart.
     */
    time_t standardStart;

    /**
     * @brief The struct tm that corresponds to standardStart (UTC)
     */
    struct tm standardStartTimeInfo;
};



class LocalTime {
public:
    static LocalTime &instance();

    /**
     * @brief Sets the default global timezone configuration
     */
    LocalTime &withConfig(LocalTimePosixTimezone config) { this->config = config; return *this; };

    const LocalTimePosixTimezone &getConfig() const { return config; };

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

protected:
    LocalTimePosixTimezone config;

    static LocalTime *_instance;
};


#endif /* __LOCALTIMERK_H */
