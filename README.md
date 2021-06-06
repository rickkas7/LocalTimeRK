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


## Version history

### 0.0.1 (2021-06-06) 

- Initial version