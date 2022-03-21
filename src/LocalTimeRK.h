#ifndef __LOCALTIMERK_H
#define __LOCALTIMERK_H

#include "Particle.h"

#include <time.h>
#include <initializer_list>
#include <vector>

class LocalTimeValue;

/**
 * @brief Class for holding a year month day efficiently (4 bytes of storage)
 * 
 * There is no method to get this object from a time_t because time_t is at UTC
 * and this object is intended to be the YMD at local time to correspond with
 * a LocalTimeHMS. Thus it requires a LocalTimeConvert object, and there is
 * a method to get a LocalTimeYMD from a LocalTimeConvert, not from this object.
 */
class LocalTimeYMD {
public:
    struct YMD {
        unsigned year:23;    // Add 1900 (like struct tm)
        unsigned month:4;    // 1 - 12 (not like struct tm which is 0 - 11)
        unsigned day:5;      // 1 - 31 ish
    };

    LocalTimeYMD() {
        ymd.year = ymd.month = ymd.day = 0;
    }

    LocalTimeYMD(const char *s) {
        (void) parse(s);
    }

    LocalTimeYMD(const LocalTimeValue &value) {
        fromLocalTimeValue(value);
    }


    bool isEmpty() const {
        return ymd.year == 0 && ymd.month == 0 && ymd.day == 0;
    }

    /**
     * @brief Get the year as a 4-digit year, for example: 2022
     * 
     * @return int 
     */
    int getYear() const {
        return ymd.year + 1900;
    }

    void setYear(int year) {
        if (year < 100) {
            ymd.year = year + 100;
        }
        else
        if (year < 999) {
            ymd.year = year;
        }
        else {
            ymd.year = year - 1900;
        }
    }
    int getMonth() const {
        return ymd.month;
    }
    void setMonth(int month) {
        ymd.month = month;
    }
    int getDay() const {
        return ymd.day;
    }
    void setDay(int day) {
        ymd.day = day;
    }
    /**
     * @brief Copies the year, month, and day from a struct tm
     * 
     * @param pTimeInfo The pointer to a struct tm to copy the year, month, and day from.
     * 
     * The tm should be in local time. 
     */
    void fromTimeInfo(const struct tm *pTimeInfo);

    /**
     * @brief The LocalTimeValue to copy the year, month and day from
     * 
     * @param value Source of the year, month, and day values
     * 
     * Since LocalTimeValue contains a struct tm, this uses fromTimeInfo internally.
     */
    void fromLocalTimeValue(const LocalTimeValue &value);

    /**
     * @brief Add a number of days to the current YMD (updating month or year as necessary)
     * 
     * @param numberOfDays Number of days to add (positive) or subtract (negative)
     * 
     * Works correctly with leap years.
     */
    void addDay(int numberOfDays = 1);

    int getDayOfWeek() const;

    int compareTo(const LocalTimeYMD other) const;

    bool operator==(const LocalTimeYMD other) const {
        return compareTo(other) == 0; 
    }

    bool operator!=(const LocalTimeYMD other) const {
        return compareTo(other) != 0; 
    }

    bool operator<(const LocalTimeYMD other) const {
        return compareTo(other) < 0; 
    }

    bool operator<=(const LocalTimeYMD other) const {
        return compareTo(other) <= 0; 
    }

    bool operator>(const LocalTimeYMD other) const {
        return compareTo(other) > 0; 
    }

    bool operator>=(const LocalTimeYMD other) const {
        return compareTo(other) >= 0; 
    }

    /**
     * @brief Parse a YMD string in the format "YYYY-MD-DD". Only this format is supported!
     * 
     * @param s 
     * @return true 
     * @return false 
     * 
     * Do not use this function with other date formats like "mm/dd/yyyy"!
     */
    bool parse(const char *s);

    String toString() const {
        return String::format("%04d-%02d-%02d", ymd.year + 1900, ymd.month, ymd.day);
    }

    YMD ymd;
};

/**
 * @brief Class for managing day of week calculations
 * 
 * Day 0 = Sunday, 1 = Monday, ..., 6 = Saturday
 */
class LocalTimeDayOfWeek {
public:
    LocalTimeDayOfWeek() {        
    }

    LocalTimeDayOfWeek(uint8_t mask) : dayOfWeekMask(mask) {
    }

    LocalTimeDayOfWeek &withWeekdays() {
        dayOfWeekMask = MASK_WEEKDAY;
        return *this;
    }
    LocalTimeDayOfWeek &withWeekends() {
        dayOfWeekMask = MASK_WEEKEND;
        return *this;
    }
    LocalTimeDayOfWeek &withAllDays() {
        dayOfWeekMask = MASK_ALL;
        return *this;
    }

    /**
     * @brief Returns true if the specified dayOfWeek is set in the mask
     * 
     * @param dayOfWeek Same as struct tm. 0 < dayOfWeek <= 6. Sunday = 0. 
     * @return true 
     * @return false 
     */
    bool isSet(int dayOfWeek) const {
        return (dayOfWeekMask & (1 << dayOfWeek)) != 0;
    }
    bool isSet(LocalTimeYMD ymd) const {
        int dayOfWeek = ymd.getDayOfWeek();
        bool result = isSet(dayOfWeek);
        return result;
    }
    bool isEmpty() const {
        return dayOfWeekMask == 0;
    }
    uint8_t getMask() const {
        return dayOfWeekMask;
    }
    void setMask(uint8_t mask) {
        dayOfWeekMask = mask;
    }

    String toString() const {
        return String::format("LocalTimeDayOfWeek dayOfWeekMask=%02x", dayOfWeekMask);
    }

    static const uint8_t MASK_SUNDAY = 0x01;
    static const uint8_t MASK_MONDAY = 0x02;
    static const uint8_t MASK_TUESDAY = 0x04;
    static const uint8_t MASK_WEDNESDAY = 0x08;
    static const uint8_t MASK_THURSDAY = 0x10;
    static const uint8_t MASK_FRIDAY = 0x20;
    static const uint8_t MASK_SATURDAY = 0x40;

    static const uint8_t MASK_ALL = MASK_SUNDAY | MASK_MONDAY | MASK_TUESDAY | MASK_WEDNESDAY | MASK_THURSDAY | MASK_FRIDAY | MASK_SATURDAY | MASK_SUNDAY; // 0x7f = 127
    static const uint8_t MASK_WEEKDAY = MASK_MONDAY | MASK_TUESDAY | MASK_WEDNESDAY | MASK_THURSDAY | MASK_FRIDAY; // 0x3e = 62
    static const uint8_t MASK_WEEKEND = MASK_SATURDAY | MASK_SUNDAY; // 0x41 = 65

    uint8_t dayOfWeekMask = 0;
};

/**
 * @brief Container for holding an hour minute second time value
 */
class LocalTimeHMS {
public:
    /**
     * @brief Default constructor. Sets time to 00:00:00
     */
    LocalTimeHMS();

    /**
     * @brief Destructor
     */
    virtual ~LocalTimeHMS();

    /**
     * @brief Constructs the object from a time string
     * 
     * @param str The time string
     * 
     * The time string is normally of the form HH:MM:SS, such as "04:00:00" for 4:00 AM.
     * The hour is in 24-hour format. Other formats are supported as well, including 
     * omitting the seconds (04:00), or including only the hour "04", or omitting the
     * leadings zeros (4:0:0). 
     * 
     * Additionally, the hour could be negative, used in UTC DST offsets. The minute
     * and second are always positive (0-59). The hour could also be > 24 when used
     * as a timezone offset.
     */
    LocalTimeHMS(const char *str);

    LocalTimeHMS(const LocalTimeValue &value);

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
     * @brief Turns the parsed data into a normalized string of the form: "HH:MM:SS" (24-hour clock, with leading zeroes)
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

    void fromLocalTimeValue(const LocalTimeValue &value);

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
     *      * 
     * After calling this, the values in the struct tm may be out of range, for example tm_hour > 23. 
     * This is fine, as calling mktime/gmtime normalizes this case and carries out-of-range values
     * into the other fields as necessary.
     */
    void adjustTimeInfo(struct tm *pTimeInfo) const;

    /**
     * @brief Parses a JSON value of type string in HH:MM:SS format
     * 
     * @param jsonObj 
     */
    void fromJson(JSONValue jsonObj);


    /**
     * @brief Sets this object to be the specified hour, with minute and second set to 0
     * 
     * @param hour 0 <= hour < 24
     * @return LocalTimeHMS& 
     */
    LocalTimeHMS &withHour(int hour) {
        this->hour = hour;
        this->minute = this->second = 0;
        return *this;
    }

    /**
     * @brief Sets this object to be the specified hour and minute, with second set to 0
     * 
     * @param hour 0 <= hour < 24
     * @param minute 0 <= minute < 60
     * @return LocalTimeHMS& 
     */
    LocalTimeHMS &withHourMinute(int hour, int minute) {
        this->hour = hour;
        this->minute = minute;
        this->second = 0;
        return *this;
    }

    /**
     * @brief Compare two LocalTimeHMS objects
     * 
     * @param other The item to compare to
     * @return int -1 if this item is < other; 0 if this = other, or +1 if this > other
     */
    int compareTo(const LocalTimeHMS &other) const {
        if (hour < other.hour) {
            return -1;
        }
        else
        if (hour > other.hour) {
            return +1;
        }
        else {
            if (minute < other.minute) {
                return -1;
            }
            else 
            if (minute > other.minute) {
                return +1;
            }
            else {
                if (second < other.second) {
                    return -1;
                }
                else 
                if (second > other.second) {
                    return +1;
                }
                else { 
                    return 0;
                }
            }
        }
    }

    /**
     * @brief Returns true if this item is equal to other. Compares hour, minute, and second.
     * 
     * @param other 
     * @return true 
     * @return false 
     */
    bool operator==(const LocalTimeHMS &other) const {
        return compareTo(other) == 0;
    }

    /**
     * @brief Returns true if this item is not equal to other.
     * 
     * @param other 
     * @return true 
     * @return false 
     */
    bool operator!=(const LocalTimeHMS &other) const {
        return compareTo(other) != 0;
    }

    /**
     * @brief Returns true if this item is < other. 
     * 
     * @param other 
     * @return true 
     * @return false 
     */
    bool operator<(const LocalTimeHMS &other) const {
        return compareTo(other) < 0;
    }

    /**
     * @brief Returns true if this item is > other.
     * 
     * @param other 
     * @return true 
     * @return false 
     */
    bool operator>(const LocalTimeHMS &other) const {
        return compareTo(other) > 0;
    }

    /**
     * @brief Returns true if this item <= other
     * 
     * @param other 
     * @return true 
     * @return false 
     */
    bool operator<=(const LocalTimeHMS &other) const {
        return compareTo(other) <= 0;
    }

    /**
     * @brief Returns true if this item is >= other
     * 
     * @param other 
     * @return true 
     * @return false 
     */
    bool operator>=(const LocalTimeHMS &other) const {
        return compareTo(other) >= 0;
    }


    int8_t hour = 0;        //!< 0-23 hour (could also be negative)
    int8_t minute = 0;      //!< 0-59 minute
    int8_t second = 0;      //!< 0-59 second
    int8_t ignore = 0;      //!< Special case
};

/**
 * @brief This class can be passed to most functions that take a LocalTimeHMS to instead not set the HMS
 */
class LocalTimeIgnoreHMS : public LocalTimeHMS {
public:
    /**
     * @brief Special version of LocalTimeHMS that does not set the HMS
     */
    LocalTimeIgnoreHMS() {
        ignore = true;
    }

    virtual String toString() const {
        return String::format("LocalTimeIgnoreHMS ignore=%d", ignore);
    }
};

/**
 * @brief Day of week, date, or date exception restrictions
 *
 * This class can specify that something (typically a LocalTimeHMSRestricted or a LocalTimeRange) only
 * applies on certain dates. This can be a mask of days of the week, optionally with specific
 * dates that should be disallowed. Or you can schedule only on specific dates. 
 */
class LocalTimeRestrictedDate {
public:
    /**
     * @brief Create an empty restricted date object. It will return false for any date passed to isValid.
     */
    LocalTimeRestrictedDate() {
    }

    /**
     * @brief Create a date restricted object restricted to days of the week
     * 
     * @param mask The days of the week to enable. Pass LocalTimeDayOfWeek::MASK_ALL to allow on every day (no restrictions)
     */
    LocalTimeRestrictedDate(uint8_t mask) : onlyOnDays(mask) {        
    }

    LocalTimeRestrictedDate(uint8_t mask, std::initializer_list<const char *> onlyOnDates, std::initializer_list<const char *> exceptDates) : onlyOnDays(mask) {   
        withOnlyOnDates(onlyOnDates);
        withExceptDates(exceptDates);
    }

    LocalTimeRestrictedDate(uint8_t mask, std::initializer_list<LocalTimeYMD> onlyOnDates, std::initializer_list<LocalTimeYMD> exceptDates) : onlyOnDays(mask) {   
        withOnlyOnDates(onlyOnDates);
        withExceptDates(exceptDates);
    }

    /**
     * @brief Restrict to days of the week
     * 
     * @param value A LocalTimeDayOfWeek object specifying the days of the week (mask bits for Sunday - Saturday)
     * @return LocalTimeRestrictedDate& 
     * 
     * A day of the week is allowed if the day of week mask bit is set.
     * If a date is in the except dates list, then isValid return false.
     * If a date is in the only on days mask OR only on dates list, then isValid will return true.
     */
    LocalTimeRestrictedDate &withOnlyOnDays(LocalTimeDayOfWeek value);

    /**
     * @brief Restrict to certain dates
     * 
     * @param mask Mask value, such as  LocalTimeDayOfWeek::MASK_MONDAY
     * @return LocalTimeRestrictedDate& 
     * 
     * A day of the week is allowed if the day of week mask bit is set.
     * If a date is in the except dates list, then isValid return false.
     * If a date is in the only on days mask OR only on dates list, then isValid will return true.
     */
    LocalTimeRestrictedDate &withOnlyOnDays(uint8_t mask) {
        return withOnlyOnDays(LocalTimeDayOfWeek(mask));
    }

    /**
     * @brief Restrict to certain dates
     * 
     * @param dates A {} list of strings of the form YYYY-MM-DD. No other date formats are allowed!
     * @return LocalTimeRestrictedDate& 
     * 
     * If a date is in the except dates list, then isValid return false.
     * If a date is in the only on days mask OR only on dates list, then isValid will return true.
     */
    LocalTimeRestrictedDate &withOnlyOnDates(std::initializer_list<const char *> dates);

    /**
     * @brief Restrict to certain dates
     * 
     * @param dates A {} list of LocalTimeYMD objects
     * @return LocalTimeRestrictedDate& 
     * 
     * If a date is in the except dates list, then isValid return false.
     * If a date is in the only on days mask OR only on dates list, then isValid will return true.
     */
    LocalTimeRestrictedDate &withOnlyOnDates(std::initializer_list<LocalTimeYMD> dates);

    /**
     * @brief Dates that will always return false for isValid
     * 
     * @param dates A {} list of strings of the form YYYY-MM-DD. No other date formats are allowed!
     * @return LocalTimeRestrictedDate& 
     * 
     * If a date is in the except dates list, then isValid return false.
     * If a date is in the only on days mask OR only on dates list, then isValid will return true.
     */
    LocalTimeRestrictedDate &withExceptDates(std::initializer_list<const char *> dates);

    /**
     * @brief Dates that will always return false for isValid
     * 
     * @param dates A {} list of LocalTimeYMD objects
     * @return LocalTimeRestrictedDate& 
     * 
     * If a date is in the except dates list, then isValid return false.
     * If a date is in the only on days mask OR only on dates list, then isValid will return true.
     */
    LocalTimeRestrictedDate &withExceptDates(std::initializer_list<LocalTimeYMD> dates);

    /**
     * @brief Returns true if onlyOnDays mask is 0 and the onlyOnDates and exceptDates lists are empty
     * 
     * @return true 
     * @return false 
     */
    bool isEmpty() const;

    /**
     * @brief Clear all settings
     */
    void clear();

    /**
     * @brief Returns true if a date is in the onlyOnDays or onlyOnDates list, and not in the exceptDates list
     * 
     * @param localTimeValue Date to check (local time)
     * @return true 
     * @return false 
     */
    bool isValid(LocalTimeValue localTimeValue) const;

    /**
     * @brief Returns true if a date is in the onlyOnDays or onlyOnDates list, and not in the exceptDates list
     * 
     * @param ymd Date to check (local time)
     * @return true 
     * @return false 
     */
    bool isValid(LocalTimeYMD ymd) const;

    /**
     * @brief Returns true of a date is in the onlyOnDates list
     * 
     * @param ymd 
     * @return true 
     * @return false 
     */
    bool inOnlyOnDates(LocalTimeYMD ymd) const;

    /**
     * @brief Returns true of a date is in the exceptDates list
     * 
     * @param ymd 
     * @return true 
     * @return false 
     */
    bool inExceptDates(LocalTimeYMD ymd) const;

    /**
     * @brief Fills in this object from JSON data
     * 
     * @param jsonObj 
     * 
     * Keys: 
     * - y (integer) mask value for onlyOnDays (optional)
     * - a (array) Array of YYYY-MM-DD value strings to allow (optional)
     * - x (array) Array of YYYY-MM-DD values to exclude (optional)
     */
    void fromJson(JSONValue jsonObj);

    LocalTimeDayOfWeek onlyOnDays;             //!< Allow on that day of week if mask bit is set
    std::vector<LocalTimeYMD> onlyOnDates;     //!< Dates to allow
    std::vector<LocalTimeYMD> exceptDates;     //!< Dates to exclude
};

/**
 * @brief HMS values, but only on specific days of week or dates (with optional exceptions)
 */
class LocalTimeHMSRestricted : public LocalTimeHMS, public LocalTimeRestrictedDate {
public:
    LocalTimeHMSRestricted() {
    }

    LocalTimeHMSRestricted(LocalTimeHMS hms) : LocalTimeHMS(hms), LocalTimeRestrictedDate(LocalTimeDayOfWeek::MASK_ALL) {
    }

    LocalTimeHMSRestricted(LocalTimeHMS hms, LocalTimeRestrictedDate restrictedDate) : LocalTimeHMS(hms), LocalTimeRestrictedDate(restrictedDate) {
    }

    /**
     * @brief Fills in this object from JSON data
     * 
     * @param jsonObj 
     * 
     * Keys: 
     * - t (string) Time string in HH:MM:SS format (can omit MM and SS parts, see LocalTimeHMS)
     * - y (integer) mask value for onlyOnDays (optional, from LocalTimeRestrictedDate)
     * - a (array) Array of YYYY-MM-DD value strings to allow (optional, from LocalTimeRestrictedDate)
     * - x (array) Array of YYYY-MM-DD values to exclude (optional, from LocalTimeRestrictedDate)
     */
    void fromJson(JSONValue jsonObj);

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
    /**
     * @brief Default contructor
     */
    LocalTimeChange();

    /**
     * @brief Destructor
     */
    virtual ~LocalTimeChange();

    /**
     * @brief Constructs a time change object with a string format (calls parse())
     * 
     * @param str the time change string to parse
     * 
     * The time change string is part of the POSIX timezone specification and looks something
     * like "M3.2.0/2:00:00". 
     */
    LocalTimeChange(const char *str);

    /**
     * @brief Clears all values
     */
    void clear();

    /**
     * @brief Parses a time change string
     * 
     * @param str the time change string to parse
     * 
     * The time change string is part of the POSIX timezone specification and looks something
     * like "M3.2.0/2:00:00". 
     * 
     * - M3 indicates that DST starts on the 3rd month (March)
     * - 2 is the week number (second week)
     * - 0 is the day of week (0 = Sunday)
     * - 2:00:00 at 2 AM local time, the transition occurs
     * 
     * Setting the week to 5 essentially means the last week of the month. If the month does
     * not have a fifth week for that day of the week, then the fourth is used instead.
     */
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
    LocalTimeHMS hms;       //!< Local time when timezone change occurs
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
 * 
 * For more information, see:
 * https://developer.ibm.com/technologies/systems/articles/au-aix-posix/
 */
class LocalTimePosixTimezone {
public:
    /**
     * @brief Default constructor (no timezone set)
     */
    LocalTimePosixTimezone();

    /**
     * @brief Destructor
     */
    virtual ~LocalTimePosixTimezone();

    /**
     * @brief Constructs the object with a specified timezone configuration
     * 
     * Calls parse() internally.
     */
    LocalTimePosixTimezone(const char *str);

    /**
     * @brief Clears the timezone setting in this object
     */
    void clear();

    /**
     * @brief Parses the timezone configuration string
     * 
     * @param str The string, for example: "EST5EDT,M3.2.0/2:00:00,M11.1.0/2:00:00"
     * 
     * If the string is not valid this function returns false and the valid flag will
     * be clear. You can call isValid() to check the validity at any time (such as
     * if you are using the constructor with a string that does not return a boolean).
     */
    bool parse(const char *str);

    /**
     * @brief Returns true if this timezone configuration has daylight saving
     */
    bool hasDST() const { return dstStart.valid; };

    /**
     * @brief Returns true if this timezone configuration has been set and appears valid
     */
    bool isValid() const { return valid; };

    /**
     * @brief Returns true if this timezone configuration is UTC
     */
    bool isZ() const { return !valid || (!hasDST() && standardHMS.toSeconds() == 0); };

    String dstName; //!< Daylight saving timezone name (empty string if no DST)
    LocalTimeHMS dstHMS; //!< Daylight saving time shift (relative to UTC)
    String standardName; //!< Standard time timezone name
    LocalTimeHMS standardHMS; //!< Standard time shift (relative to UTC). Note that this is positive in the United States, which is kind of backwards.
    LocalTimeChange dstStart; //!< Rule for when DST starts
    LocalTimeChange standardStart; //!< Rule for when standard time starts. 
    bool valid = false; //!< true if the configuration looks valid
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
    /**
     * @brief Returns the hour (0 - 23)
     */
    int hour() const { return tm_hour; };

    /**
     * @brief Returns the hour (1 - 12) used in AM/PM mode
     */
    int hourFormat12() const;

    /**
     * @brief Returns true if the time is in the AM (before noon)
     */
    uint8_t isAM() const { return tm_hour < 12; };

    /**
     * @brief Returns true if the time is in the PM (>= 12:00:00 in 24-hour clock).
     */
    uint8_t isPM() const { return !isAM(); };

    /**
     * @brief Returns the minute 0 - 59
     */
    int minute() const { return tm_min; };

    /**
     * @brief Returns the second 0 - 59
     */
    int second() const { return tm_sec; };

    /**
     * @brief Returns the day of the month 1 - 31 (or less in some months)
     */
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

    
    LocalTimeYMD ymd() const;

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


class LocalTimeConvert; // Forward declaration

/**
 * @brief Class to hold a time range in local time in HH:MM:SS format
 */
class LocalTimeRange : public LocalTimeRestrictedDate {
public: 
    /**
     * @brief Construct a new Time Range object with the range of the entire day (inclusive) 
     * 
     * This is start = 00:00:00, end = 23:59:59. The system clock does not have a concept of leap seconds.
     */
    LocalTimeRange() : hmsStart(LocalTimeHMS("00:00:00")), hmsEnd(LocalTimeHMS("23:59:59")), LocalTimeRestrictedDate(LocalTimeDayOfWeek::MASK_ALL) {
    }

    /**
     * @brief Construct a new Time Range object with the specifies start and end times.
     * 
     * @param hmsStart Start time in local time 00:00:00 <= hmsStart <= 23:59:59
     * @param hmsEnd  End time in local time 00:00:00 <= hmsStart <= 23:59:59

        * Note that 24:00:00 is not a valid time. You should generally use inclusive times such that
        * 23:59:59 is the end of the day.
        * 
        */
    LocalTimeRange(LocalTimeHMS hmsStart, LocalTimeHMS hmsEnd = LocalTimeHMS("23:59:59")) : hmsStart(hmsStart), hmsEnd(hmsEnd), LocalTimeRestrictedDate(LocalTimeDayOfWeek::MASK_ALL) {
    }

    LocalTimeRange(LocalTimeHMS hmsStart, LocalTimeHMS hmsEnd, LocalTimeRestrictedDate dateRestriction) : hmsStart(hmsStart), hmsEnd(hmsEnd), LocalTimeRestrictedDate(dateRestriction) {
    }


    void clear() {
        hmsStart = LocalTimeHMS("00:00:00");
        hmsEnd = LocalTimeHMS("23:59:59");
        LocalTimeRestrictedDate::clear();
    }

    /**
     * @brief Get the number of seconds between start and end based on a LocalTimeConvert object
     * 
     * The reason for the conv object is that it contains the time to calculate at, as well as 
     * the daylight saving time settings. This methods takes into account the actual number of
     * seconds including when a time change is crossed.
     * 
     * @param conv The time and timezone settings to calculate the time span at
     * @return time_t Time difference in seconds
     * 
     * In the weird case that start > end, it can return a negative value, as time_t is a signed
     * long (or long long) value.
     * 
     * This does not take into account date restrictions!
     * 
     * TODO: I think I can remove this
     */
    time_t getTimeSpan(const LocalTimeConvert &conv) const;

    /**
     * @brief Compares a time (LocalTimeHHS, local time) to this time range
     * 
     * @param hms 
     * @return int -1 if hms is before hmsStart, 0 if in range, +1 if hms is after hmsEnd
     */
    int compareTo(LocalTimeHMS hms) const {
        if (hms < hmsStart) {
            return -1;
        }
        else
        if (hms > hmsEnd) {
            return +1;
        }
        else {
            return 0;
        }
    }

    bool isValidDate(LocalTimeYMD ymd) const {
        return LocalTimeRestrictedDate::isValid(ymd);
    }

    bool inRangeDate(LocalTimeValue localTimeValue) const {    
        return LocalTimeRestrictedDate::isValid(localTimeValue);
    }

    bool inRange(LocalTimeValue localTimeValue) const {
        if (isValidDate(localTimeValue)) {
            LocalTimeHMS hms = localTimeValue.hms();
            return (hmsStart <= hms) && (hms <= hmsEnd);
        }
        else {
            return false;
        }
    }

    void fromTime(LocalTimeHMSRestricted hms) {
        *(LocalTimeRestrictedDate *)this = hms;
        hmsStart = hms;
    }

    /**
     * @brief Fills in the time range from a JSON object
     * 
     * @param jsonObj 
     * 
     * Keys:
     * - s (string) The start time (HH:MM:SS format, can omit MM or SS)
     * - e (string) The end time (HH:MM:SS format, can omit MM or SS)
     */
    void fromJson(JSONValue jsonObj);

    LocalTimeHMS hmsStart; //!< Starting time, inclusive
    LocalTimeHMS hmsEnd; //!< Ending time, inclusive
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
     * @brief Schedule option for "every n minutes"
     */
    class ScheduleItem {
    public: 
        /**
         * @brief Type of schedule item this is
         */
        enum class ScheduleItemType : int {
            NONE = 0,               //!< No multiple defined
            MINUTE_OF_HOUR,         //!< Minute of the hour (1)
            HOUR_OF_DAY,            //!< Hour of day (2)
            DAY_OF_WEEK_OF_MONTH,   //!< The nth day of week of the month (3)
            DAY_OF_MONTH,           //!< Day of the month (4)
            TIME                    //!< Specific time (5)    
        };

        /**
         * @brief Default constructor. Set increment and optionally timeRange to use.
         */
        ScheduleItem() {
        }

        /**
         * @brief Returns true ScheduleItemType is not NONE
         * 
         * @return true 
         * @return false 
         * 
         * This is used to check if an object was constructed by the default constructor and never set.
         */
        bool isValid() const { return (scheduleItemType != ScheduleItemType::NONE); };

        /**
         * @brief Get number of seconds in the time range at a given time
         * 
         * @param conv The timezone and date information for time span calculation
         * @return time_t 
         * 
         * The conv object is necessary because getTimeSpan takes into account daylight saving transitions.
         * When springing forward to daylight saving, from 01:15:00 to 03:15:00 is only one hour because of the 
         * DST transition.
         */
        time_t getTimeSpan(const LocalTimeConvert &conv) const {
            return timeRange.getTimeSpan(conv);
        }

        /**
         * @brief Update the conv object to point at the next schedule item
         * 
         * @param conv LocalTimeConvert object, may be modified
         * @return true if there is an item available or false if not. if false, conv will be unchanged.
         * 
         * This method finds the next scheduled time of this item, if it's in the near future.
         * The LocalTime::instance().getScheduleLookaheadDays() setting determines how far in the future
         * to check; the default is 3 days. The way schedules work each day needs to be checked to make
         * sure all of the constraints are met, so long look-aheads are computationally intensive. This
         * is not normally an issue, because the idea is that you'll wake from sleep or check the
         * schedule at least every few days, at which point the new schedule may be available.
         */
        bool getNextScheduledTime(LocalTimeConvert &conv) const;

        /**
         * @brief Creates an object from JSON
         * 
         * @param jsonObj The schedule. This should be the object containing the values, not the array.
         * 
         * Keys:
         * - m (integer) ScheduleItemType (1 = minute of hour, 2 = hour of day, 3 = day of week, 4 = day of month)
         * - i (integer) increment or ordinal value
         * - d (integer) dayOfWeek value (optional, only used for DAY_OF_WEEK_OF_MONTH)
         * - f (integer) flag bits (optional)
         * - s (string) The start time (HH:MM:SS format, can omit MM or SS) [from LocalTimeRange via LocalTimeRange]
         * - e (string) The end time (HH:MM:SS format, can omit MM or SS) [from LocalTimeRange via LocalTimeRange]
         * - y (integer) mask value for onlyOnDays [from LocalTimeRestrictedDate via LocalTimeRange]
         * - a (array) Array of YYYY-MM-DD value strings to allow [from LocalTimeRestrictedDate via LocalTimeRange]
         * - x (array) Array of YYYY-MM-DD values to exclude [from LocalTimeRestrictedDate via LocalTimeRange]
         */
        void fromJson(JSONValue jsonObj);

    
        LocalTimeRange timeRange; //!< Range of local time, inclusive
        int increment = 0; //!< Increment value, or sometimes ordinal value
        int dayOfWeek = 0; //!< Used for DAY_OF_WEEK_OF_MONTH only
        int flags = 0; //!< Optional scheduling flags
        ScheduleItemType scheduleItemType = ScheduleItemType::NONE; //!< The type of schedule item
    };

    /**
     * @brief A complete time schedule
     * 
     * A time schedule consists of minute multiples ("every 15 minutes"), optionally within a time range (all day, 
     * or from 09:00:00 to 17:00:00 local time, for example.
     * 
     * It can also have hour multiples, optionally in a time range, at a defined minute ("every 4 hours at :15 
     * past the hour").
     * 
     * Schedules can be at a specifc day week, with an ordinal (first Monday, last Friday) at a specific time, 
     * optionally with exceptions.
     * 
     * Schedules can be a specific day of the month (the 1st, the 5th of the month, the last day of the month, the 
     * second to last day of month).
     * 
     * It can also have any number of specific times in the day ("at 08:17:30 local time, 18:15:20 local time")
     * every day, specific days of the week, on specific dates, or with date exceptions.
     */
    class Schedule {
    public:
        /**
         * @brief Construct a new, empty schedule
         */
        Schedule() {
        }


        /**
         * @brief Adds a minute multiple schedule in a time range
         * 
         * @param increment Number of minutes (must be 1 <= minutes <= 59). A value that is is divisible by is recommended.
         * @param timeRange When to apply this minute multiple and/or minute offset.
         * 
         * This schedule publishes every n minutes within the hour. This really is every hour, not rolling, so you
         * should use a value that 60 is divisible by (2, 3, 4, 5, 6, 10, 12, 15, 20, 30) otherwise there will be
         * an inconsistent period at the top of the hour.
         * 
         * If you specify a time range that does not start at 00:00:00 you can customize which minute the schedule
         * starts at. For example: `15, LocalTimeRange(LocalTimeHMS("00:05:00"), LocalTimeHMS("23:59:59")` 
         * will schedule every 15 minutes, but starting at 5 minutes past the hour, so 05:00, 20:00, 35:00, 50:00.
         * 
         * The largest value for hmsEnd of the time range is 23:59:59.
         * 
         * @return Schedule& 
         */        
        Schedule &withMinuteMultiple(int increment, LocalTimeRange timeRange = LocalTimeRange());

        /**
         * @brief Add a scheduled item at a time in local time during the day. 
         * 
         * @param hms The time in local time 00:00:00 to 23:59:59.
         * @return Schedule& 
         * 
         * You can call this multiple times, and also combine it with minute multiple schedules.
         */
        Schedule &withTime(LocalTimeHMSRestricted hms);

        /**
         * @brief Add multiple scheduled items at a time in local time during the day. 
         * 
         * @param timesParam an auto-initialized list of LocalTimeHMS objects
         * @return Schedule& 
         * 
         * You can call this multiple times, and also combine it with minute multiple schedules.
         * 
         * schedule.withTimes({LocalTimeHMS("06:00"), LocalTimeHMS("18:30")});
         */
        Schedule &withTimes(std::initializer_list<LocalTimeHMSRestricted> timesParam);

        /**
         * @brief Adds multiple times periodically in a time range with an hour increment
         * 
         * @param hourMultiple Hours between items must be >= 1. For example: 2 = every other hour.
         * @param timeRange Time range to add items to. This is optional; if not specified then the entire day. 
         * Also is used to specify a minute offset.
         * 
         * @return Schedule& 
         * 
         * Hours are per day, local time. For whole-day schedules, you will typically use a value that
         * 24 is evenly divisible by (2, 3, 4, 6, 8, 12), because otherwise the time periods will brief
         * unequal at the top of the hour.
         * 
         * Also note that times are local, and take into account daylight saving. Thus during a time switch,
         * the interval may end up being a different number of hours than specified. For example, if the
         * times would have been 00:00 and 04:00, a hourMultiple of 4, and you do this over a spring forward, 
         * the actual number hours between 00:00 and 04:00 is 5 (at least in the US where DST starts at 2:00).
         */
        Schedule &withHourMultiple(int hourMultiple, LocalTimeRange timeRange = LocalTimeRange());

        /**
         * @brief Returns true if the schedule does not have any items in it
         * 
         * @return true 
         * @return false 
         */
        bool isEmpty() const { 
            return scheduleItems.empty();
        }

        /**
         * @brief Clear the existing settings
         */
        void clear() {
            scheduleItems.clear();   
        }

        /**
         * @brief Set the schedule from a JSON string containing an array of objects.
         * 
         * @param jsonStr 
         * 
         * See the overload that takes a JSONValue if the JSON string has already been parsed.
         */
        void fromJson(const char *jsonStr);

        /**
         * @brief Set the schedule of this object from a JSONValue, typically the outer object
         * 
         * @param jsonArray A JSONValue containing an array of objects
         * 
         * Array of ScheduleItem objects:
         *  - mh (integer): Minute of hour (takes place of setting m and i separately)
         *  - hd (integer): Hour of day (takes place of setting m and i separately)
         *  - dw (integer): Day of week (takes place of setting m and i separately)
         *  - dm (integer): Day of month (takes place of setting m and i separately)
         *  - tm (string) Time string in HH:MM:SS format (can omit MM and SS parts, see LocalTimeHMS) for TIME items
         *  - m (integer) type of multiple (optional if mm, )
         *  - i (integer) increment
         *  - f (integer) flag bits (optional)
         *  - s (string) The start time (HH:MM:SS format, can omit MM or SS) [from LocalTimeRange via LocalTimeRange]
         *  - e (string) The end time (HH:MM:SS format, can omit MM or SS) [from LocalTimeRange via LocalTimeRange]
         *  - y (integer) mask value for onlyOnDays [from LocalTimeRestrictedDate via LocalTimeRange]
         *  - a (array) Array of YYYY-MM-DD value strings to allow [from LocalTimeRestrictedDate via LocalTimeRange]
         *  - x (array) Array of YYYY-MM-DD values to exclude [from LocalTimeRestrictedDate via LocalTimeRange]
         */
        void fromJson(JSONValue jsonArray);

        std::vector<ScheduleItem> scheduleItems; //!< Schedule items
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
     * @brief Adds a number of seconds to the current object
     * 
     * @param seconds 
     */
    void addSeconds(int seconds);

    /**
     * @brief Moves the current time the next specified multiple of minutes
     * 
     * @param increment Typically something like 5, 15, 20, 30 that 60 is evenly divisible by
     * 
     * @param startingModulo (optional). If present, must be 0 < startingModulo < increment
     * 
     * Moves the time forward to the next multiple of that number of minutes. For example, if the 
     * clock is at :10 past the hour and the multiple is 15, then time will be updated to :15. If
     * the time is equal to an even multple, the next multiple is selected.
     * 
     * Upon completion, all fields are updated appropriately. For example:
     * - time specifies the time_t of the new time at UTC
     * - localTimeValue contains the broken-out values for the local time
     * - isDST() return true if the new time is in daylight saving time
     */
    void nextMinuteMultiple(int increment, int startingModulo = 0);

    /**
     * @brief Moves the current time the next specified local time. This could be today or tomorrow.
     * 
     * @param hms Moves to the next occurrence of that time of day (local time)
     * 
     * Upon completion, all fields are updated appropriately. For example:
     * - time specifies the time_t of the new time at UTC
     * - localTimeValue contains the broken-out values for the local time
     * - isDST() return true if the new time is in daylight saving time
     */
    void nextTime(LocalTimeHMS hms);

    /**
     * @brief Moves the current time the closest local time om hmsList. This could be today or tomorrow.
     * 
     * @param hmsList An initialize list of LocalTimeHMS surrounded by {}
     * 
     * For example, this sets the time to the nearest noon or midnight local time greater than the time
     * set in this object:
     * 
     * conv.nextTimeList({LocalTimeHMS("00:00"), LocalTimeHMS("12:00")});
     */
    void nextTimeList(std::initializer_list<LocalTimeHMS> hmsList);


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
     * @brief Moves the current to the next day, or right after the next time change, whichever comes first 
     * 
     * @param hms If specified, moves to that time of day (local time). If omitted, leaves the current time and only changes the date.
     * 
     * Upon completion, all fields are updated appropriately. For example:
     * - time specifies the time_t of the new time at UTC
     * - localTimeValue contains the broken-out values for the local time
     * - isDST() return true if the new time is in daylight saving time
     * 
     * This method is used when you want to synchronize an external device clock daily to keep it synchronized,
     * or right after a time change.
     * 
     * Do not pick the local time of the time change as the hms time! For example, in the United State do *not* select
     * 02:00:00. The reason is that on spring forward, that time doesn't actually exist, because as soon as the clock
     * hits 02:00:00 it jumps forward to 03:00:00 local time. Picking 03:00:00 or really any other time that's not between 
     * 02:00:00 and 02:59:59 is fine. During fall back, even though you've picked the time sync time to be 03:00 local
     * time it will sync at the time of the actual time change correctly, which is why this function is different than
     * nextDay().
     */
    void nextDayOrTimeChange(LocalTimeHMS hms = LocalTimeIgnoreHMS());

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
     * @param dayOfMonth The day of the month (1 = first day of the month). See also special cases below.
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
     * 
     * dayOfMonth is normally a a day number (1 = first day of the month). There are also special cases:
     * 0 = the last day of the month, -1 the second to last day of month, ... The number of days in the month are
     * based on the date in local time.
     */
    bool nextDayOfMonth(int dayOfMonth, LocalTimeHMS hms = LocalTimeIgnoreHMS());

    /**
     * @brief Moves the date and time (local time) forward to the specified day of month and local time
     * 
     * @param dayOfMonth The day of the month (1 = first day of the month). See also special cases below.
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
     * 
     * dayOfMonth is normally a a day number (1 = first day of the month). There are also special cases:
     * 0 = the last day of the month, -1 the second to last day of month, ... The number of days in the month are
     * based on the date in local time.
     */
    bool nextDayOfNextMonth(int dayOfMonth, LocalTimeHMS hms = LocalTimeIgnoreHMS());


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
     * @brief Sets the time to the nearest scheduled time in the future base on the schedule
     * 
     * @param schedule 
     */
    bool nextSchedule(const Schedule &schedule);

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
     * @brief Get the value of this object as a LocalTimeHMS (hour minute second)
     * 
     * @return LocalTimeHMS 
     */
    LocalTimeHMS getLocalTimeHMS() const { return LocalTimeHMS(localTimeValue); };

    /**
     * @brief Get the value of this object as a LocalTimeYMD (year month day0)
     * 
     * @return LocalTimeYMD 
     */
    LocalTimeYMD getLocalTimeYMD() const { return LocalTimeYMD(localTimeValue); };

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
     * @brief Returns the last day of the month, local time (based on localTimeValue)
     */
    int lastDayOfMonth() const;

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


/**
 * @brief Global time settings
 */
class LocalTime {
public:
    /**
     * @brief Get the global singleton instance of this class
     */
    static LocalTime &instance();

    /**
     * @brief Sets the default global timezone configuration
     */
    LocalTime &withConfig(LocalTimePosixTimezone config) { this->config = config; return *this; };

    /**
     * @brief Gets the default global timezone configuration
     */
    const LocalTimePosixTimezone &getConfig() const { return config; };

    /**
     * @brief Sets the maximum number of days to look ahead in the schedule for a match (default: 3)
     * 
     * @param value 
     * @return LocalTime& 
     */
    LocalTime &withScheduleLookaheadDays(int value) { scheduleLookaheadDays = value; return *this; };

    /**
     * @brief Gets the maximum number of days to look ahead in the schedule for a match
     * 
     * @return int 
     */
    int getScheduleLookaheadDays() const { return scheduleLookaheadDays; };

    
    /**
     * @brief Converts a Unix time (seconds past Jan 1 1970) UTC value to a struct tm
     * 
     * @param time Unix time (seconds past Jan 1 1970) UTC
     * 
     * @param pTimeInfo Pointer to a struct tm that is filled in with the time broken out
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
     * @param pTimeInfo Pointer to a struct tm
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
     * 
     * @param pTimeInfo Pointer to a struct tm to convert
     */
    static String getTmString(struct tm *pTimeInfo);

    /**
     * @brief Converts a string in ISO-8601 format (ignoring the timezone)
     * 
     * @param str String to convert
     * 
     * @param pTimeInfo Pointer to a struct tm to fill in or NULL if you don't need that
     * 
     * @returns a time_t (Unix time, seconds past January 1, 1970, UTC)
     * 
     * The string must be of the form "YYYY-MM-DDTHH:MM:SS". Any additional characters
     * are ignored, so it's OK if the time includes the timezone, though it will be ignored.
     * Same for milliseconds. The T between the day and hour can be any single 
     * non-numeric character, such as a space, instead of a T.
     * 
     * See also timeToString to convert in the other direction.
     */
    static time_t stringToTime(const char *str, struct tm *pTimeInfo = NULL);


    /**
     * @brief Converts a time to a string in a modified ISO-8601 format with no timezone
     * 
     * @param time Unix time (seconds past Jan 1 1970) UTC
     * 
     * @param separator the separator between the day of month and hour, typically
     * T or a space.
     * 
     * @returns a time_t (Unix time, seconds past January 1, 1970, UTC)
     * 
     * The string will be of the form "YYYY-MM-DDTHH:MM:SS". 
     * 
     * See also timeToString to convert in the other direction.
     */
    static String timeToString(time_t time, char separator = ' ');

    /**
     * @brief Returns the last day of the month in a given month and year
     * 
     * @param year The year (note, actual year like 2021, not the value of tm_year).
     * 
     * @param month The month (1 - 12)
     * 
     * For example, If the month in the current object is January, returns 31.
     * The year is required to handle leap years when the month is February.
     */
    static int lastDayOfMonth(int year, int month);

    /**
     * @brief Return the nth instance of a day of week in a month and year
     * 
     * @param year The 4-digit year (2020, for example)
     * @param month Month 1 - 12 inclusive, 1 = January, 12 = December
     * @param dayOfWeek 0 = Sunday, 1 = Monday, 2 = Tuesday, ..., 6 = Saturday
     * @param ordinal 1 = first instance of that day of week in the month, 2 = second, ...
     * @return int The day of the month, or 0 if that ordinal does not exist in the month
     */
    static int dayOfWeekOfMonth(int year, int month, int dayOfWeek, int ordinal);

protected:
    /**
     * @brief This class is a singleton and should not be manually allocated
     */
    LocalTime() {};

    /**
     * @brief This class is a singleton and should not be manually destructed
     */
    virtual ~LocalTime() {};

    /**
     * @brief This class is not copyable
     */
    LocalTime(const LocalTime&) = delete;

    /**
     * @brief This class is not copyable
     */
    LocalTime& operator=(const LocalTime&) = delete;


    /**
     * @brief Global default timezone
     * 
     * The LocalTimeConverter class will use this if a config is not set for that specific 
     * converter.
     */
    LocalTimePosixTimezone config;

    /**
     * @brief Number of days to look forward to see if there are scheduled events. Default: 3
     */
    int scheduleLookaheadDays = 3;

    /**
     * @brief Singleton instance of this class
     */
    static LocalTime *_instance;
};


#endif /* __LOCALTIMERK_H */
