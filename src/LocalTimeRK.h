#ifndef __LOCALTIMERK_H
#define __LOCALTIMERK_H

#include "Particle.h"

#include <time.h>

class LocalTimeChange {
public:
    LocalTimeChange();
    virtual ~LocalTimeChange();

    LocalTimeChange(const char *str);

    void clear();

    // M3.2.0/2:00:00
    void parse(const char *str);

    String toString() const;


    int8_t month = 0;       //!< 1-12, 1=January
    int8_t week = 0;        //!< 1-5, 1=first
    int8_t dayOfWeek = 0;   //!< 0-6, 0=Sunday, 1=Monday, ...
    int8_t hour = 0;        //!< 0-23 hour to make change
    int8_t minute = 0;      //!< 0-59 minute
    int8_t second = 0;      //!< 0-59 second
    int8_t valid = 0;       //!< true = valid
};

class LocalTimePosixTimezone {
public:
    LocalTimePosixTimezone();
    virtual ~LocalTimePosixTimezone();

    LocalTimePosixTimezone(const char *str);

    void parse(const char *str);

protected:
     

};


class LocalTime {
public:
    LocalTime();
    virtual ~LocalTime();

    // POSIX timezone strings
    // https://developer.ibm.com/technologies/systems/articles/au-aix-posix/
    // https://support.cyberdata.net/index.php?/Knowledgebase/Article/View/438/10/posix-timezone-strings
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
     */
    static void timeToTm(time_t time, struct tm *pTimeInfo);

    /**
     * @brief Converts a struct tm to a Unix time (seconds past Jan 1 1970) UTC
     * 
     * @param pTimeTime Pointer to a struct tm
     * 
     * Note: tm_wday, tm_yday, and tm_isdst are ignored.
     */
    static time_t tmToTime(struct tm *pTimeInfo);

};


#endif /* __LOCALTIMERK_H */
