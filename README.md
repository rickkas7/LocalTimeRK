# LocalTimeRK

*Timezone and DST handling for Particle devices*

This library is designed for specific local time and daylight saving time scenarios on Particle devices. It is not intended to solve all problems in all situations, however it will work well in certain scenarios:

- [Full API docs](https://rickkas7.github.io/LocalTimeRK/)
- Github repository: [https://github.com/rickkas7/LocalTimeRK/](https://github.com/rickkas7/LocalTimeRK/)
- License: MIT

**What it's good for:**

- Managing local time, and daylight saving time, if needed, on devices in a known location
- Mostly intended for devices in your own home
- Managing scheduling of tasks at a specific local time
- Displaying local time

**What it's not intended for:**

- Mobile devices that may change locations
- Customer devices that could be in any timezone

## Features

- Timezone configuration using a POSIX timezone rule string
- Does not require network access to determine timezone and daylight saving transitions
- Good for displaying local time, such as on clock-like devices
- Good for scheduling operations at a specific local time. For example, every day at 8:00 AM regardless of timezone and DST
- Support for locations with DST and without DST (timezone only)
- Should work in the southern hemisphere were DST is opposite on the calendar
- Should work in any country as long as a compatible POSIX timezone configuration string can be generated

## What is a POSIX timezone configuration?

For the United States east coast, the configuration string is:

```
EST5EDT,M3.2.0/2:00:00,M11.1.0/2:00:00"
```

What this means is:

- "EST" is the standard time timezone name
- 5 is the offset from UTC in hours. Actually -5, as the sign is backwards from the way it's normally expressed. And it could be hours and minutes.
- "EDT" is the daylight saving time timezone name
- "M3.2.0" is when DST starts. Month 3 (March), 2nd week, 0 = Sunday
- "2:00:00" transition to DST at 2:00 AM local time
- "M11.1.0" transition back to standard time November, 1st week, on Sunday
- "2:00:00" transition back to standard time at 2:00 AM local time

There are a bunch of supported formats, including for locations that don't have DST (such as Arizona, "MST7") and southern hemisphere where daylight saving is on the opposite part of the year, such as Eastern Australian time used in Sydney, Australia "AEST-10AEDT,M10.1.0/02:00:00,M4.1.0/03:00:00".

## Using for scheduling

This library does not modify the `Time` class timezone! This has the potential to disrupt all sorts of things that depend on `Time`, and should be avoided. A new class and methods provide access to local time when needed.

Additionally, it's designed to handle scheduling. For example, say you want to perform an operation at 3:00 AM every day, local time. The class can find the UTC time corresponding to this, taking into account timezones and DST changes. Using `Time.now()` comparisons (at UTC) is fast and efficient. It's also good when you want to store the desired time in EEPROM, retained memory, or the file system.

The library also handles weird transition scenarios that occur on spring forward (in the north hemisphere) where the hour from 2:00 AM local time to 2:59:59 doesn't exist, and in the fall back where the hour from 1:00:00 to 1:59:59 local time occurs twice.

It also handles other common scheduling scenarios:

- Tomorrow (same time)
- Tomorrow (at a specific local time)
- On a specific day of week at a specific time ("every Saturday at 3:00 AM")
- On a day of week with an ordinal ("on the 2nd Saturday of the month")
- On a specific day of month at a specific time
- Also the last day of the month, the second to last day of the month, ...
- On the next weekday (Monday - Friday)
- On the next weekend day (Saturday - Sunday)

## Advanced scheduling

An advanced scheduling mode was added in version 0.1.0. This allows complex schedules such as:

- Every 15 minute of the hour (between 9:00 AM and 5:00 PM local time Monday - Friday, except on 2022-03-21
- Every 4 hours of the day starting at 00:00:00 (midnight) other times

This can either be specified in code, or it can be expressed in JSON. This allows the schedule to be updating using a Particle.function, for example. 

This is a compact representation of that schedule in 77 bytes of JSON data:

```
[{"mh":15,"y":62,"s":"09:00:00","e":"16:59:59","x":["2022-03-21"]},{"hd":4}]
```

While scheduling is designed to work with local time, each schedule calculator can optionally have a time zone override, which makes it possible to do some calculations at UTC if you prefer to do that.

### Shared types

#### LocalTimeHMS

This object holds a hour, minute, and second value (HMS). There are numerous methods to compare time values, and convert the values to other formats.

When converting from string format always use 24-hour clock format; this object does not support AM/PM. Also when converting from strings you can omit the seconds, or even both the minutes and seconds.

Note that hour 24 is never a valid value. Because HMS calculations are always inclusive, the end of the day is 23:59:59. Leap seconds are not supported by the underlying C standard time library.

#### LocalTimeYMD

This object specifies a year, month, and day. There are numerous methods to compare time values, and convert the values to other formats.

When converting from a string this must always be "YYYY-MM-DD" format, with dashes, and in that order. Other date formats including common but poorly defined United States date formats cannot be used!

#### LocalTimeRange

A `LocalTimeRange` is a start time and end time in local time, expressed as `LocalTimeHMS` objects (hours, minutes, seconds). 

Time ranges are inclusive, so the entire day is 00:00:00 to 23:59:59.

When converting from JSON:

| Key | Type | Description | Default |
| :--- | :--- | :--- | :--- |
| "s" | string | Starting time in "HH:MM:SS" format, 24 hour clock, local time | "00:00:00" |
| "e" | string | Ending time in "HH:MM:SS" format, 24 hour clock, local time | "23:59:59" |


#### LocalTimeDayOfWeek

A `LocalTimeDayOfWeek` specifies zero or more days of the week (Sunday, Monday, ...)

Common mask values:

| Value Hex | Value Decimal | Constant | Description |
| :--- | ---: | :--- |
| 0x01 | 1 | MASK_SUNDAY | Sunday |
| 0x02 | 2 | MASK_MONDAY | Monday |
| 0x04 | 4 | MASK_TUESDAY | Tuesday |
| 0x08 | 8 | MASK_WEDNESDAY | Wednesday |
| 0x10 | 16 | MASK_THURSDAY | Thursday |
| 0x20 | 32 | MASK_FRIDAY | Friday |
| 0x40 | 64 | MASK_SATURDAY | Saturday | 
| 0x7f | 127 | MASK_ALL | Every day |
| 0x3e | 62 | MASK_WEEKDAY | Weekdays (Monday - Friday) | 
| 0x41 | 65 | MASK_WEEKEND | Weekends (Saturday - Sunday) |

Note that you can combine any mask bits. For example, Monday, Wednesday, Friday is 2 + 8 + 32 = 42,

In JSON, you can only use the numeric mask values, as a decimal number. For example, weekdays Monday - Friday is 62.

| Key | Type | Description |
| :--- | :--- | :--- |
| "y" | number | Mask value for days of the week, see table above |

#### LocalTimeRestrictedDate

A `LocalTimeRestrictedDate` is used for both multiples and times (below). It can have:

- A day of week selection (`LocalTimeDayOfWeek`), which is a bitmask of days to allow. This makes it easy to specific, for example, weekdays (Monday - Friday).
- A list of dates to allow (vector of `LocalTimeYMD`)
- A list of dates to exclude (vector of `LocalTimeYMD`)

| Key | Type | Description | Default |
| :--- | :--- | :--- | :--- |
| "y" | number | Mask value for days of the week, see table above (optional) |
| "a" | Array of string | Array of strings of the form YYYY-MM-DD to allow specific dates (optional) |
| "x" | Array of string | Array of strings of the form YYYY-MM-DD to exclude specific dates (optional) |


### Schedule items

Schedule include things like every n minutes, every n hours, as well as day of week and day of month multiples. Each multiple has a type, an increment, in some cases additional data, and a `TimeRangeRestricted` that determines when the multiple is used.

`TimeRangeRestricted` is itself composed of a `TimeRange`, and a `LocalTimeRestrictedDate`. This specifies both the time of day, as well as an optional restriction on the dates it applies. See above for for an explanation of these types.

#### Minute multiples

This multiple is used for "every n minutes." For example, if you want to publish an event every 15 minutes.

- Minute multiples are relative to the hour, so you typically want to use a value that 60 is evenly divisible by in order to keep the period constant (2, 3, 4, 5, 6, 10, 12, 15, 20, 30).
- A `TimeRangeRestricted` can restrict this to certain hours of the day, and certain days of the week.
- A `TimeRangeRestricted` can also handle exception dates, both only on date, or to exclude dates.
- The minute of the start of the time range specifies the offset relative to the hour to start the increment from (modulo the increment).
- The second of the start of the time range specifies the second offset.

| Key | Type | Description | Default |
| :--- | :--- | :--- | :--- |
| "mh" | integer | Minute of hour multiple | |
| "f" | integer | Flag bits (optional) | 0 |
| "s" | string | The start time (HH:MM:SS format, can omit MM or SS) | "00:00:00" |
| "e" | string | The end time (HH:MM:SS format, can omit MM or SS) | "23:59:59" |
| "y" | integer | Mask value for days of the week (optional) |
| "a" | Array of string | Array of strings of the form YYYY-MM-DD to allow specific dates (optional) |
| "x" | Array of string | Array of strings of the form YYYY-MM-DD to exclude specific dates (optional) |


#### Hour of day multiples

This multiple is used for "every n hours." For example, if you want to wake and publish every 4 hours.

- Hour multiples are relative to the day, so you typically want to use a value that 24 is evenly divisible by (2, 3, 4, 6, 8, 12).
- A `TimeRangeRestricted` can restrict this to certain hours of the day, and certain days of the week.
- A `TimeRangeRestricted` can also handle exception dates, both only on date, or to exclude dates.
- The hour of the start of the time range specifies the offset relative to the day to start the increment from (modulo the increment)
- The minute and second of the start of the time range specifies the minute and second offset

| Key | Type | Description | Default |
| :--- | :--- | :--- | :--- |
| "hd" | integer | Hour of day multiple | |
| "f" | integer | Flag bits (optional) | 0 |
| "s" | string | The start time (HH:MM:SS format, can omit MM or SS) | "00:00:00" |
| "e" | string | The end time (HH:MM:SS format, can omit MM or SS) | "23:59:59" |
| "y" | integer | Mask value for days of the week (optional) |
| "a" | Array of string | Array of strings of the form YYYY-MM-DD to allow specific dates (optional) |
| "x" | Array of string | Array of strings of the form YYYY-MM-DD to exclude specific dates (optional) |


#### Day of week of the month 

This is used for things like: "Every first Monday of the month," "Every second Tuesday of the month," or "Last Friday of the month."

- The dayOfWeek specifies the day of the week (Sunday = 0, Monday = 1, Tuesday = 2, ..., Saturday = 6).
- The increment specifies which instance (1 = first, 2 = second, ... Or -1 = last, -2 = second to last, ...)
- A `TimeRangeRestricted` can handle exception dates, both only on date, or to exclude dates.
- The start of the time range specifies the hour, minute, and second (local time)
- Use a time of day with day of week restriction instead if you want to do "Every Monday"

| Key | Type | Description | Default |
| :--- | :--- | :--- | :--- |
| "dw" | integer | Day of week instance (1 = first, 2 = second, ..., or -1 = last, -2 = second to last, ... | |
| "d" | integer | Day of the week 0 = Sunday, 1 = Monday, ..., 6 = Saturday |
| "f" | integer | Flag bits (optional) | 0 |
| "s" | string | The start time (HH:MM:SS format, can omit MM or SS) | "00:00:00" |
| "e" | string | The end time (HH:MM:SS format, can omit MM or SS) | "23:59:59" |
| "y" | integer | Mask value for days of the week (optional) |
| "a" | Array of string | Array of strings of the form YYYY-MM-DD to allow specific dates (optional) |
| "x" | Array of string | Array of strings of the form YYYY-MM-DD to exclude specific dates (optional) |


#### Day of month 

This is used for things like "The 1st of the month," "The 15th of the month," or "the last day of the month."

- The increment specifies which instance (1 = 1st of the month, 2 = 2nd of the month, ... Or -1 = last day, -2 = second to last day, ...)
- A `TimeRangeRestricted` can handle exception dates, both only on date, or to exclude dates.
- The start of the time range specifies the hour, minute, and second (local time)


| Key | Type | Description | Default |
| :--- | :--- | :--- | :--- |
| "dw" | integer | Day of month instance (1 = 1st, 2 = 2nd, ... or -1 = last day of month, -2 = second to last, ...| |
| "f" | integer | Flag bits (optional) | 0 |
| "s" | string | The start time (HH:MM:SS format, can omit MM or SS) | "00:00:00" |
| "e" | string | The end time (HH:MM:SS format, can omit MM or SS) | "23:59:59" |
| "y" | integer | Mask value for days of the week (optional) |
| "a" | Array of string | Array of strings of the form YYYY-MM-DD to allow specific dates (optional) |
| "x" | Array of string | Array of strings of the form YYYY-MM-DD to exclude specific dates (optional) |

#### Time schedule

It's also possible to schedule at a specific time in local time. In code, this is an array of `LocalTimeHMSRestricted` objects. You are limited only by RAM for the number of objects, as the array is stored in a std::vector.

The `LocalTimeHMSRestricted` is itself composed of a `LocalTimeHMS` object, for hours minutes and seconds, and a `LocalTimeRestrictedDate` which can optionally restrict which dates the times re used.

Some ways you can use times:

- "At 17:00:00" every day (local time)
- "At 17:00:00, Monday - Friday"
- "At 09:00:00, Monday - Friday, except for 2022-03-21"
- "At 23:59:59 on 2022-03-31"

| Key | Type | Description | Default |
| :--- | :--- | :--- | :--- |
| "tm" | string | Time in "HH:MM:SS" format, 24 hour clock, local time |
| "f" | integer | Flag bits (optional) | 0 |
| "y" | number | Mask value for days of the week (optional) |
| "a" | Array of string | Array of strings of the form YYYY-MM-DD to allow specific dates (optional) |
| "x" | Array of string | Array of strings of the form YYYY-MM-DD to exclude specific dates (optional) |



## Clock Example

There is an example of using the library to use an Adafruit FeatherWing OLED Display 128x32 as a clock.

The display is available at [Adafruit](https://www.adafruit.com/product/2900). You can find more technical information [at Adafruit](https://learn.adafruit.com/adafruit-oled-featherwing/overview). The library is available in the Web IDE as [oled-wing-adafruit](https://build.particle.io/libs/oled-wing-adafruit/0.0.4/tab/oled-wing-adafruit.cpp). You can find additional documentation [here](https://github.com/rickkas7/oled-wing-adafruit).

The code example is in the more-examples directory in the Clock directory and is quite simple:

```cpp
#include "Particle.h"

#include "LocalTimeRK.h"
#include "oled-wing-adafruit.h"

SerialLogHandler logHandler;

SYSTEM_THREAD(ENABLED);

OledWingAdafruit display;

void setup() {
    // Set timezone to the Eastern United States
    LocalTime::instance().withConfig(LocalTimePosixTimezone("EST5EDT,M3.2.0/2:00:00,M11.1.0/2:00:00"));

	display.setup();

	display.clearDisplay();
	display.display();
}

void loop() {
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate >= 1000 && Time.isValid()) {
        lastUpdate = millis();

        LocalTimeConvert conv;
        conv.withCurrentTime().convert();

        // Log.info("local time: %s", conv.format(TIME_FORMAT_ISO8601_FULL).c_str());
        
        // Room for 11 characters at text size 2
        display.clearDisplay();
        display.setTextColor(WHITE);
        display.setTextSize(2);
        display.setFont(NULL);
        display.setCursor(0, 1);

        String msg;

        msg = conv.format("%Y-%m-%d"); // 2021-06-07
        display.println(msg);

        msg = conv.format("%I:%M:%S%p"); // 10:00:00AM
        display.println(msg);

        display.display();
    }
}
```

![](images/clock.png)


## Using the library


### Configure the timezone

- Determine the POSIX timezone string for your location. For example, the US eastern timezone is shown in the example

- At setup, configure the timezone:

```cpp
LocalTime::instance().withConfig(LocalTimePosixTimezone("EST5EDT,M3.2.0/2:00:00,M11.1.0/2:00:00"));
```

Some configuration strings:

| Location | Timezone Configuration |
| :--- | :--- |
| New York | "EST5EDT,M3.2.0/02:00:00,M11.1.0/02:00:00" |
| Chicago | "CST6CDT,M3.2.0/2:00:00,M11.1.0/2:00:00" |
| Denver | "MST7MDT,M3.2.0/2:00:00,M11.1.0/2:00:00" |
| Phoenix | "MST7" |
| Los Angeles | "PST8PDT,M3.2.0/2:00:00,M11.1.0/2:00:00" |
| London | "BST0GMT,M3.5.0/1:00:00,M10.5.0/2:00:00" |
| Sydney, Australia | "AEST-10AEDT,M10.1.0/02:00:00,M4.1.0/03:00:00" | 
| Adelaide, Australia | "ACST-9:30ACDT,M10.1.0/02:00:00,M4.1.0/03:00:00" |
| UTC | "UTC" |

### Getting the current local time

Use the `LocalTimeConvert` class like this to get the current time:

```cpp
LocalTimeConvert conv;
conv.withCurrentTime().convert();
```

This will initialize the `conv` object with information about the current local time in the timezone you have configured.

Once you have the converted time, you can use methods such as:

#### bool LocalTimeConvert::isDST() const 

Returns true if the current time is in daylight saving time.

```
bool isDST() const
```

---

#### bool LocalTimeConvert::isStandardTime() const 

Returns true of the current time in in standard time.

```
bool isStandardTime() const
```


---

#### String LocalTimeConvert::timeStr() 

Works like Time.timeStr() to generate a readable string of the local time.

```
String timeStr()
```

Uses asctime formatting, which looks like "Fri Jan  1 18:45:56 2021". The strings are not localized; they're always in English.

---

#### String LocalTimeConvert::format(const char * formatSpec) 

Works like Time.format()

```
String format(const char * formatSpec)
```

##### Parameters
* `formatSpec` the format specifies, which can be

* TIME_FORMAT_DEFAULT (example: "Thu Apr  1 12:00:00 2021")

* TIME_FORMAT_ISO8601_FULL (example: "2021-04-01T12:00:00-04:00")

* custom format based on strftime()

There are many options to strftime described here: [https://www.cplusplus.com/reference/ctime/strftime/?kw=strftime](https://www.cplusplus.com/reference/ctime/strftime/?kw=strftime)

Unlike Time.format(), you can use Z to output the timezone abbreviation, for example "EDT" for the Eastern United States, daylight saving instead of -04:00.

The z formatting matches that of Time.format(), which is wrong. The correct output should be "-400" but the output will be "-04:00" for compatibility.

If you want to make a US-style AM/PM clock display, the formatting string "%I:%M %p" will produce something like "12:30 PM".

---

#### String LocalTimeConvert::zoneName() const 

Returns the abbreviated time zone name for the current time.

```
String zoneName() const
```

For example, for the United States east coast, EST or EDT depending on whether the current time is DST or not. See also isDST().

This string comes from the LocalTimePosixTimezone object.


### Converting UTC to local time

Use the `LocalTimeConvert` class like this to get the current time:

```cpp
time_t timeVal = 1612393852; // Wednesday, February 3, 2021 11:10:52 PM

LocalTimeConvert conv;
conv.withTime(timeVal).convert();
```

### Scheduling

The scheduling methods move the time set in the conv object forward to schedule a time in the future. These methods include:

#### void LocalTimeConvert::nextDay(LocalTimeHMS hms) 

Moves the current time to the next day.

```
void nextDay(LocalTimeHMS hms)
```

##### Parameters
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

#### bool LocalTimeConvert::nextDayOfWeek(int dayOfWeek, LocalTimeHMS hms) 

Moves the current time to the next of the specified day of week.

```
bool nextDayOfWeek(int dayOfWeek, LocalTimeHMS hms)
```

##### Parameters
* `dayOfWeek` The day of week (0 - 6, 0 = Sunday, 1 = Monday, ..., 6 = Saturday)

* `hms` If specified, moves to that time of day (local time). If omitted, leaves the current time and only changes the date.

Upon completion, all fields are updated appropriately. For example:

* time specifies the time_t of the new time at UTC

* localTimeValue contains the broken-out values for the local time

* isDST() return true if the new time is in daylight saving time

---

#### void LocalTimeConvert::nextWeekday(LocalTimeHMS hms) 

Returns the next day that is a weekday (Monday - Friday)

```
void nextWeekday(LocalTimeHMS hms)
```

##### Parameters
* `hms` If specified, moves to that time of day (local time). If omitted, leaves the current time and only changes the date.

Upon completion, all fields are updated appropriately. For example:

* time specifies the time_t of the new time at UTC

* localTimeValue contains the broken-out values for the local time

* isDST() return true if the new time is in daylight saving time

---

#### void LocalTimeConvert::nextWeekendDay(LocalTimeHMS hms) 

Returns the next day that is a weekend day (Saturday or Sunday)

```
void nextWeekendDay(LocalTimeHMS hms)
```

##### Parameters
* `hms` If specified, moves to that time of day (local time). If omitted, leaves the current time and only changes the date.

Upon completion, all fields are updated appropriately. For example:

* time specifies the time_t of the new time at UTC

* localTimeValue contains the broken-out values for the local time

* isDST() return true if the new time is in daylight saving time

---

#### bool LocalTimeConvert::nextDayOfMonth(int dayOfMonth, LocalTimeHMS hms) 

Moves the date and time (local time) forward to the specified day of month and local time.

```
bool nextDayOfMonth(int dayOfMonth, LocalTimeHMS hms)
```

##### Parameters
* `dayOfMonth` The day of the month (1 = first day of the month). See also special cases below.

* `hms` If specified, moves to that time of day (local time). If omitted, leaves the current time and only changes the date.

This version will move to the closest forward time. It could be as close as 1 second later, but it will always advance at least once second. It could be as much as 1 month minus 1 second later.

Upon completion, all fields are updated appropriately. For example:

* time specifies the time_t of the new time at UTC

* localTimeValue contains the broken-out values for the local time

* isDST() return true if the new time is in daylight saving time

dayOfMonth is normally a a day number (1 = first day of the month). There are also special cases: 0 = the last day of the month, -1 the second to last day of month, ... The number of days in the month are based on the date in local time.

---

#### bool LocalTimeConvert::nextDayOfNextMonth(int dayOfMonth, LocalTimeHMS hms) 

Moves the date and time (local time) forward to the specified day of month and local time.

```
bool nextDayOfNextMonth(int dayOfMonth, LocalTimeHMS hms)
```

##### Parameters
* `dayOfMonth` The day of the month (1 = first day of the month). See also special cases below.

* `hms` If specified, moves to that time of day (local time). If omitted, leaves the current time and only changes the date.

This version always picks the next month, even if the target day of month hasn't been reached in this month yet. This will always more forward at least a month, and may be as much as two months minus one day.

Upon completion, all fields are updated appropriately. For example:

* time specifies the time_t of the new time at UTC

* localTimeValue contains the broken-out values for the local time

* isDST() return true if the new time is in daylight saving time

dayOfMonth is normally a a day number (1 = first day of the month). There are also special cases: 0 = the last day of the month, -1 the second to last day of month, ... The number of days in the month are based on the date in local time.

---

#### bool LocalTimeConvert::nextDayOfWeekOrdinal(int dayOfWeek, int ordinal, LocalTimeHMS hms) 

Moves the date and time (local time) forward to the specified day of month and local time.

```
bool nextDayOfWeekOrdinal(int dayOfWeek, int ordinal, LocalTimeHMS hms)
```

##### Parameters
* `dayOfWeek` The day of week (0 - 6, 0 = Sunday, 1 = Monday, ..., 6 = Saturday)

* `ordinal` 1 = first of that day of week in month, 2 = second, ...

* `hms` If specified, moves to that time of day (local time). If omitted, leaves the current time and only changes the date.

If you specify an ordinal that does not exist (for example, there may not be a 5th ordinal for certain days in certain months), returns false and leaves the date unchanged.

Upon successful completion, all fields are updated appropriately. For example:

* time specifies the time_t of the new time at UTC

* localTimeValue contains the broken-out values for the local time

* isDST() return true if the new time is in daylight saving time

---

#### void LocalTimeConvert::nextLocalTime(LocalTimeHMS hms) 

Sets the time to the nearest hms in local time in the future.

```
void nextLocalTime(LocalTimeHMS hms)
```

##### Parameters
* `hms` The time of day to set in local time

Moves the time forward to the next instance of hms in local time. If hms has not occurred yet, it will select the one today, but it will always move the time forward.

There is a weird special case on the beginning of daylight saving (spring forward in the northern hemisphere). If you select a hms between 2:00 AM and 2:59:59 AM local time, this time does not exist on spring forward day because it's the hour that is skipped. In order to preserve the requirement that the time will always be advanced by this call, it will jump forward to the next day when the 2:00 hour occurs next. (The hour may be different in other locations, for example it's 1:00 AM in the UK.)

In the case of fall back, if you specify a hms in the repeated hour (1:00:00 to 1:59:59) the time returned will be the second time this time occurs, in standard time in the US.

Upon completion, all fields are updated appropriately. For example:

* time specifies the time_t of the new time at UTC

* localTimeValue contains the broken-out values for the local time

* isDST() return true if the new time is in daylight saving time

---

### void LocalTimeConvert::nextMinuteMultiple(int minuteMultiple) 

Moves the current time the next specified multiple of minutes.

```
void nextMinuteMultiple(int minuteMultiple)
```

#### Parameters
* `minuteMultiple` Typically something like 5, 15, 20, 30 that 60 is evenly divisible by

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

### void LocalTimeConvert::atLocalTime(LocalTimeHMS hms) 

Changes the time of day to the specified hms in local time on the same local day.

```
void atLocalTime(LocalTimeHMS hms)
```

##### Parameters
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


---


## Version history

### 0.1.0 (2022-03-21)

- Added advanced scheduling modes (JSON scheduling)

### 0.0.7 (2022-03-08)

- Added inLocalTimeRange and nextTimeList functions
- Added LocalTimeConvert::Schedule

### 0.0.6 (2022-03-08)

- Added nextMinuteMultiple() and nextTime() functions

### 0.0.5 (2021-07-08)

- Fixed bug in calculation of negative hour offsets that also included minutes
- Added unit test for Adelaide, Australia

### 0.0.4 (2021-06-26)

- Added nextDayOrTimeChange

### 0.0.3 (2021-06-11)

- Remove more-examples code from library

### 0.0.2 (2021-06-07)

- Add examples

### 0.0.1 (2021-06-06) 

- Initial version