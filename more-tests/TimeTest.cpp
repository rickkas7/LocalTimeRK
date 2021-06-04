#include "Particle.h"
#include "LocalTimeRK.h"

#include <time.h>

// This test program assumes it's run with TZ set to "UTC" so strftime prints the same format
// as a Particle device when using the native strftime. The Makefile calls it this way:
// 
// export TZ='UTC' && ./TimeTest

char *readTestData(const char *filename) {
	char *data;

	FILE *fd = fopen(filename, "r");
	if (!fd) {
		printf("failed to open %s", filename);
		return 0;
	}

	fseek(fd, 0, SEEK_END);
	size_t size = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	data = (char *) malloc(size + 1);
	fread(data, 1, size, fd);
	data[size] = 0;

	fclose(fd);

	return data;
}

#define assertInt(msg, got, expected) _assertInt(msg, got, expected, __LINE__)
void _assertInt(const char *msg, int got, int expected, int line) {
	if (expected != got) {
		printf("assertion failed %s line %d\n", msg, line);
		printf("expected: %d\n", expected);
		printf("     got: %d\n", got);
		assert(false);
	}
}

#define assertStr(msg, got, expected) _assertStr(msg, got, expected, __LINE__)
void _assertStr(const char *msg, const char *got, const char *expected, int line) {
	if (strcmp(expected, got) != 0) {
		printf("assertion failed %s line %d\n", msg, line);
		printf("expected: %s\n", expected);
		printf("     got: %s\n", got);
		assert(false);
	}
}

#define assertTime(msg, got, expected) _assertTime(msg, got, expected, __LINE__)
void _assertTime(const char *msg, time_t got, const char *expected, int line) {
	struct tm timeInfo;
	LocalTime::timeToTm(got, &timeInfo);
	String gotStr = LocalTime::getTmString(&timeInfo);
	if (strcmp(expected, gotStr) != 0) {
		printf("assertion failed %s line %d\n", msg, line);
		printf("expected: %s\n", expected);
		printf("     got: %s\n", gotStr.c_str());
		assert(false);
	}
}


const char *timeChanges[4] = {
	"M3.2.0/2:00:00",
	"M11.1.0/2:00:00",
	"M3.2.0/2:00:00",
	0
};

void testLocalTimeChange() {
	for(size_t ii = 0; timeChanges[ii]; ii++) {
		LocalTimeChange tc;

		tc.parse(timeChanges[ii]);
		if (strcmp(timeChanges[ii], tc.toString()) != 0) {
			printf("failed to parse local time change %s got %s", timeChanges[ii], tc.toString().c_str() );
		}
	}

	LocalTimeChange tc;
	tc.parse("M3.2.0/2:00:00");
	assert(tc.valid);
	assert(tc.hms.hour == 2);
	assert(tc.hms.minute == 0);
	assert(tc.hms.second == 0);

	tc.parse("M3.2.0/2:30");
	assert(tc.valid);
	assert(tc.hms.hour == 2);
	assert(tc.hms.minute == 30);
	assert(tc.hms.second == 0);

	tc.parse("M3.2.0/2:45:30");
	assert(tc.valid);
	assert(tc.hms.hour == 2);
	assert(tc.hms.minute == 45);
	assert(tc.hms.second == 30);

	tc.parse("M3.2.0/2");
	assert(tc.valid);
	assert(tc.hms.hour == 2);
	assert(tc.hms.minute == 0);
	assert(tc.hms.second == 0);

	tc.parse("M3.2.0/-1");
	assert(tc.valid);
	assert(tc.hms.hour == -1);
	assert(tc.hms.minute == 0);
	assert(tc.hms.second == 0);

	tc.parse("M3.2.0");
	assert(tc.valid);
	assert(tc.hms.hour == 0);
	assert(tc.hms.minute == 0);
	assert(tc.hms.second == 0);

	tc.parse("");
	assert(!tc.valid);

	tc.parse(0);
	assert(!tc.valid);

	tc.parse("3.2.0/2:00:00");
	assert(!tc.valid);

}

void testLocalTimePosixTimezone() {
	LocalTimePosixTimezone tz;

	// Iceland, no DST
	tz.parse("GMT");
	assert(strcmp(tz.standardName.c_str(), "GMT") == 0);
	assert(tz.standardHMS.hour == 0);
	assert(tz.standardHMS.minute == 0);
	assert(tz.standardHMS.second == 0);
	assert(tz.standardStart.valid == 0);
	assert(strcmp(tz.dstName.c_str(), "") == 0);
	assert(tz.dstStart.valid == 0);

	// Phoenix, AZ, no DST
	tz.parse("MST7");
	assert(strcmp(tz.standardName.c_str(), "MST") == 0);
	assert(tz.standardHMS.hour == 7);
	assert(tz.standardHMS.minute == 0);
	assert(tz.standardHMS.second == 0);
	assert(tz.standardStart.valid == 0);
	assert(strcmp(tz.dstName.c_str(), "") == 0);
	assert(tz.dstStart.valid == 0);

	// Greece, no DST
	tz.parse("MSK-3");
	assert(strcmp(tz.standardName.c_str(), "MSK") == 0);
	assert(tz.standardHMS.hour == -3);
	assert(tz.standardHMS.minute == 0);
	assert(tz.standardHMS.second == 0);
	assert(tz.standardStart.valid == 0);
	assert(strcmp(tz.dstName.c_str(), "") == 0);
	assert(tz.dstStart.valid == 0);

	// Western Australia, no DST
	tz.parse("ACWST-8:45");
	assert(strcmp(tz.standardName.c_str(), "ACWST") == 0);
	assert(tz.standardHMS.hour == -8);
	assert(tz.standardHMS.minute == 45);
	assert(tz.standardHMS.second == 0);
	assert(tz.standardStart.valid == 0);
	assert(strcmp(tz.dstName.c_str(), "") == 0);
	assert(tz.dstStart.valid == 0);


	// New York
	tz.parse("EST5EDT,M3.2.0/2:00:00,M11.1.0/2:00:00");
	assert(strcmp(tz.standardName.c_str(), "EST") == 0);
	assert(tz.standardHMS.hour == 5);
	assert(tz.standardHMS.minute == 0);
	assert(tz.standardHMS.second == 0);
	assert(tz.standardStart.month == 11);
	assert(tz.standardStart.week == 1);
	assert(tz.standardStart.dayOfWeek == 0);
	assert(tz.standardStart.hms.hour == 2);
	assert(tz.standardStart.hms.minute == 0);
	assert(tz.standardStart.hms.second == 0);
	assert(tz.standardStart.valid == 1);
	assert(strcmp(tz.dstName.c_str(), "EDT") == 0);
	assert(tz.dstStart.valid == 1);
	assert(tz.dstHMS.hour == 4);
	assert(tz.dstHMS.minute == 0);
	assert(tz.dstHMS.second == 0);
	assert(tz.dstStart.month == 3);
	assert(tz.dstStart.week == 2);
	assert(tz.dstStart.dayOfWeek == 0);
	assert(tz.dstStart.hms.hour == 2);
	assert(tz.dstStart.hms.minute == 0);
	assert(tz.dstStart.hms.second == 0);
	assert(tz.dstStart.valid == 1);

	// Chicago
	tz.parse("CST6CDT,M3.2.0/2:00:00,M11.1.0/2:00:00");
	assert(strcmp(tz.standardName.c_str(), "CST") == 0);
	assert(tz.standardHMS.hour == 6);
	assert(tz.standardHMS.minute == 0);
	assert(tz.standardHMS.second == 0);
	assert(tz.standardStart.month == 11);
	assert(tz.standardStart.week == 1);
	assert(tz.standardStart.dayOfWeek == 0);
	assert(tz.standardStart.hms.hour == 2);
	assert(tz.standardStart.hms.minute == 0);
	assert(tz.standardStart.hms.second == 0);
	assert(tz.standardStart.valid == 1);
	assert(strcmp(tz.dstName.c_str(), "CDT") == 0);
	assert(tz.dstStart.valid == 1);
	assert(tz.dstHMS.hour == 5);
	assert(tz.dstHMS.minute == 0);
	assert(tz.dstHMS.second == 0);
	assert(tz.dstStart.month == 3);
	assert(tz.dstStart.week == 2);
	assert(tz.dstStart.dayOfWeek == 0);
	assert(tz.dstStart.hms.hour == 2);
	assert(tz.dstStart.hms.minute == 0);
	assert(tz.dstStart.hms.second == 0);
	assert(tz.dstStart.valid == 1);

	// These are from the link below, and I think some of the start/end dates are wrong. They all seem to be set
	// to the US standard, which I would not expect. They do parse correctly, however.
	// https://support.cyberdata.net/index.php?/Knowledgebase/Article/View/438/10/posix-timezone-strings

	// Iran
	tz.parse("IRDT-3:30IRST,M3.2.0/2:00:00,M11.1.0/2:00:00");
	assert(strcmp(tz.standardName.c_str(), "IRDT") == 0);
	assert(tz.standardHMS.hour == -3);
	assert(tz.standardHMS.minute == 30);
	assert(tz.standardHMS.second == 0);
	assert(tz.standardStart.month == 11);
	assert(tz.standardStart.week == 1);
	assert(tz.standardStart.dayOfWeek == 0);
	assert(tz.standardStart.hms.hour == 2);
	assert(tz.standardStart.hms.minute == 0);
	assert(tz.standardStart.hms.second == 0);
	assert(tz.standardStart.valid == 1);
	assert(strcmp(tz.dstName.c_str(), "IRST") == 0);
	assert(tz.dstStart.valid == 1);
	assert(tz.dstHMS.hour == -4);
	assert(tz.dstHMS.minute == 30);
	assert(tz.dstHMS.second == 0);
	assert(tz.dstStart.month == 3);
	assert(tz.dstStart.week == 2);
	assert(tz.dstStart.dayOfWeek == 0);
	assert(tz.dstStart.hms.hour == 2);
	assert(tz.dstStart.hms.minute == 0);
	assert(tz.dstStart.hms.second == 0);
	assert(tz.dstStart.valid == 1);

	// Central Australia
	tz.parse("ACST-8:30ACDT,M3.2.0/2:00:00,M11.1.0/2:00:00");
	assert(strcmp(tz.standardName.c_str(), "ACST") == 0);
	assert(tz.standardHMS.hour == -8);
	assert(tz.standardHMS.minute == 30);
	assert(tz.standardHMS.second == 0);
	assert(tz.standardStart.month == 11);
	assert(tz.standardStart.week == 1);
	assert(tz.standardStart.dayOfWeek == 0);
	assert(tz.standardStart.hms.hour == 2);
	assert(tz.standardStart.hms.minute == 0);
	assert(tz.standardStart.hms.second == 0);
	assert(tz.standardStart.valid == 1);
	assert(strcmp(tz.dstName.c_str(), "ACDT") == 0);
	assert(tz.dstStart.valid == 1);
	assert(tz.dstHMS.hour == -9);
	assert(tz.dstHMS.minute == 30);
	assert(tz.dstHMS.second == 0);
	assert(tz.dstStart.month == 3);
	assert(tz.dstStart.week == 2);
	assert(tz.dstStart.dayOfWeek == 0);
	assert(tz.dstStart.hms.hour == 2);
	assert(tz.dstStart.hms.minute == 0);
	assert(tz.dstStart.hms.second == 0);
	assert(tz.dstStart.valid == 1);

	// Chatham Islands
	tz.parse("CHAST-11:45CHADT,M3.2.0/2:00:00,M11.1.0/2:00:00");
	assert(strcmp(tz.standardName.c_str(), "CHAST") == 0);
	assert(tz.standardHMS.hour == -11);
	assert(tz.standardHMS.minute == 45);
	assert(tz.standardHMS.second == 0);
	assert(tz.standardStart.month == 11);
	assert(tz.standardStart.week == 1);
	assert(tz.standardStart.dayOfWeek == 0);
	assert(tz.standardStart.hms.hour == 2);
	assert(tz.standardStart.hms.minute == 0);
	assert(tz.standardStart.hms.second == 0);
	assert(tz.standardStart.valid == 1);
	assert(strcmp(tz.dstName.c_str(), "CHADT") == 0);
	assert(tz.dstStart.valid == 1);
	assert(tz.dstHMS.hour == -12);
	assert(tz.dstHMS.minute == 45);
	assert(tz.dstHMS.second == 0);
	assert(tz.dstStart.month == 3);
	assert(tz.dstStart.week == 2);
	assert(tz.dstStart.dayOfWeek == 0);
	assert(tz.dstStart.hms.hour == 2);
	assert(tz.dstStart.hms.minute == 0);
	assert(tz.dstStart.hms.second == 0);
	assert(tz.dstStart.valid == 1);

	// Some more formats from 
	// https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html

	// New York
	tz.parse("EST+5EDT,M3.2.0/2,M11.1.0/2");
	assert(strcmp(tz.standardName.c_str(), "EST") == 0);
	assert(tz.standardHMS.hour == 5);
	assert(tz.standardHMS.minute == 0);
	assert(tz.standardHMS.second == 0);
	assert(tz.standardStart.month == 11);
	assert(tz.standardStart.week == 1);
	assert(tz.standardStart.dayOfWeek == 0);
	assert(tz.standardStart.hms.hour == 2);
	assert(tz.standardStart.hms.minute == 0);
	assert(tz.standardStart.hms.second == 0);
	assert(tz.standardStart.valid == 1);
	assert(strcmp(tz.dstName.c_str(), "EDT") == 0);
	assert(tz.dstStart.valid == 1);
	assert(tz.dstHMS.hour == 4);
	assert(tz.dstHMS.minute == 0);
	assert(tz.dstHMS.second == 0);
	assert(tz.dstStart.month == 3);
	assert(tz.dstStart.week == 2);
	assert(tz.dstStart.dayOfWeek == 0);
	assert(tz.dstStart.hms.hour == 2);
	assert(tz.dstStart.hms.minute == 0);
	assert(tz.dstStart.hms.second == 0);
	assert(tz.dstStart.valid == 1);

	// Israel
	tz.parse("IST-2IDT,M3.4.4/26,M10.5.0");
	assert(strcmp(tz.standardName.c_str(), "IST") == 0);
	assert(tz.standardHMS.hour == -2);
	assert(tz.standardHMS.minute == 0);
	assert(tz.standardHMS.second == 0);
	assert(tz.standardStart.month == 10);
	assert(tz.standardStart.week == 5);
	assert(tz.standardStart.dayOfWeek == 0);
	assert(tz.standardStart.hms.hour == 0);
	assert(tz.standardStart.hms.minute == 0);
	assert(tz.standardStart.hms.second == 0);
	assert(tz.standardStart.valid == 1);
	assert(strcmp(tz.dstName.c_str(), "IDT") == 0);
	assert(tz.dstStart.valid == 1);
	assert(tz.dstHMS.hour == -3);
	assert(tz.dstHMS.minute == 0);
	assert(tz.dstHMS.second == 0);
	assert(tz.dstStart.month == 3);
	assert(tz.dstStart.week == 4);
	assert(tz.dstStart.dayOfWeek == 4);
	assert(tz.dstStart.hms.hour == 26);
	assert(tz.dstStart.hms.minute == 0);
	assert(tz.dstStart.hms.second == 0);
	assert(tz.dstStart.valid == 1);

	// Western Greenland
	tz.parse("WGT3WGST,M3.5.0/-2,M10.5.0/-1");
	assert(strcmp(tz.standardName.c_str(), "WGT") == 0);
	assert(tz.standardHMS.hour == 3);
	assert(tz.standardHMS.minute == 0);
	assert(tz.standardHMS.second == 0);
	assert(tz.standardStart.month == 10);
	assert(tz.standardStart.week == 5);
	assert(tz.standardStart.dayOfWeek == 0);
	assert(tz.standardStart.hms.hour == -1);
	assert(tz.standardStart.hms.minute == 0);
	assert(tz.standardStart.hms.second == 0);
	assert(tz.standardStart.valid == 1);
	assert(strcmp(tz.dstName.c_str(), "WGST") == 0);
	assert(tz.dstStart.valid == 1);
	assert(tz.dstHMS.hour == 2);
	assert(tz.dstHMS.minute == 0);
	assert(tz.dstHMS.second == 0);
	assert(tz.dstStart.month == 3);
	assert(tz.dstStart.week == 5);
	assert(tz.dstStart.dayOfWeek == 0);
	assert(tz.dstStart.hms.hour == -2);
	assert(tz.dstStart.hms.minute == 0);
	assert(tz.dstStart.hms.second == 0);
	assert(tz.dstStart.valid == 1);

}

void test1() {
	LocalTimePosixTimezone tzConfig("EST5EDT,M3.2.0/2:00:00,M11.1.0/2:00:00");
	// Also works with: EST+5EDT,M3.2.0/2,M11.1.0/2

	//
	// LocalTimeValue
	//
	LocalTimeValue v1;
	v1.fromString("2021-01-01 00:00:00");
	assertTime("", v1.toUTC(tzConfig), "tm_year=121 tm_mon=0 tm_mday=1 tm_hour=5 tm_min=0 tm_sec=0 tm_wday=5");

	v1.fromString("2021-03-14 01:59:59");
	assertTime("", v1.toUTC(tzConfig), "tm_year=121 tm_mon=2 tm_mday=14 tm_hour=6 tm_min=59 tm_sec=59 tm_wday=0");

	// This is a weird edge case as there isn't a 2:00 local time because it would have sprung forward to DST
	// This test is just to make sure the code doesn't crash or do something really weird
	v1.fromString("2021-03-14 02:00:00"); 
	assertTime("", v1.toUTC(tzConfig), "tm_year=121 tm_mon=2 tm_mday=14 tm_hour=6 tm_min=0 tm_sec=0 tm_wday=0");

	v1.fromString("2021-03-14 03:00:00"); 
	assertTime("", v1.toUTC(tzConfig), "tm_year=121 tm_mon=2 tm_mday=14 tm_hour=7 tm_min=0 tm_sec=0 tm_wday=0");

	// In DST, before falling back
	v1.fromString("2021-11-07 00:00:00"); 
	assertTime("", v1.toUTC(tzConfig), "tm_year=121 tm_mon=10 tm_mday=7 tm_hour=4 tm_min=0 tm_sec=0 tm_wday=0");

	// This could be two different UTC times as the time from 1:00 to 1:59:59 local time occurs twice because
	// of falling back. We somewhat arbitrarily pick the second time, which occurs in standard time (offset = 5:00)
	v1.fromString("2021-11-07 01:59:59"); 
	assertTime("", v1.toUTC(tzConfig), "tm_year=121 tm_mon=10 tm_mday=7 tm_hour=6 tm_min=59 tm_sec=59 tm_wday=0");

	// This is back on standard time
	v1.fromString("2021-11-07 02:00:00"); 
	assertTime("", v1.toUTC(tzConfig), "tm_year=121 tm_mon=10 tm_mday=7 tm_hour=7 tm_min=0 tm_sec=0 tm_wday=0");

	v1.fromString("2021-01-01 00:00:00");
	assertInt("", v1.ordinal(), 1);

	v1.fromString("2021-01-07 00:00:00");
	assertInt("", v1.ordinal(), 1);

	v1.fromString("2021-01-08 00:00:00");
	assertInt("", v1.ordinal(), 2);

	v1.fromString("2021-01-09 00:00:00");
	assertInt("", v1.ordinal(), 2);

	v1.fromString("2021-01-15 00:00:00");
	assertInt("", v1.ordinal(), 3);

	v1.fromString("2021-01-22 00:00:00");
	assertInt("", v1.ordinal(), 4);

	v1.fromString("2021-01-29 00:00:00");
	assertInt("", v1.ordinal(), 5);

	v1.fromString("2021-01-04 00:00:00");
	assertInt("", v1.ordinal(), 1);

	v1.fromString("2021-01-11 00:00:00");
	assertInt("", v1.ordinal(), 2);

	v1.fromString("2021-01-25 00:00:00");
	assertInt("", v1.ordinal(), 4);

	LocalTimeConvert conv;

	// Thu, 03 Jun 2021 18:10:52 GMT
	conv.withConfig(tzConfig).withTime(1622743852).convert();
	assertInt("position", (int)conv.position, (int)LocalTimeConvert::Position::IN_DST);

	// Note: tm_mon is zero-based so 2 = March
	// tm_hour is 7 because when entering DST it's standard time so 2 AM local time is 07:00 UTC
	assertStr("dstStart", LocalTime::getTmString(&conv.dstStartTimeInfo).c_str(), "tm_year=121 tm_mon=2 tm_mday=14 tm_hour=7 tm_min=0 tm_sec=0 tm_wday=0");
	assertInt("dstStart", (int)conv.dstStart, 1615705200);
	assertStr("standardStart", LocalTime::getTmString(&conv.standardStartTimeInfo).c_str(), "tm_year=121 tm_mon=10 tm_mday=7 tm_hour=6 tm_min=0 tm_sec=0 tm_wday=0");
	assertInt("standardStart", (int)conv.standardStart, 1636264800);

	// Wednesday, February 3, 2021 11:10:52 PM
	conv.withConfig(tzConfig).withTime(1612393852).convert();
	assertInt("position", (int)conv.position, (int)LocalTimeConvert::Position::BEFORE_DST);
	assertStr("dstStart", LocalTime::getTmString(&conv.dstStartTimeInfo).c_str(), "tm_year=121 tm_mon=2 tm_mday=14 tm_hour=7 tm_min=0 tm_sec=0 tm_wday=0");
	assertInt("dstStart", (int)conv.dstStart, 1615705200);

	assertStr("standardStart", LocalTime::getTmString(&conv.standardStartTimeInfo).c_str(), "tm_year=121 tm_mon=10 tm_mday=7 tm_hour=6 tm_min=0 tm_sec=0 tm_wday=0");
	assertInt("standardStart", (int)conv.standardStart, 1636264800);
	
	// Friday, December 3, 2021 11:10:52 PM
	conv.withConfig(tzConfig).withTime(1638573052).convert();
	assertInt("position", (int)conv.position, (int)LocalTimeConvert::Position::AFTER_DST);
	assertStr("dstStart", LocalTime::getTmString(&conv.dstStartTimeInfo).c_str(), "tm_year=121 tm_mon=2 tm_mday=14 tm_hour=7 tm_min=0 tm_sec=0 tm_wday=0");
	assertInt("dstStart", (int)conv.dstStart, 1615705200);
	assertStr("standardStart", LocalTime::getTmString(&conv.standardStartTimeInfo).c_str(), "tm_year=121 tm_mon=10 tm_mday=7 tm_hour=6 tm_min=0 tm_sec=0 tm_wday=0");
	assertInt("standardStart", (int)conv.standardStart, 1636264800);

	// Thu, 03 Jun 2021 18:10:52 GMT
	conv.withConfig(tzConfig).withTime(1622743852).convert();
	conv.atLocalTime(LocalTimeHMS("2:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=5 tm_mday=3 tm_hour=6 tm_min=0 tm_sec=0 tm_wday=4");	

	// Thu, 03 Jun 2021 18:10:52 GMT (14:10:52 EDT)
	conv.withConfig(tzConfig).withTime(1622743852).convert();
	conv.atLocalTime(LocalTimeHMS("14:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=5 tm_mday=3 tm_hour=18 tm_min=0 tm_sec=0 tm_wday=4");	

	// Date and time (GMT): Saturday, December 4, 2021 4:10:52 AM
	// Date and time (your time zone): Friday, December 3, 2021 11:10:52 PM GMT-05:00
	// This makes sure atDate is picking the date based on local date, not GMT date
	conv.withConfig(tzConfig).withTime(1638591052).convert();
	conv.atLocalTime(LocalTimeHMS("2:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=7 tm_min=0 tm_sec=0 tm_wday=5");	

	// 
	conv.withConfig(tzConfig).withTime(1638591052).convert();
	conv.atLocalTime(LocalTimeHMS("23:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=4 tm_min=0 tm_sec=0 tm_wday=6");	

	// Thu, 03 Jun 2021 18:10:52 GMT (14:10:52 EDT)
	// Rolls over to tomorrow
	conv.withConfig(tzConfig).withTime(1622743852).convert();
	conv.nextLocalTime(LocalTimeHMS("14:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=5 tm_mday=4 tm_hour=18 tm_min=0 tm_sec=0 tm_wday=5");	

	// Thu, 03 Jun 2021 18:10:52 GMT (14:10:52 EDT)
	// Does not change date
	conv.withConfig(tzConfig).withTime(1622743852).convert();
	conv.nextLocalTime(LocalTimeHMS("15:00")); 
	assertTime("", conv.time, "tm_year=121 tm_mon=5 tm_mday=3 tm_hour=19 tm_min=0 tm_sec=0 tm_wday=4");	


	// Wednesday, February 3, 2021 11:10:52 PM
	// This is not DST, so tm_hour == 7 UTC
	conv.withConfig(tzConfig).withTime(1612393852).convert();
	conv.atLocalTime(LocalTimeHMS("2:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=1 tm_mday=3 tm_hour=7 tm_min=0 tm_sec=0 tm_wday=3");	

	// 23:10:52 UTC = 18:10:52 local time (standard time offset 5:00)
	// setting the local time to 23:00 local time results in a UTC time of 04:00 the next day
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-03 23:10:52")).convert();
	conv.atLocalTime(LocalTimeHMS("23:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=4 tm_min=0 tm_sec=0 tm_wday=6");	

	// 23:10:52 UTC = 18:10:52 local time (standard time offset 5:00)
	// Since it's not yet 23:00 local time, nextLocalTime is the same as atLocalTime
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-03 23:10:52")).convert();
	conv.nextLocalTime(LocalTimeHMS("23:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=4 tm_min=0 tm_sec=0 tm_wday=6");	

	// 4:10:52 UTC = 23:10:52 local time the previous day
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 04:10:52")).convert();
	conv.nextLocalTime(LocalTimeHMS("23:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=5 tm_hour=4 tm_min=0 tm_sec=0 tm_wday=0");	

	// Thu, 03 Jun 2021 18:10:52 GMT (14:10:52 EDT)
	conv.withConfig(tzConfig).withTime(1622743852).convert();
	conv.nextDay();
	assertTime("", conv.time, "tm_year=121 tm_mon=5 tm_mday=4 tm_hour=18 tm_min=10 tm_sec=52 tm_wday=5");	

	// Thu, 03 Jun 2021 18:10:52 GMT (14:10:52 EDT)
	conv.withConfig(tzConfig).withTime(1622743852).convert();
	conv.nextDay(LocalTimeHMS("2:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=5 tm_mday=4 tm_hour=6 tm_min=0 tm_sec=0 tm_wday=5");	

	// Thu, 03 Jun 2021 18:10:52 GMT (14:10:52 EDT)
	conv.withConfig(tzConfig).withTime(1622743852).convert();
	conv.nextDay(LocalTimeHMS("15:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=5 tm_mday=4 tm_hour=19 tm_min=0 tm_sec=0 tm_wday=5");	

	// Thu, 03 Jun 2021 18:10:52 GMT (14:10:52 EDT)
	conv.withConfig(tzConfig).withTime(1622743852).convert();
	conv.nextDayOfWeek(5, LocalTimeHMS("15:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=5 tm_mday=4 tm_hour=19 tm_min=0 tm_sec=0 tm_wday=5");	

	// Thu, 03 Jun 2021 18:10:52 GMT (14:10:52 EDT)
	conv.withConfig(tzConfig).withTime(1622743852).convert();
	conv.nextDayOfWeek(6, LocalTimeHMS("15:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=5 tm_mday=5 tm_hour=19 tm_min=0 tm_sec=0 tm_wday=6");	

	// Thu, 03 Jun 2021 18:10:52 GMT (14:10:52 EDT)
	conv.withConfig(tzConfig).withTime(1622743852).convert();
	conv.nextDayOfWeek(0, LocalTimeHMS("15:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=5 tm_mday=6 tm_hour=19 tm_min=0 tm_sec=0 tm_wday=0");	

	// Thu, 03 Jun 2021 18:10:52 GMT (14:10:52 EDT)
	conv.withConfig(tzConfig).withTime(1622743852).convert();
	conv.nextDayOfWeek(1, LocalTimeHMS("15:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=5 tm_mday=7 tm_hour=19 tm_min=0 tm_sec=0 tm_wday=1");	

	// Thu, 03 Jun 2021 18:10:52 GMT (14:10:52 EDT)
	conv.withConfig(tzConfig).withTime(1622743852).convert();
	conv.nextDayOfWeek(1);
	assertTime("", conv.time, "tm_year=121 tm_mon=5 tm_mday=7 tm_hour=18 tm_min=10 tm_sec=52 tm_wday=1");	

	// Thu, 03 Jun 2021 18:10:52 GMT (14:10:52 EDT)
	conv.withConfig(tzConfig).withTime(1622743852).convert();
	conv.nextWeekday(LocalTimeHMS("15:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=5 tm_mday=4 tm_hour=19 tm_min=0 tm_sec=0 tm_wday=5");	

	conv.nextWeekday(LocalTimeHMS("15:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=5 tm_mday=7 tm_hour=19 tm_min=0 tm_sec=0 tm_wday=1");	

	// Thu, 03 Jun 2021 18:10:52 GMT (14:10:52 EDT)
	conv.withConfig(tzConfig).withTime(1622743852).convert();
	conv.nextWeekday();
	assertTime("", conv.time, "tm_year=121 tm_mon=5 tm_mday=4 tm_hour=18 tm_min=10 tm_sec=52 tm_wday=5");	

	// Thu, 03 Jun 2021 18:10:52 GMT (14:10:52 EDT)
	conv.withConfig(tzConfig).withTime(1622743852).convert();
	conv.nextWeekendDay(LocalTimeHMS("15:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=5 tm_mday=5 tm_hour=19 tm_min=0 tm_sec=0 tm_wday=6");	

	conv.nextWeekendDay(LocalTimeHMS("15:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=5 tm_mday=6 tm_hour=19 tm_min=0 tm_sec=0 tm_wday=0");	

	conv.nextWeekendDay(LocalTimeHMS("15:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=5 tm_mday=12 tm_hour=19 tm_min=0 tm_sec=0 tm_wday=6");	

	// Thu, 03 Jun 2021 18:10:52 GMT (14:10:52 EDT)
	conv.withConfig(tzConfig).withTime(1622743852).convert();
	conv.nextWeekendDay();
	assertTime("", conv.time, "tm_year=121 tm_mon=5 tm_mday=5 tm_hour=18 tm_min=10 tm_sec=52 tm_wday=6");	

	// 
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-06-04 10:10:52")).convert();
	conv.nextDayOfWeekOrdinal(6, 1);
	assertTime("", conv.time, "tm_year=121 tm_mon=5 tm_mday=5 tm_hour=10 tm_min=10 tm_sec=52 tm_wday=6");	

	// 
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-06-04 10:10:52")).convert();
	conv.nextDayOfWeekOrdinal(6, 1, LocalTimeHMS("07:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=5 tm_mday=5 tm_hour=11 tm_min=0 tm_sec=0 tm_wday=6");	

	// 
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-06-04 10:10:52")).convert();
	conv.nextDayOfWeekOrdinal(6, 2, LocalTimeHMS("07:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=5 tm_mday=12 tm_hour=11 tm_min=0 tm_sec=0 tm_wday=6");	

	// 
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-06-04 10:10:52")).convert();
	conv.nextDayOfWeekOrdinal(6, 3, LocalTimeHMS("07:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=5 tm_mday=19 tm_hour=11 tm_min=0 tm_sec=0 tm_wday=6");	

	// 
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-06-04 10:10:52")).convert();
	conv.nextDayOfWeekOrdinal(6, 4, LocalTimeHMS("07:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=5 tm_mday=26 tm_hour=11 tm_min=0 tm_sec=0 tm_wday=6");	

	// Special case: there is no 5th Saturday in this month but there is in July
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-06-04 10:10:52")).convert();
	conv.nextDayOfWeekOrdinal(6, 5, LocalTimeHMS("07:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=6 tm_mday=31 tm_hour=11 tm_min=0 tm_sec=0 tm_wday=6");	

	// There's never a 6th ordinal
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-06-04 10:10:52")).convert();
	assert(!conv.nextDayOfWeekOrdinal(6, 6, LocalTimeHMS("07:00")));

	// Does not roll forward to next month because it's not yet 5 AM local time
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 05:10:52")).convert();
	conv.nextDayOfMonth(4, LocalTimeHMS("05:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=10 tm_min=0 tm_sec=0 tm_wday=6");	

	// With no HMS, will advance to next month always
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 05:10:52")).convert();
	conv.nextDayOfMonth(4);
	assertTime("", conv.time, "tm_year=122 tm_mon=0 tm_mday=4 tm_hour=5 tm_min=10 tm_sec=52 tm_wday=2");	

	// Converting it again jumps forward a month
	conv.nextDayOfMonth(4, LocalTimeHMS("05:00"));
	assertTime("", conv.time, "tm_year=122 tm_mon=0 tm_mday=4 tm_hour=10 tm_min=0 tm_sec=0 tm_wday=2");	

	// Rolls forward to next month immediately
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 05:10:52")).convert();
	conv.nextDayOfMonth(4, LocalTimeHMS("00:00"));
	assertTime("", conv.time, "tm_year=122 tm_mon=0 tm_mday=4 tm_hour=5 tm_min=0 tm_sec=0 tm_wday=2");	

	// The local date is 12-03 due to timezone, make sure this works right
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 02:10:52")).convert();
	conv.nextDayOfMonth(4, LocalTimeHMS("05:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=10 tm_min=0 tm_sec=0 tm_wday=6");	

	// Even though we haven't reached this date, nextDateOfNextMonth always increments the month
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 05:10:52")).convert();
	conv.nextDayOfNextMonth(4, LocalTimeHMS("05:00"));
	assertTime("", conv.time, "tm_year=122 tm_mon=0 tm_mday=4 tm_hour=10 tm_min=0 tm_sec=0 tm_wday=2");	

	// A few more edge cases to be sure
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 10:10:52")).convert();
	conv.nextDayOfNextMonth(4, LocalTimeHMS("05:00"));
	assertTime("", conv.time, "tm_year=122 tm_mon=0 tm_mday=4 tm_hour=10 tm_min=0 tm_sec=0 tm_wday=2");	

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-31 23:23:59")).convert();
	conv.nextDayOfNextMonth(4, LocalTimeHMS("05:00"));
	assertTime("", conv.time, "tm_year=122 tm_mon=0 tm_mday=4 tm_hour=10 tm_min=0 tm_sec=0 tm_wday=2");	

	// This is still 2021-13-31 local time so next month is still January
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2022-01-01 00:00:00")).convert();
	conv.nextDayOfNextMonth(4, LocalTimeHMS("05:00"));
	assertTime("", conv.time, "tm_year=122 tm_mon=0 tm_mday=4 tm_hour=10 tm_min=0 tm_sec=0 tm_wday=2");	

	// This is still 2021-13-31 local time so next month is still January
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2022-01-01 04:59:59")).convert();
	conv.nextDayOfNextMonth(4, LocalTimeHMS("05:00"));
	assertTime("", conv.time, "tm_year=122 tm_mon=0 tm_mday=4 tm_hour=10 tm_min=0 tm_sec=0 tm_wday=2");	

	// Thu, 03 Jun 2021 18:10:52 GMT (14:10:52 EDT)
	conv.withConfig(tzConfig).withTime(1622743852).convert();
	conv.nextDayOfNextMonth(1, LocalTimeHMS("15:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=6 tm_mday=1 tm_hour=19 tm_min=0 tm_sec=0 tm_wday=4");	

	// Local time formatting default format

	// Standard time, before DST, offset = 0500
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-01-01 10:00:00")).convert();
	assertStr("", conv.timeStr().c_str(), "Fri Jan  1 05:00:00 2021");

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-01-01 23:45:56")).convert();
	assertStr("", conv.timeStr().c_str(), "Fri Jan  1 18:45:56 2021");

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-01-02 03:00:00")).convert();
	assertStr("", conv.timeStr().c_str(), "Fri Jan  1 22:00:00 2021");

	// DST, offset = 0400
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-04-01 10:00:00")).convert();
	assertStr("", conv.timeStr().c_str(), "Thu Apr  1 06:00:00 2021");

	// Local time strftime formats
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-01-01 10:00:00")).convert();
	assertStr("", conv.format("%Y-%m-%d %H:%M:%S").c_str(), "2021-01-01 05:00:00");

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-01-01 00:00:00")).convert();
	assertStr("", conv.format("%Y-%m-%d %H:%M:%S").c_str(), "2020-12-31 19:00:00");

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2022-01-01 01:00:00")).convert();
	assertStr("", conv.format("%Y-%m-%d %H:%M:%S").c_str(), "2021-12-31 20:00:00");

	// This format doesn't match the standard C lib for %z, but it's what Wiring returns. It should be -500 not -05:00
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-01-01 10:00:00")).convert();
	assertStr("", conv.format("%Y-%m-%d %H:%M:%S %z").c_str(), "2021-01-01 05:00:00 -05:00");

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-01-01 10:00:00")).convert();
	assertStr("", conv.format("%Y-%m-%d %H:%M:%S %Z").c_str(), "2021-01-01 05:00:00 EST");

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-04-01 10:00:00")).convert();
	assertStr("", conv.format("%Y-%m-%d %H:%M:%S %z").c_str(), "2021-04-01 06:00:00 -04:00");

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-04-01 10:00:00")).convert();
	assertStr("", conv.format("%Y-%m-%d %H:%M:%S %Z").c_str(), "2021-04-01 06:00:00 EDT");

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-04-01 04:00:00")).convert();
	assertStr("", conv.format("%I:%M %p").c_str(), "12:00 AM");

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-04-01 10:00:00")).convert();
	assertStr("", conv.format("%I:%M %p").c_str(), "06:00 AM");

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-04-01 16:00:00")).convert();
	assertStr("", conv.format("%I:%M %p").c_str(), "12:00 PM");

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-04-01 16:00:00")).convert();
	assertStr("", conv.format("%a %b %m").c_str(), "Thu Apr 04");

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-04-01 16:00:00")).convert();
	assertStr("", conv.format("%a %b %m").c_str(), "Thu Apr 04");

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-04-01 16:00:00")).convert();
	assertStr("", conv.format(TIME_FORMAT_DEFAULT).c_str(), "Thu Apr  1 12:00:00 2021");

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-04-01 16:00:00")).convert();
	assertStr("", conv.format(TIME_FORMAT_ISO8601_FULL).c_str(), "2021-04-01T12:00:00-04:00");


}

int main(int argc, char *argv[]) {
	testLocalTimeChange();
	testLocalTimePosixTimezone();
	test1();
	return 0;
}
