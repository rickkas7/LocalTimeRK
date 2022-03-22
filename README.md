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

- Every 15 minutes in the hour, between 9:00 AM and 5:00 PM local time, Monday - Friday, except on 2022-03-21
- Every 4 hours of the day starting at 00:00:00 (midnight) other times

This can either be specified in code, or it can be expressed in JSON. JSON allows the schedule to be updating using a Particle.function, for example. 

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


### LocalTimeSchedule items

LocalTimeSchedule include things like every n minutes, every n hours, as well as day of week and day of month multiples. Each multiple has a type, an increment, in some cases additional data, and a `TimeRangeRestricted` that determines when the multiple is used.

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


---


## Version history

### 0.0.8 (2022-03-22)

- Added advanced scheduling modes (JSON scheduling)

### 0.0.7 (2022-03-08)

- Added inLocalTimeRange and nextTimeList functions
- Added LocalTimeSchedule

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