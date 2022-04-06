
# class LocalTime 

Global time settings.

## Members

---

### LocalTime & LocalTime::withConfig(LocalTimePosixTimezone config) 

Sets the default global timezone configuration.

```
LocalTime & withConfig(LocalTimePosixTimezone config)
```

---

### const LocalTimePosixTimezone & LocalTime::getConfig() const 

Gets the default global timezone configuration.

```
const LocalTimePosixTimezone & getConfig() const
```

# class LocalTimeChange 

Handles the time change part of the Posix timezone string like "M3.2.0/2:00:00".

Other formats with shortened time of day are also allowed like "M3.2.0/2" or even "M3.2.0" (midnight) are also allowed. Since the hour is local time, it can also be negative "M3.2.0/-1".

## Members

---

### int8_t month 

1-12, 1=January

```
int8_t month
```

---

### int8_t week 

1-5, 1=first

```
int8_t week
```

---

### int8_t dayOfWeek 

0-6, 0=Sunday, 1=Monday, ...

```
int8_t dayOfWeek
```

---

### int8_t valid 

true = valid

```
int8_t valid
```

---

### LocalTimeHMS hms 

Local time when timezone change occurs.

```
LocalTimeHMS hms
```

---

###  LocalTimeChange::LocalTimeChange() 

Default contructor.

```
 LocalTimeChange()
```

---

###  LocalTimeChange::~LocalTimeChange() 

Destructor.

```
virtual  ~LocalTimeChange()
```

---

###  LocalTimeChange::LocalTimeChange(const char * str) 

Constructs a time change object with a string format (calls parse())

```
 LocalTimeChange(const char * str)
```

#### Parameters
* `str` the time change string to parse

The time change string is part of the POSIX timezone specification and looks something like "M3.2.0/2:00:00".

---

### void LocalTimeChange::clear() 

Clears all values.

```
void clear()
```

---

### void LocalTimeChange::parse(const char * str) 

Parses a time change string.

```
void parse(const char * str)
```

#### Parameters
* `str` the time change string to parse

The time change string is part of the POSIX timezone specification and looks something like "M3.2.0/2:00:00".

* M3 indicates that DST starts on the 3rd month (March)

* 2 is the week number (second week)

* 0 is the day of week (0 = Sunday)

* 2:00:00 at 2 AM local time, the transition occurs

Setting the week to 5 essentially means the last week of the month. If the month does not have a fifth week for that day of the week, then the fourth is used instead.

---

### String LocalTimeChange::toString() const 

Turns the parsed data into a normalized string like "M3.2.0/2:00:00".

```
String toString() const
```

---

### time_t LocalTimeChange::calculate(struct tm * pTimeInfo, LocalTimeHMS tzAdjust) const 

Calculate the time change in a given year.

```
time_t calculate(struct tm * pTimeInfo, LocalTimeHMS tzAdjust) const
```

On input, pTimeInfo must have a valid tm_year (121 = 2021) set.

On output, all struct tm values are set appropriately with UTC values of when the time change occurs.

# class LocalTimeConvert 

Perform time conversions. This is the main class you will need.

## Members

---

### Position position 

Where time is relative to DST.

```
Position position
```

---

### LocalTimePosixTimezone config 

Timezone configuration for this time conversion.

```
LocalTimePosixTimezone config
```

If you don't specify this using withConfig then the global setting is retrieved from the LocalTime singleton instance.

---

### time_t time 

The time that is being converted. This is always Unix time at UTC.

```
time_t time
```

Seconds after January 1, 1970, UTC. Using methods like nextDay() increment this value.

---

### LocalTimeValue localTimeValue 

The local time that corresponds to time.

```
LocalTimeValue localTimeValue
```

The convert() method sets this, as do methods like nextDay(). The local time will depend on the timezone set in config, as well as the date which will determine whether it's daylight saving or not.

---

### time_t dstStart 

The time that daylight saving starts, Unix time, UTC.

```
time_t dstStart
```

The config specifies the rule (2nd Sunday in March, for example), and the local time that the change occurs. This is the UTC value that corresponds to that rule in the year specified by time.

Note in the southern hemisphere, dstStart is after standardStart.

---

### struct tm dstStartTimeInfo 

The struct tm that corresponds to dstStart (UTC)

```
struct tm dstStartTimeInfo
```

---

### time_t standardStart 

The time that standard time starts, Unix time, UTC.

```
time_t standardStart
```

The config specifies the rule (1st Sunday in November, for example), and the local time that the change occurs. This is the UTC value that corresponds to that rule in the year specified by time.

Note in the southern hemisphere, dstStart is after standardStart.

---

### struct tm standardStartTimeInfo 

The struct tm that corresponds to standardStart (UTC)

```
struct tm standardStartTimeInfo
```

---

### LocalTimeConvert & LocalTimeConvert::withConfig(LocalTimePosixTimezone config) 

Sets the timezone configuration to use for time conversion.

```
LocalTimeConvert & withConfig(LocalTimePosixTimezone config)
```

If you do not use withConfig() the global default set in the LocalTime class is used. If neither are set, the local time is UTC (with no DST).

---

### LocalTimeConvert & LocalTimeConvert::withTime(time_t time) 

Sets the UTC time to begin conversion from.

```
LocalTimeConvert & withTime(time_t time)
```

#### Parameters
* `time` The time (UTC) to set. This is the Unix timestamp (seconds since January 1, 1970) UTC such as returned by Time.now().

This does not start the conversion; you must also call the convert() method after setting all of the settings you want to use.

For the current time, you can instead use withCurrentTime();

---

### LocalTimeConvert & LocalTimeConvert::withCurrentTime() 

Use the current time as the time to start with.

```
LocalTimeConvert & withCurrentTime()
```

This does not start the conversion; you must also call the convert() method after setting all of the settings you want to use.

---

### void LocalTimeConvert::convert() 

Do the time conversion.

```
void convert()
```

You must call this after changing the configuration or the time using withTime() or withCurrentTime()

---

### bool LocalTimeConvert::isDST() const 

Returns true if the current time is in daylight saving time.

```
bool isDST() const
```

---

### bool LocalTimeConvert::isStandardTime() const 

Returns true of the current time in in standard time.

```
bool isStandardTime() const
```

---

### void LocalTimeConvert::addSeconds(int seconds) 

Adds a number of seconds to the current object.

```
void addSeconds(int seconds)
```

#### Parameters
* `seconds`

---

### void LocalTimeConvert::nextMinuteMultiple(int minuteMultiple, int startingModulo) 

Moves the current time the next specified multiple of minutes.

```
void nextMinuteMultiple(int minuteMultiple, int startingModulo)
```

#### Parameters
* `minuteMultiple` Typically something like 5, 15, 20, 30 that 60 is evenly divisible by

* `startingModulo` (optional). If present, must be 0 < startingModulo < minuteMultiple

Moves the time forward to the next multiple of that number of minutes. For example, if the clock is at :10 past the hour and the multiple is 15, then time will be updated to :15. If the time is equal to an even multple, the next multiple is selected.

Upon completion, all fields are updated appropriately. For example:

* time specifies the time_t of the new time at UTC

* localTimeValue contains the broken-out values for the local time

* isDST() return true if the new time is in daylight saving time

---

### void LocalTimeConvert::nextTime(LocalTimeHMS hms) 

Moves the current time the next specified local time. This could be today or tomorrow.

```
void nextTime(LocalTimeHMS hms)
```

#### Parameters
* `hms` Moves to the next occurrence of that time of day (local time)

Upon completion, all fields are updated appropriately. For example:

* time specifies the time_t of the new time at UTC

* localTimeValue contains the broken-out values for the local time

* isDST() return true if the new time is in daylight saving time

---

### void LocalTimeConvert::nextTimeList(std::initializer_list< LocalTimeHMS > hmsList) 

Moves the current time the closest local time om hmsList. This could be today or tomorrow.

```
void nextTimeList(std::initializer_list< LocalTimeHMS > hmsList)
```

#### Parameters
* `hmsList` An initialize list of LocalTimeHMS surrounded by {}

For example, this sets the time to the nearest noon or midnight local time greater than the time set in this object:

conv.nextTimeList({LocalTimeHMS("00:00"), LocalTimeHMS("12:00")});

---

### void LocalTimeConvert::nextDay(LocalTimeHMS hms) 

Moves the current time to the next day.

```
void nextDay(LocalTimeHMS hms)
```

#### Parameters
* `hms` If specified, moves to that time of day (local time). If omitted, leaves the current time and only changes the date.

Upon completion, all fields are updated appropriately. For example:

* time specifies the time_t of the new time at UTC

* localTimeValue contains the broken-out values for the local time

* isDST() return true if the new time is in daylight saving time

---

### void LocalTimeConvert::nextDayOrTimeChange(LocalTimeHMS hms) 

Moves the current to the next day, or right after the next time change, whichever comes first.

```
void nextDayOrTimeChange(LocalTimeHMS hms)
```

#### Parameters
* `hms` If specified, moves to that time of day (local time). If omitted, leaves the current time and only changes the date.

Upon completion, all fields are updated appropriately. For example:

* time specifies the time_t of the new time at UTC

* localTimeValue contains the broken-out values for the local time

* isDST() return true if the new time is in daylight saving time

This method is used when you want to synchronize an external device clock daily to keep it synchronized, or right after a time change.

Do not pick the local time of the time change as the hms time! For example, in the United State do *not* select 02:00:00. The reason is that on spring forward, that time doesn't actually exist, because as soon as the clock hits 02:00:00 it jumps forward to 03:00:00 local time. Picking 03:00:00 or really any other time that's not between 02:00:00 and 02:59:59 is fine. During fall back, even though you've picked the time sync time to be 03:00 local time it will sync at the time of the actual time change correctly, which is why this function is different than nextDay().

---

### bool LocalTimeConvert::nextDayOfWeek(int dayOfWeek, LocalTimeHMS hms) 

Moves the current time to the next of the specified day of week.

```
bool nextDayOfWeek(int dayOfWeek, LocalTimeHMS hms)
```

#### Parameters
* `dayOfWeek` The day of week (0 - 6, 0 = Sunday, 1 = Monday, ..., 6 = Saturday)

* `hms` If specified, moves to that time of day (local time). If omitted, leaves the current time and only changes the date.

Upon completion, all fields are updated appropriately. For example:

* time specifies the time_t of the new time at UTC

* localTimeValue contains the broken-out values for the local time

* isDST() return true if the new time is in daylight saving time

---

### void LocalTimeConvert::nextWeekday(LocalTimeHMS hms) 

Returns the next day that is a weekday (Monday - Friday)

```
void nextWeekday(LocalTimeHMS hms)
```

#### Parameters
* `hms` If specified, moves to that time of day (local time). If omitted, leaves the current time and only changes the date.

Upon completion, all fields are updated appropriately. For example:

* time specifies the time_t of the new time at UTC

* localTimeValue contains the broken-out values for the local time

* isDST() return true if the new time is in daylight saving time

---

### void LocalTimeConvert::nextWeekendDay(LocalTimeHMS hms) 

Returns the next day that is a weekend day (Saturday or Sunday)

```
void nextWeekendDay(LocalTimeHMS hms)
```

#### Parameters
* `hms` If specified, moves to that time of day (local time). If omitted, leaves the current time and only changes the date.

Upon completion, all fields are updated appropriately. For example:

* time specifies the time_t of the new time at UTC

* localTimeValue contains the broken-out values for the local time

* isDST() return true if the new time is in daylight saving time

---

### bool LocalTimeConvert::nextDayOfMonth(int dayOfMonth, LocalTimeHMS hms) 

Moves the date and time (local time) forward to the specified day of month and local time.

```
bool nextDayOfMonth(int dayOfMonth, LocalTimeHMS hms)
```

#### Parameters
* `dayOfMonth` The day of the month (1 = first day of the month). See also special cases below.

* `hms` If specified, moves to that time of day (local time). If omitted, leaves the current time and only changes the date.

This version will move to the closest forward time. It could be as close as 1 second later, but it will always advance at least once second. It could be as much as 1 month minus 1 second later.

Upon completion, all fields are updated appropriately. For example:

* time specifies the time_t of the new time at UTC

* localTimeValue contains the broken-out values for the local time

* isDST() return true if the new time is in daylight saving time

dayOfMonth is normally a a day number (1 = first day of the month). There are also special cases: 0 = the last day of the month, -1 the second to last day of month, ... The number of days in the month are based on the date in local time.

---

### bool LocalTimeConvert::nextDayOfNextMonth(int dayOfMonth, LocalTimeHMS hms) 

Moves the date and time (local time) forward to the specified day of month and local time.

```
bool nextDayOfNextMonth(int dayOfMonth, LocalTimeHMS hms)
```

#### Parameters
* `dayOfMonth` The day of the month (1 = first day of the month). See also special cases below.

* `hms` If specified, moves to that time of day (local time). If omitted, leaves the current time and only changes the date.

This version always picks the next month, even if the target day of month hasn't been reached in this month yet. This will always more forward at least a month, and may be as much as two months minus one day.

Upon completion, all fields are updated appropriately. For example:

* time specifies the time_t of the new time at UTC

* localTimeValue contains the broken-out values for the local time

* isDST() return true if the new time is in daylight saving time

dayOfMonth is normally a a day number (1 = first day of the month). There are also special cases: 0 = the last day of the month, -1 the second to last day of month, ... The number of days in the month are based on the date in local time.

---

### bool LocalTimeConvert::nextDayOfWeekOrdinal(int dayOfWeek, int ordinal, LocalTimeHMS hms) 

Moves the date and time (local time) forward to the specified day of month and local time.

```
bool nextDayOfWeekOrdinal(int dayOfWeek, int ordinal, LocalTimeHMS hms)
```

#### Parameters
* `dayOfWeek` The day of week (0 - 6, 0 = Sunday, 1 = Monday, ..., 6 = Saturday)

* `ordinal` 1 = first of that day of week in month, 2 = second, ...

* `hms` If specified, moves to that time of day (local time). If omitted, leaves the current time and only changes the date.

If you specify an ordinal that does not exist (for example, there may not be a 5th ordinal for certain days in certain months), returns false and leaves the date unchanged.

Upon successful completion, all fields are updated appropriately. For example:

* time specifies the time_t of the new time at UTC

* localTimeValue contains the broken-out values for the local time

* isDST() return true if the new time is in daylight saving time

---

### void LocalTimeConvert::nextLocalTime(LocalTimeHMS hms) 

Sets the time to the nearest hms in local time in the future.

```
void nextLocalTime(LocalTimeHMS hms)
```

#### Parameters
* `hms` The time of day to set in local time

Moves the time forward to the next instance of hms in local time. If hms has not occurred yet, it will select the one today, but it will always move the time forward.

There is a weird special case on the beginning of daylight saving (spring forward in the northern hemisphere). If you select a hms between 2:00 AM and 2:59:59 AM local time, this time does not exist on spring forward day because it's the hour that is skipped. In order to preserve the requirement that the time will always be advanced by this call, it will jump forward to the next day when the 2:00 hour occurs next. (The hour may be different in other locations, for example it's 1:00 AM in the UK.)

In the case of fall back, if you specify a hms in the repeated hour (1:00:00 to 1:59:59) the time returned will be the second time this time occurs, in standard time in the US.

Upon completion, all fields are updated appropriately. For example:

* time specifies the time_t of the new time at UTC

* localTimeValue contains the broken-out values for the local time

* isDST() return true if the new time is in daylight saving time

---

### bool LocalTimeConvert::nextSchedule(const Schedule & schedule) 

Sets the time to the nearest scheduled time in the future base on the schedule.

```
bool nextSchedule(const Schedule & schedule)
```

#### Parameters
* `schedule`

---

### void LocalTimeConvert::atLocalTime(LocalTimeHMS hms) 

Changes the time of day to the specified hms in local time on the same local day.

```
void atLocalTime(LocalTimeHMS hms)
```

#### Parameters
* `hms` A LocalTimeHMS object with hour, minute, and second

You can use the string constructor like this to set the time to 2:00 PM local time.

```cpp
converter.atLocalTime(LocalTimeHms("14:00:00"));
```

It's possible that this will set the time to a time earlier than the object's current time. To only set a time in the future, use nextLocalTime() instead.

Upon completion, all fields are updated appropriately. For example:

* time specifies the time_t of the new time at UTC

* localTimeValue contains the broken-out values for the local time

* isDST() return true if the new time is in daylight saving time

---

### bool LocalTimeConvert::inLocalTimeRange(TimeRange localTimeRange) 

Returns true if this object time is in the specified time range in local time.

```
bool inLocalTimeRange(TimeRange localTimeRange)
```

#### Parameters
* `localTimeRange` time in this object is: start <= time <= end (inclusive) 

#### Returns
true 

#### Returns
false

---

### bool LocalTimeConvert::beforeLocalTimeRange(TimeRange localTimeRange) 

Returns true if this object time before the specified time range in local time.

```
bool beforeLocalTimeRange(TimeRange localTimeRange)
```

#### Parameters
* `localTimeRange` time in this object is: time < start (exclusive) 

#### Returns
true 

#### Returns
false

---

### String LocalTimeConvert::timeStr() 

Works like Time.timeStr() to generate a readable string of the local time.

```
String timeStr()
```

Uses asctime formatting, which looks like "Fri Jan  1 18:45:56 2021". The strings are not localized; they're always in English.

---

### String LocalTimeConvert::format(const char * formatSpec) 

Works like Time.format()

```
String format(const char * formatSpec)
```

#### Parameters
* `formatSpec` the format specifies, which can be

* TIME_FORMAT_DEFAULT (example: "Thu Apr  1 12:00:00 2021")

* TIME_FORMAT_ISO8601_FULL (example: "2021-04-01T12:00:00-04:00")

* custom format based on strftime()

There are many options to strftime described here: [https://www.cplusplus.com/reference/ctime/strftime/?kw=strftime](https://www.cplusplus.com/reference/ctime/strftime/?kw=strftime)

Unlike Time.format(), you can use Z to output the timezone abbreviation, for example "EDT" for the Eastern United States, daylight saving instead of -04:00.

The z formatting matches that of Time.format(), which is wrong. The correct output should be "-400" but the output will be "-04:00" for compatibility.

---

### String LocalTimeConvert::zoneName() const 

Returns the abbreviated time zone name for the current time.

```
String zoneName() const
```

For example, for the United States east coast, EST or EDT depending on whether the current time is DST or not. See also isDST().

This string comes from the LocalTimePosixTimezone object.

---

### int LocalTimeConvert::lastDayOfMonth() const 

Returns the last day of the month, local time (based on localTimeValue)

```
int lastDayOfMonth() const
```

---

### enum Position 

Whether the specified time is DST or not. See also isDST().

```
enum Position
```

 Values                         | Descriptions                                
--------------------------------|---------------------------------------------
BEFORE_DST            | This time is before the start of DST (northern hemisphere)
IN_DST            | This time is in daylight saving time (northern hemisphere)
AFTER_DST            | This time is after the end of DST (northern hemisphere)
BEFORE_STANDARD            | This time is before the start of standard time (southern hemisphere)
IN_STANDARD            | This time is in standard saving time (southern hemisphere)
AFTER_STANDARD            | This time is after the end of standard time (southern hemisphere)
NO_DST            | This config does not use daylight saving.

# class LocalTimeHMS 

Container for holding an hour minute second time value.

## Members

---

### int8_t hour 

0-23 hour (could also be negative)

```
int8_t hour
```

---

### int8_t minute 

0-59 minute

```
int8_t minute
```

---

### int8_t second 

0-59 second

```
int8_t second
```

---

### int8_t ignore 

Special case.

```
int8_t ignore
```

---

###  LocalTimeHMS::LocalTimeHMS() 

Default constructor. Sets time to 00:00:00.

```
 LocalTimeHMS()
```

---

###  LocalTimeHMS::~LocalTimeHMS() 

Destructor.

```
virtual  ~LocalTimeHMS()
```

---

###  LocalTimeHMS::LocalTimeHMS(const char * str) 

Constructs the object from a time string.

```
 LocalTimeHMS(const char * str)
```

#### Parameters
* `str` The time string

The time string is normally of the form HH:MM:SS, such as "04:00:00" for 4:00 AM. The hour is in 24-hour format. Other formats are supported as well, including omitting the seconds (04:00), or including only the hour "04", or omitting the leadings zeros (4:0:0).

Additionally, the hour could be negative, used in UTC DST offsets. The minute and second are always positive (0-59). The hour could also be > 24 when used as a timezone offset.

---

### void LocalTimeHMS::clear() 

Sets the hour, minute, and second to 0.

```
void clear()
```

---

### void LocalTimeHMS::parse(const char * str) 

Parse a "H:MM:SS" string.

```
void parse(const char * str)
```

#### Parameters
* `str` Input string

Multiple formats are supported, and parts are optional:

* H:MM:SS (examples: "2:00:00" or "2:0:0")

* H:MM (examples: "2:00" or "2:0")

* H (examples: "2")

Hours are always 0 - 23 (24-hour clock). Can also be a negative hour -1 to -23.

---

### String LocalTimeHMS::toString() const 

Turns the parsed data into a normalized string of the form: "H:MM:SS" (24-hour clock)

```
String toString() const
```

---

### int LocalTimeHMS::toSeconds() const 

Convert hour minute second into a number of seconds (simple multiplication and addition)

```
int toSeconds() const
```

---

### void LocalTimeHMS::fromTimeInfo(const struct tm * pTimeInfo) 

Sets the hour, minute, and second fields from a struct tm.

```
void fromTimeInfo(const struct tm * pTimeInfo)
```

---

### void LocalTimeHMS::toTimeInfo(struct tm * pTimeInfo) const 

Fill in the tm_hour, tm_min, and tm_sec fields of a struct tm from the values in this object.

```
void toTimeInfo(struct tm * pTimeInfo) const
```

#### Parameters
* `pTimeInfo` The struct tm to modify

---

### void LocalTimeHMS::adjustTimeInfo(struct tm * pTimeInfo) const 

Adjust the values in a struct tm from the values in this object.

```
void adjustTimeInfo(struct tm * pTimeInfo) const
```

#### Parameters
* `pTimeInfo` The struct tm to modify

* After calling this, the values in the struct tm may be out of range, for example tm_hour > 23. This is fine, as calling mktime/gmtime normalizes this case and carries out-of-range values into the other fields as necessary.

---

### LocalTimeHMS & LocalTimeHMS::withHour(int hour) 

Sets this object to be the specified hour, with minute and second set to 0.

```
LocalTimeHMS & withHour(int hour)
```

#### Parameters
* `hour` 0 <= hour < 24 

#### Returns
LocalTimeHMS&

---

### LocalTimeHMS & LocalTimeHMS::withHourMinute(int hour, int minute) 

Sets this object to be the specified hour and minute, with second set to 0.

```
LocalTimeHMS & withHourMinute(int hour, int minute)
```

#### Parameters
* `hour` 0 <= hour < 24 

* `minute` 0 <= minute < 60 

#### Returns
LocalTimeHMS&

---

### int LocalTimeHMS::compareTo(const LocalTimeHMS & other) const 

Compare two LocalTimeHMS objects.

```
int compareTo(const LocalTimeHMS & other) const
```

#### Parameters
* `other` The item to compare to 

#### Returns
int -1 if this item is < other; 0 if this = other, or +1 if this > other

---

### bool LocalTimeHMS::operator==(const LocalTimeHMS & other) const 

Returns true if this item is equal to other. Compares hour, minute, and second.

```
bool operator==(const LocalTimeHMS & other) const
```

#### Parameters
* `other` 

#### Returns
true 

#### Returns
false

---

### bool LocalTimeHMS::operator!=(const LocalTimeHMS & other) const 

Returns true if this item is not equal to other.

```
bool operator!=(const LocalTimeHMS & other) const
```

#### Parameters
* `other` 

#### Returns
true 

#### Returns
false

---

### bool LocalTimeHMS::operator<(const LocalTimeHMS & other) const 

Returns true if this item is < other.

```
bool operator<(const LocalTimeHMS & other) const
```

#### Parameters
* `other` 

#### Returns
true 

#### Returns
false

---

### bool LocalTimeHMS::operator>(const LocalTimeHMS & other) const 

Returns true if this item is > other.

```
bool operator>(const LocalTimeHMS & other) const
```

#### Parameters
* `other` 

#### Returns
true 

#### Returns
false

---

### bool LocalTimeHMS::operator<=(const LocalTimeHMS & other) const 

Returns true if this item <= other.

```
bool operator<=(const LocalTimeHMS & other) const
```

#### Parameters
* `other` 

#### Returns
true 

#### Returns
false

---

### bool LocalTimeHMS::operator>=(const LocalTimeHMS & other) const 

Returns true if this item is >= other.

```
bool operator>=(const LocalTimeHMS & other) const
```

#### Parameters
* `other` 

#### Returns
true 

#### Returns
false

# class LocalTimeIgnoreHMS 

```
class LocalTimeIgnoreHMS
  : public LocalTimeHMS
```  

This class can be passed to most functions that take a LocalTimeHMS to instead not set the HMS.

## Members

---

###  LocalTimeIgnoreHMS::LocalTimeIgnoreHMS() 

Special version of LocalTimeHMS that does not set the HMS.

```
 LocalTimeIgnoreHMS()
```

# class LocalTimePosixTimezone 

Parses a Posix timezone string into its component parts.

For the Eastern US timezone, the string is: "EST5EDT,M3.2.0/2:00:00,M11.1.0/2:00:00"

* EST is the standard timezone name

* 5 is the offset in hours (the sign is backwards from the normal offset from UTC)

* EDT is the daylight saving timezone name

* M3 indicates that DST starts on the 3rd month (March)

* 2 is the week number (second week)

* 0 is the day of week (0 = Sunday)

* 2:00:00 at 2 AM local time, the transition occurs

* M11 indicates that standard time begins in the 11th month (November)

* 1 is the week number (first week)

* 0 is the day of week (0 = Sunday)

* 2:00:00 at 2 AM local time, the transition occurs

There are many other acceptable formats, including formats for locations that don't have DST.

For more information, see: [https://developer.ibm.com/technologies/systems/articles/au-aix-posix/](https://developer.ibm.com/technologies/systems/articles/au-aix-posix/)

## Members

---

### String dstName 

Daylight saving timezone name (empty string if no DST)

```
String dstName
```

---

### LocalTimeHMS dstHMS 

Daylight saving time shift (relative to UTC)

```
LocalTimeHMS dstHMS
```

---

### String standardName 

Standard time timezone name.

```
String standardName
```

---

### LocalTimeHMS standardHMS 

Standard time shift (relative to UTC). Note that this is positive in the United States, which is kind of backwards.

```
LocalTimeHMS standardHMS
```

---

### LocalTimeChange dstStart 

Rule for when DST starts.

```
LocalTimeChange dstStart
```

---

### LocalTimeChange standardStart 

Rule for when standard time starts.

```
LocalTimeChange standardStart
```

---

### bool valid 

true if the configuration looks valid

```
bool valid
```

---

###  LocalTimePosixTimezone::LocalTimePosixTimezone() 

Default constructor (no timezone set)

```
 LocalTimePosixTimezone()
```

---

###  LocalTimePosixTimezone::~LocalTimePosixTimezone() 

Destructor.

```
virtual  ~LocalTimePosixTimezone()
```

---

###  LocalTimePosixTimezone::LocalTimePosixTimezone(const char * str) 

Constructs the object with a specified timezone configuration.

```
 LocalTimePosixTimezone(const char * str)
```

Calls parse() internally.

---

### void LocalTimePosixTimezone::clear() 

Clears the timezone setting in this object.

```
void clear()
```

---

### bool LocalTimePosixTimezone::parse(const char * str) 

Parses the timezone configuration string.

```
bool parse(const char * str)
```

#### Parameters
* `str` The string, for example: "EST5EDT,M3.2.0/2:00:00,M11.1.0/2:00:00"

If the string is not valid this function returns false and the valid flag will be clear. You can call isValid() to check the validity at any time (such as if you are using the constructor with a string that does not return a boolean).

---

### bool LocalTimePosixTimezone::hasDST() const 

Returns true if this timezone configuration has daylight saving.

```
bool hasDST() const
```

---

### bool LocalTimePosixTimezone::isValid() const 

Returns true if this timezone configuration has been set and appears valid.

```
bool isValid() const
```

---

### bool LocalTimePosixTimezone::isZ() const 

Returns true if this timezone configuration is UTC.

```
bool isZ() const
```

# class LocalTimeValue 

```
class LocalTimeValue
  : public tm
```  

Container for a local time value with accessors similar to the Wiring Time class.

Really just a C++ wrapper around struct tm with adjustments for weekday and month being 0-based in struct tm and 1-based in Wiring. Also tm_year being weird in struct tm.

If you want to format a time string, use the methods in LocalTimeConvert. The reason is that the LocalTimeValue is only the value container and doesn't know the current timezone offset for the local time.

## Members

---

### int LocalTimeValue::hour() const 

Returns the hour (0 - 23)

```
int hour() const
```

---

### int LocalTimeValue::hourFormat12() const 

Returns the hour (1 - 12) used in AM/PM mode.

```
int hourFormat12() const
```

---

### uint8_t LocalTimeValue::isAM() const 

Returns true if the time is in the AM (before noon)

```
uint8_t isAM() const
```

---

### uint8_t LocalTimeValue::isPM() const 

Returns true if the time is in the PM (>= 12:00:00 in 24-hour clock).

```
uint8_t isPM() const
```

---

### int LocalTimeValue::minute() const 

Returns the minute 0 - 59.

```
int minute() const
```

---

### int LocalTimeValue::second() const 

Returns the second 0 - 59.

```
int second() const
```

---

### int LocalTimeValue::day() const 

Returns the day of the month 1 - 31 (or less in some months)

```
int day() const
```

---

### int LocalTimeValue::weekday() const 

Returns the day of week 1 - 7 (Sunday = 1, Monday = 2, ..., Saturday = 7)

```
int weekday() const
```

Note: the underlying struct tm tm_wday is 0 - 6 (Sunday = 0, Monday = 1, ..., Saturday = 6) but Wiring uses 1 - 7 instead of 0 - 6.

---

### int LocalTimeValue::month() const 

Returns the month of the year 1 - 12 (1 = January, 2 = February, ...)

```
int month() const
```

Note: the underlying struct tm tm_mon is 0 - 11, but this returns the more common 1 - 12.

---

### int LocalTimeValue::year() const 

Returns the 4-digit year.

```
int year() const
```

---

### LocalTimeHMS LocalTimeValue::hms() const 

Gets the local time as a LocalTimeHMS object.

```
LocalTimeHMS hms() const
```

---

### void LocalTimeValue::setHMS(LocalTimeHMS hms) 

Sets the local time from a LocalTimeHMS object.

```
void setHMS(LocalTimeHMS hms)
```

---

### time_t LocalTimeValue::toUTC(LocalTimePosixTimezone config) const 

Converts the specified local time into a UTC time.

```
time_t toUTC(LocalTimePosixTimezone config) const
```

There are some caveats to this that occur on when the time change occurs. On spring forward, there is an hour that doesn't technically map to a UTC time. For example, in the United States, 2:00 AM to 3:00 AM local time doesn't exist because at 2:00 AM local time, the local time springs forward to 3:00 AM.

When falling back, the hour from 1:00 AM to 2:00 AM is not unique, because it happens twice, once in DST before falling back, and a second time after falling back. The toUTC() function returns the second one that occurs in standard time.

---

### void LocalTimeValue::fromString(const char * str) 

Converts time from ISO-8601 format, ignoring the timezone.

```
void fromString(const char * str)
```

#### Parameters
* `str` The string to convert

The string should be of the form:

YYYY-MM-DDTHH:MM:SS

The T can be any single character, such as a space. For example:

2021-04-01 10:00:00

Any characters after the seconds are ignored.

---

### int LocalTimeValue::ordinal() const 

Returns which week of this day it is.

```
int ordinal() const
```

For example, if this day is a Friday and it's the first Friday of the month, then 1 is returned. If it's the second Friday, then 2 is returned.

(This is different than the week number of the month, which depends on which day you begin the week on.)

# class LocalTimeConvert::Schedule 

A complete time schedule.

A time schedule consists of minute multiples ("every 15 minutes"), optionally within a time range (all day, or from 09:00:00 to 17:00:00 local time, for example.

It can also have hour multiples, optionally in a time range, at a defined minute ("every 4 hours at :15 
past the hour").

It can also have any number of specific times in the day ("at 08:17:30 local time, 18:15:20 local time").

## Members

---

### std::vector< ScheduleItemMinuteMultiple > minuteMultipleItems 

Minute multiple items.

```
std::vector< ScheduleItemMinuteMultiple > minuteMultipleItems
```

---

### std::vector< LocalTimeHMS > times 

Local time items (includes hour multiple items)

```
std::vector< LocalTimeHMS > times
```

---

###  LocalTimeConvert::Schedule::Schedule() 

Construct a new, empty schedule.

```
 Schedule()
```

---

### Schedule & LocalTimeConvert::Schedule::withMinuteMultiple(int minuteMultiple) 

Adds a minute multiple schedule all day.

```
Schedule & withMinuteMultiple(int minuteMultiple)
```

#### Parameters
* `minuteMultiple` Number of minutes (must be 1 <= minutes <= 59). A value that is is divisible by is recommended.

This schedule publishes every n minutes within the hour. This really is every hour, not rolling, so you should use a value that 60 is divisible by (2, 3, 4, 5, 6, 10, 12, 15, 20, 30) otherwise there will be an inconsistent period at the top of the hour.

If you want to schedule at a minute offset as well, for example every 15 minutes at 02:00, 17:00, 32:00, 47:00, see the overload with a time range.

#### Returns
Schedule&

---

### Schedule & LocalTimeConvert::Schedule::withMinuteMultiple(int minuteMultiple, TimeRange timeRange) 

Adds a minute multiple schedule in a time range.

```
Schedule & withMinuteMultiple(int minuteMultiple, TimeRange timeRange)
```

#### Parameters
* `minuteMultiple` Number of minutes (must be 1 <= minutes <= 59). A value that is is divisible by is recommended. 

* `timeRange` When to apply this minute multiple and/or minute offset.

This schedule publishes every n minutes within the hour. This really is every hour, not rolling, so you should use a value that 60 is divisible by (2, 3, 4, 5, 6, 10, 12, 15, 20, 30) otherwise there will be an inconsistent period at the top of the hour.

If you specify a time range that does not start at 00:00:00 you can customize which minute the schedule starts at. For example: `15, LocalTimeConvert::TimeRange(LocalTimeHMS("00:05:00"), LocalTimeHMS("23:59:59")` will schedule every 15 minutes, but starting at 5 minutes past the hour, so 05:00, 20:00, 35:00, 50:00.

The largest value for hmsEnd of the time range is 23:59:59.

#### Returns
Schedule&

---

### Schedule & LocalTimeConvert::Schedule::withMinuteMultiple(ScheduleItemMinuteMultiple item) 

Adds a minute multiple schedule from a ScheduleItemMinuteMultiple object.

```
Schedule & withMinuteMultiple(ScheduleItemMinuteMultiple item)
```

#### Parameters
* `item` 

#### Returns
Schedule&

---

### Schedule & LocalTimeConvert::Schedule::withTime(LocalTimeHMS hms) 

Add a scheduled item at a time in local time during the day.

```
Schedule & withTime(LocalTimeHMS hms)
```

#### Parameters
* `hms` The time in local time 00:00:00 to 23:59:59. 

#### Returns
Schedule&

You can call this multiple times, and also combine it with minute multiple schedules.

---

### Schedule & LocalTimeConvert::Schedule::withTimes(std::initializer_list< LocalTimeHMS > timesParam) 

Add multiple scheduled items at a time in local time during the day.

```
Schedule & withTimes(std::initializer_list< LocalTimeHMS > timesParam)
```

#### Parameters
* `timesParam` an auto-initialized list of LocalTimeHMS objects 

#### Returns
Schedule&

You can call this multiple times, and also combine it with minute multiple schedules.

schedule.withTimes({LocalTimeHMS("06:00"), LocalTimeHMS("18:30")});

---

### Schedule & LocalTimeConvert::Schedule::withHours(std::initializer_list< int > hoursParam, int atMinute) 

Adds multiple scheduled hours at a time in a local time during the day at a specified minute.

```
Schedule & withHours(std::initializer_list< int > hoursParam, int atMinute)
```

#### Parameters
* `hoursParam` 

* `atMinute` 

#### Returns
Schedule&

Example: Schedules at 00:05, 06:05, 09:05, 10:05, 12:05, 15:05, 18:05

schedule.withHours({0, 6, 9, 10, 12, 15, 18}, 5);

This is just a shortcut that is easier to type than using the version that takes LocalTimeHMS objects.

---

### Schedule & LocalTimeConvert::Schedule::withHourMultiple(int hourMultiple, TimeRange timeRange) 

Adds multiple times periodically in a time range with an hour increment.

```
Schedule & withHourMultiple(int hourMultiple, TimeRange timeRange)
```

#### Parameters
* `hourMultiple` Hours between items must be >= 1. For example: 2 = every other hour. 

* `timeRange` Time range to add items to. This is optional; if not specified then the entire day. Also is used to specify a minute offset.

#### Returns
Schedule&

Hours are per day, local time. For whole-day schedules, you will typically use a value that 24 is evenly divisible by (2, 3, 4, 6, 8, 12), because otherwise the time periods will brief unequal at the top of the hour.

Also note that times are local, and take into account daylight saving. Thus during a time switch, the interval may end up being a different number of hours than specified. For example, if the times would have been 00:00 and 04:00, a hourMultiple of 4, and you do this over a spring forward, the actual number hours between 00:00 and 04:00 is 5 (at least in the US where DST starts at 2:00).

---

### Schedule & LocalTimeConvert::Schedule::withHourMultiple(int hourStart, int hourMultiple, int atMinute, int hourEnd) 

Adds multiple times periodically in a time range with an hour increment.

```
Schedule & withHourMultiple(int hourStart, int hourMultiple, int atMinute, int hourEnd)
```

#### Parameters
* `hourStart` Hour to start at 0 <= hourStart <= 23 

* `hourMultiple` Increment for hours 

* `atMinute` Minute past the hour for each item. Seconds is always 0. 

* `hourEnd` Hour to end, inclusive. 0 <= hourEnd <= 23 

#### Returns
Schedule&

# class LocalTimeConvert::ScheduleItemMinuteMultiple 

Schedule option for "every n minutes".

## Members

---

### TimeRange timeRange 

Range of local time, inclusive.

```
TimeRange timeRange
```

---

### int minuteMultiple 

Increment for minutes. Typically a value 60 is evenly divisible by.

```
int minuteMultiple
```

---

###  LocalTimeConvert::ScheduleItemMinuteMultiple::ScheduleItemMinuteMultiple() 

Default constructor. Set minuteMultiple and optionally timeRange to use.

```
 ScheduleItemMinuteMultiple()
```

---

###  LocalTimeConvert::ScheduleItemMinuteMultiple::ScheduleItemMinuteMultiple(int minuteMultiple, TimeRange timeRange) 

Construct an item with a time range and number of minutes.

```
 ScheduleItemMinuteMultiple(int minuteMultiple, TimeRange timeRange)
```

#### Parameters
* `minuteMultiple` Number of minutes (must be 1 <= minutes <= 59). A value that is is divisible by is recommended. 

* `timeRange` When to apply this minute multiple (optional)

This schedule publishes every n minutes within the hour. This really is every hour, not rolling, so you should use a value that 60 is divisible by (2, 3, 4, 5, 6, 10, 12, 15, 20, 30) otherwise there will be an inconsistent period at the top of the hour.

If you do not specify a time range, the entire day is the range, and it will always start at the top of the hour.

If you specify a time range that does not start at 00:00:00 you can customize which minute the schedule starts at. For example: `15, LocalTimeConvert::TimeRange(LocalTimeHMS("00:05:00"), LocalTimeHMS("23:59:59")` will schedule every 15 minutes, but starting at 5 minutes past the hour, so 05:00, 20:00, 35:00, 50:00.

---

### bool LocalTimeConvert::ScheduleItemMinuteMultiple::isValid() const 

Returns true if minuteMultiple is non-zero.

```
bool isValid() const
```

#### Returns
true 

#### Returns
false

This is used to check if an object was constructed by the default constructor and never set.

---

### time_t LocalTimeConvert::ScheduleItemMinuteMultiple::getTimeSpan(const LocalTimeConvert & conv) const 

Get number of seconds in the time range at a given time.

```
time_t getTimeSpan(const LocalTimeConvert & conv) const
```

#### Parameters
* `conv` The timezone and date information for time span calculation 

#### Returns
time_t

The conv object is necessary because getTimeSpan takes into account daylight saving transitions. When springing forward to daylight saving, from 01:15:00 to 03:15:00 is only one hour because of the DST transition.

# class LocalTimeConvert::TimeRange 

Class to hold a time range in local time in HH:MM:SS format.

## Members

---

### LocalTimeHMS hmsStart 

Starting time, inclusive.

```
LocalTimeHMS hmsStart
```

---

### LocalTimeHMS hmsEnd 

Ending time, inclusive.

```
LocalTimeHMS hmsEnd
```

---

###  LocalTimeConvert::TimeRange::TimeRange() 

Construct a new Time Range object with the range of the entire day (inclusive)

```
 TimeRange()
```

This is start = 00:00:00, end = 23:59:59. The system clock does not have a concept of leap seconds.

---

###  LocalTimeConvert::TimeRange::TimeRange(LocalTimeHMS hmsStart, LocalTimeHMS hmsEnd) 

Construct a new Time Range object with the specifies start and end times.

```
 TimeRange(LocalTimeHMS hmsStart, LocalTimeHMS hmsEnd)
```

#### Parameters
* `hmsStart` Start time in local time 00:00:00 <= hmsStart <= 23:59:59 

* `hmsEnd` End time in local time 00:00:00 <= hmsStart <= 23:59:59

Note that 24:00:00 is not a valid time. You should generally use inclusive times such that 23:59:59 is the end of the day.

---

### time_t LocalTimeConvert::TimeRange::getTimeSpan(const LocalTimeConvert & conv) const 

Get the number of seconds between start and end based on a LocalTimeConvert object.

```
time_t getTimeSpan(const LocalTimeConvert & conv) const
```

The reason for the conv object is that it contains the time to calculate at, as well as the daylight saving time settings. This methods takes into account the actual number of seconds including when a time change is crossed.

#### Parameters
* `conv` The time and timezone settings to calculate the time span at 

#### Returns
time_t Time difference in seconds

In the weird case that start > end, it can return a negative value, as time_t is a signed long (or long long) value.

Generated by [Moxygen](https://sourcey.com/moxygen)