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

JSONValue readTestDataJson(const char *filename) {
	JSONValue jsonObj;

	char *data = readTestData(filename);

	jsonObj = JSONValue::parse(data, strlen(data));

	return jsonObj;
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


#define assertTime2(msg, got, expected) _assertTime2(msg, got, expected, __LINE__)
void _assertTime2(const char *msg, time_t got, const char *expected, int line) {
	struct tm timeInfo;
	LocalTimeYMD ymd;
	LocalTimeHMS hms;

	LocalTime::timeToTm(got, &timeInfo);
	ymd.fromTimeInfo(&timeInfo);
	hms.fromTimeInfo(&timeInfo);
	
	String gotStr = ymd.toString() + String(" ") + hms.toString();
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

	// Newfoundland Time Zone (Canada) Newfoundland and southeastern Labrador
	tz.parse("NST3:30NDT,M3.2.0/2:00:00,M11.1.0/2:00:00");
	assert(strcmp(tz.standardName.c_str(), "NST") == 0);
	assert(tz.standardHMS.hour == 3);
	assert(tz.standardHMS.minute == 30);
	assert(tz.standardHMS.second == 0);
	assert(tz.standardStart.month == 11);
	assert(tz.standardStart.week == 1);
	assert(tz.standardStart.dayOfWeek == 0);
	assert(tz.standardStart.hms.hour == 2);
	assert(tz.standardStart.hms.minute == 0);
	assert(tz.standardStart.hms.second == 0);
	assert(tz.standardStart.valid == 1);
	assert(strcmp(tz.dstName.c_str(), "NDT") == 0);
	assert(tz.dstStart.valid == 1);
	assert(tz.dstHMS.hour == 2);
	assert(tz.dstHMS.minute == 30);
	assert(tz.dstHMS.second == 0);
	assert(tz.dstStart.month == 3);
	assert(tz.dstStart.week == 2);
	assert(tz.dstStart.dayOfWeek == 0);
	assert(tz.dstStart.hms.hour == 2);
	assert(tz.dstStart.hms.minute == 0);
	assert(tz.dstStart.hms.second == 0);
	assert(tz.dstStart.valid == 1);
}


void test1() {
	LocalTimePosixTimezone tzConfig("EST5EDT,M3.2.0/2:00:00,M11.1.0/2:00:00");
	// Also works with: EST+5EDT,M3.2.0/2,M11.1.0/2

	LocalTimePosixTimezone tzConfigNewfoundland("NST3:30NDT,M3.2.0/2:00:00,M11.1.0/2:00:00");

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

	// Weird special case on spring forward
	// Starting at 1:40 AM local time, on spring forward there is no 2:30 on this day
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-03-14 06:40:52")).convert();
	conv.nextLocalTime(LocalTimeHMS("02:30"));
	assertTime("", conv.time, "tm_year=121 tm_mon=2 tm_mday=15 tm_hour=6 tm_min=30 tm_sec=0 tm_wday=1");	

	// Fall back special case 12:40 AM local time -> 1:30 AM local time
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-11-07 05:40:52")).convert();
	conv.nextLocalTime(LocalTimeHMS("01:30"));
	assertTime("", conv.time, "tm_year=121 tm_mon=10 tm_mday=7 tm_hour=6 tm_min=30 tm_sec=0 tm_wday=0");	

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

	// 2021 time changes: March 14, November 7
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-02-01 06:40:52")).convert();
	conv.nextDayOrTimeChange(LocalTimeHMS("03:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=1 tm_mday=2 tm_hour=8 tm_min=0 tm_sec=0 tm_wday=2");	

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-03-14 01:40:52")).convert();
	conv.nextDayOrTimeChange(LocalTimeHMS("03:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=2 tm_mday=14 tm_hour=7 tm_min=0 tm_sec=0 tm_wday=0");	

	conv.nextDayOrTimeChange(LocalTimeHMS("03:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=2 tm_mday=15 tm_hour=7 tm_min=0 tm_sec=0 tm_wday=1");	

	conv.nextDayOrTimeChange(LocalTimeHMS("03:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=2 tm_mday=16 tm_hour=7 tm_min=0 tm_sec=0 tm_wday=2");	

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-11-05 06:40:52")).convert();
	conv.nextDayOrTimeChange(LocalTimeHMS("03:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=10 tm_mday=6 tm_hour=7 tm_min=0 tm_sec=0 tm_wday=6");	

	conv.nextDayOrTimeChange(LocalTimeHMS("03:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=10 tm_mday=7 tm_hour=6 tm_min=0 tm_sec=0 tm_wday=0");	

	conv.nextDayOrTimeChange(LocalTimeHMS("03:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=10 tm_mday=8 tm_hour=8 tm_min=0 tm_sec=0 tm_wday=1");	

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

	// 0 = last day of month
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 02:10:52")).convert();
	conv.nextDayOfMonth(0, LocalTimeHMS("05:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=31 tm_hour=10 tm_min=0 tm_sec=0 tm_wday=5");	

	// -1 = 2nd to last day of month
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 02:10:52")).convert();
	conv.nextDayOfMonth(-1, LocalTimeHMS("05:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=30 tm_hour=10 tm_min=0 tm_sec=0 tm_wday=4");	

	// Moves to next month if in the past
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-31 02:10:52")).convert();
	conv.nextDayOfMonth(-1, LocalTimeHMS("05:00"));
	assertTime("", conv.time, "tm_year=122 tm_mon=0 tm_mday=30 tm_hour=10 tm_min=0 tm_sec=0 tm_wday=0");	

	// 28 day month
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-02-04 02:10:52")).convert();
	conv.nextDayOfMonth(0, LocalTimeHMS("05:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=1 tm_mday=28 tm_hour=10 tm_min=0 tm_sec=0 tm_wday=0");	

	// Still a 28 day month, but make sure it's using local time for the calculation
	// so the UTC date is March 1
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-02-04 02:10:52")).convert();
	conv.nextDayOfMonth(0, LocalTimeHMS("23:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=2 tm_mday=1 tm_hour=4 tm_min=0 tm_sec=0 tm_wday=1");	


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

	// Last day of next month
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 05:10:52")).convert();
	conv.nextDayOfNextMonth(0, LocalTimeHMS("05:00"));
	assertTime("", conv.time, "tm_year=122 tm_mon=0 tm_mday=31 tm_hour=10 tm_min=0 tm_sec=0 tm_wday=1");	

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 05:10:52")).convert();
	conv.nextDayOfNextMonth(-1, LocalTimeHMS("05:00"));
	assertTime("", conv.time, "tm_year=122 tm_mon=0 tm_mday=30 tm_hour=10 tm_min=0 tm_sec=0 tm_wday=0");	

	// nextMinuteMultiple tests
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 10:10:52")).convert(); // 5 AM local time
	conv.nextMinuteMultiple(5);
	assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=10 tm_min=15 tm_sec=0 tm_wday=6");	

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 10:10:00")).convert(); // 5 AM local time
	conv.nextMinuteMultiple(5);
	assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=10 tm_min=15 tm_sec=0 tm_wday=6");	

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 10:14:49")).convert(); // 5 AM local time
	conv.nextMinuteMultiple(5);
	assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=10 tm_min=15 tm_sec=0 tm_wday=6");	

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-05 23:14:49")).convert(); //
	conv.nextMinuteMultiple(15);
	assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=5 tm_hour=23 tm_min=15 tm_sec=0 tm_wday=0");	

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-05 23:15:00")).convert(); //
	conv.nextMinuteMultiple(15);
	assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=5 tm_hour=23 tm_min=30 tm_sec=0 tm_wday=0");	

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-05 23:56:49")).convert(); //
	conv.nextMinuteMultiple(15);
	assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=6 tm_hour=0 tm_min=0 tm_sec=0 tm_wday=1");	

	// nextTime tests
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 10:10:52")).convert(); // 5 AM local time
	conv.nextTime(LocalTimeHMS("06:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=11 tm_min=0 tm_sec=0 tm_wday=6");	

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 10:10:52")).convert(); // 5:10:52 local time
	conv.nextTime(LocalTimeHMS("05:00"));
	assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=5 tm_hour=10 tm_min=0 tm_sec=0 tm_wday=0");	

	// nextTime spring forward time change
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2022-03-13 06:10:52")).convert(); // 1:10:52 AM EST
	conv.nextTime(LocalTimeHMS("03:00")); // 3 AM EDT = 7:00 UTC (-0400)
	assertTime("", conv.time, "tm_year=122 tm_mon=2 tm_mday=13 tm_hour=7 tm_min=0 tm_sec=0 tm_wday=0");	

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2022-03-13 19:10:52")).convert(); // 1:10:52 AM EST
	conv.nextTime(LocalTimeHMS("04:00")); // 4 AM EDT = 8:00 UTC (-0400)
	assertTime("", conv.time, "tm_year=122 tm_mon=2 tm_mday=14 tm_hour=8 tm_min=0 tm_sec=0 tm_wday=1");

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2022-03-12 19:10:52")).convert(); // 1:10:52 AM EST
	conv.nextTime(LocalTimeHMS("02:00")); // 2 AM EDT - weird case because 2:00 doesn't really exist because of spring forward
	assertTime("", conv.time, "tm_year=122 tm_mon=2 tm_mday=13 tm_hour=6 tm_min=0 tm_sec=0 tm_wday=0"); // 02:00:00 EDT

	// nextTimeList
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 10:10:52")).convert(); // 5 AM local time
	conv.nextTimeList({LocalTimeHMS("06:00")});
	assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=11 tm_min=0 tm_sec=0 tm_wday=6");	

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 10:10:52")).convert(); // 5 AM local time
	conv.nextTimeList({LocalTimeHMS("04:00"), LocalTimeHMS("08:00")});
	assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=13 tm_min=0 tm_sec=0 tm_wday=6");	

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 10:10:52")).convert(); // 5 AM local time
	conv.nextTimeList({LocalTimeHMS("03:00"), LocalTimeHMS("06:00")});
	assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=11 tm_min=0 tm_sec=0 tm_wday=6");	

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 03:10:52")).convert(); // 10 PM local time
	conv.nextTimeList({LocalTimeHMS("00:00"), LocalTimeHMS("12:00")});
	assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=5 tm_min=0 tm_sec=0 tm_wday=6");	

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 16:10:52")).convert();
	conv.nextTimeList({LocalTimeHMS("00:00"), LocalTimeHMS("12:00")});
	assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=17 tm_min=0 tm_sec=0 tm_wday=6");	

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 03:10:52")).convert(); 
	conv.nextTimeList({LocalTimeHMS("00"), LocalTimeHMS("12")});
	assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=5 tm_min=0 tm_sec=0 tm_wday=6");	

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 16:10:52")).convert(); 
	conv.nextTimeList({LocalTimeHMS("00"), LocalTimeHMS("12")});
	assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=17 tm_min=0 tm_sec=0 tm_wday=6");	

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 03:10:52")).convert(); // 10 PM local time
	conv.nextTimeList({LocalTimeHMS("00:05"), LocalTimeHMS("12:05")});
	assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=5 tm_min=5 tm_sec=0 tm_wday=6");	

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 16:10:52")).convert(); 
	conv.nextTimeList({LocalTimeHMS("00:15:30"), LocalTimeHMS("12:15:30")});
	assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=17 tm_min=15 tm_sec=30 tm_wday=6");	

	// inLocalTimeRange
	/*
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 10:10:52")).convert(); // 05:10:52 local time EST
	assert(conv.inLocalTimeRange(LocalTimeRange(LocalTimeHMS("05:00"), LocalTimeHMS("05:59:59"))));
	assert(!conv.inLocalTimeRange(LocalTimeRange(LocalTimeHMS("06:00"), LocalTimeHMS("06:59:59"))));

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 10:00:00")).convert(); // 05:00:00 local time EST
	assert(conv.inLocalTimeRange(LocalTimeRange(LocalTimeHMS("05:00"), LocalTimeHMS("05:59:59"))));

	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 11:00:00")).convert(); // 6:00:00 local time EST
	assert(!conv.inLocalTimeRange(LocalTimeRange(LocalTimeHMS("05:00"), LocalTimeHMS("05:59:59"))));

	// Crossing day boundary
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 03:10:52")).convert(); // 22:10:52 local time EST
	assert(conv.inLocalTimeRange(LocalTimeRange(LocalTimeHMS("22:00"), LocalTimeHMS("22:59:59"))));
	*/

	// Time range tests
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 16:10:52")).convert(); 
	{
		LocalTimeRange timeRange(LocalTimeHMS("00:15:30"), LocalTimeHMS("00:15:35"));
		assertInt("", timeRange.getTimeSpan(conv), 5);
	}
	{
		LocalTimeRange timeRange(LocalTimeHMS("00:15:30"), LocalTimeHMS("01:15:30"));
		assertInt("", timeRange.getTimeSpan(conv), 3600);
	}
	// Right before spring forward
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2022-03-13 19:10:52")).convert(); // 1:10:52 AM EST
	{
		// This crosses spring forward, so 2 hours on the clock is actually 1 hour
		LocalTimeRange timeRange(LocalTimeHMS("1:10:52"), LocalTimeHMS("3:10:52"));
		assertInt("", timeRange.getTimeSpan(conv), 3600);
	}
	{
		LocalTimeRange timeRange(LocalTimeHMS("3:10:52"), LocalTimeHMS("4:10:52"));
		assertInt("", timeRange.getTimeSpan(conv), 3600);
	}
	// Right before fall back
	conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-11-07 18:10:52")).convert(); // 12:10:52 AM EDT
	{
		// This crosses fall back, so 3 hours on the clock is actually 4 hours
		LocalTimeRange timeRange(LocalTimeHMS("00:10:52"), LocalTimeHMS("3:10:52"));
		assertInt("", timeRange.getTimeSpan(conv), 4 * 3600);
	}

	// LocalTimeYMD
	{
		LocalTimeYMD ymd;
		bool bResult;

		assertInt("", ymd.isEmpty(), true);
		
		bResult = ymd.parse("2022-03-12");
		assertInt("", bResult, true);
		assertInt("", ymd.getYear(), 2022);
		assertInt("", ymd.getMonth(), 3);
		assertInt("", ymd.getDay(), 12);
		assertInt("", ymd.getDayOfWeek(), 6);

		assertInt("", ymd.isEmpty(), false);

		ymd.setYear(2021);
		ymd.setMonth(12);
		ymd.setDay(10);
		assertInt("", ymd.getYear(), 2021);
		assertInt("", ymd.getMonth(), 12);
		assertInt("", ymd.getDay(), 10);
		assertInt("", ymd.getDayOfWeek(), 5);


		bResult = ymd.parse("2022-3-1");
		assertInt("", bResult, true);
		assertInt("", ymd.getYear(), 2022);
		assertInt("", ymd.getMonth(), 3);
		assertInt("", ymd.getDay(), 1);
		assertInt("", ymd.getDayOfWeek(), 2);

		LocalTimeYMD ymd2("2022-03-12");
		assertInt("", ymd2.getYear(), 2022);
		assertInt("", ymd2.getMonth(), 3);
		assertInt("", ymd2.getDay(), 12);
		assertInt("", ymd2.getDayOfWeek(), 6);

		LocalTimeYMD ymd3 = ymd2;
		assertInt("", ymd3.getYear(), 2022);
		assertInt("", ymd3.getMonth(), 3);
		assertInt("", ymd3.getDay(), 12);
		assertInt("", ymd3.getDayOfWeek(), 6);

		assertInt("", LocalTimeYMD("2022-03-12") == LocalTimeYMD("2022-03-12"), true);
		assertInt("", LocalTimeYMD("2022-03-12") == LocalTimeYMD("2022-03-13"), false);
		assertInt("", LocalTimeYMD("2022-03-12") <= LocalTimeYMD("2022-03-12"), true);
		assertInt("", LocalTimeYMD("2022-03-13") <= LocalTimeYMD("2022-03-12"), false);
		assertInt("", LocalTimeYMD("2022-03-12") >= LocalTimeYMD("2022-03-12"), true);
		assertInt("", LocalTimeYMD("2022-03-12") != LocalTimeYMD("2022-03-13"), true);
		assertInt("", LocalTimeYMD("2022-03-12") < LocalTimeYMD("2022-03-13"), true);
		assertInt("", LocalTimeYMD("2022-03-12") <= LocalTimeYMD("2022-03-13"), true);
		assertInt("", LocalTimeYMD("2022-03-14") > LocalTimeYMD("2022-03-13"), true);
		assertInt("", LocalTimeYMD("2022-03-14") >= LocalTimeYMD("2022-03-13"), true);
		assertInt("", LocalTimeYMD("2022-02-14") < LocalTimeYMD("2022-03-13"), true);
		assertInt("", LocalTimeYMD("2021-03-14") < LocalTimeYMD("2022-03-13"), true);

		bResult = ymd.parse("2022-03-11");
		ymd.addDay(1);
		assertInt("", bResult, true);
		assertInt("", ymd.getYear(), 2022);
		assertInt("", ymd.getMonth(), 3);
		assertInt("", ymd.getDay(), 12);
		assertInt("", ymd.getDayOfWeek(), 6);

		bResult = ymd.parse("2022-03-10");
		ymd.addDay(2);
		assertInt("", bResult, true);
		assertInt("", ymd.getYear(), 2022);
		assertInt("", ymd.getMonth(), 3);
		assertInt("", ymd.getDay(), 12);
		assertInt("", ymd.getDayOfWeek(), 6);

		bResult = ymd.parse("2022-02-28");
		ymd.addDay(12);
		assertInt("", bResult, true);
		assertInt("", ymd.getYear(), 2022);
		assertInt("", ymd.getMonth(), 3);
		assertInt("", ymd.getDay(), 12);
		assertInt("", ymd.getDayOfWeek(), 6);

		bResult = ymd.parse("2022-03-13");
		ymd.addDay(-1);
		assertInt("", bResult, true);
		assertInt("", ymd.getYear(), 2022);
		assertInt("", ymd.getMonth(), 3);
		assertInt("", ymd.getDay(), 12);
		assertInt("", ymd.getDayOfWeek(), 6);

		bResult = ymd.parse("2024-02-28");
		ymd.addDay(1);
		assertInt("", bResult, true);
		assertInt("", ymd.getYear(), 2024);
		assertInt("", ymd.getMonth(), 2);
		assertInt("", ymd.getDay(), 29);
		assertInt("", ymd.getDayOfWeek(), 4);

		ymd.addDay(1);
		assertInt("", bResult, true);
		assertInt("", ymd.getYear(), 2024);
		assertInt("", ymd.getMonth(), 3);
		assertInt("", ymd.getDay(), 1);
		assertInt("", ymd.getDayOfWeek(), 5);

		bResult = ymd.parse("2024-02-28");
		ymd.addDay(2);
		assertInt("", bResult, true);
		assertInt("", ymd.getYear(), 2024);
		assertInt("", ymd.getMonth(), 3);
		assertInt("", ymd.getDay(), 1);
		assertInt("", ymd.getDayOfWeek(), 5);


	}

	// LocalTimeRestrictedDate - Day Of Week
	{
		LocalTimeRestrictedDate t1;
		t1.withOnlyOnDays(LocalTimeDayOfWeek::MASK_WEEKDAY);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-06")), false);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-07")), true);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-08")), true);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-09")), true);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-10")), true);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-11")), true);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-12")), false);

		LocalTimeRestrictedDate t2;
		t2.withOnlyOnDays(LocalTimeDayOfWeek::MASK_WEEKEND);
		assertInt("", t2.isValid(LocalTimeYMD("2022-03-06")), true);
		assertInt("", t2.isValid(LocalTimeYMD("2022-03-07")), false);
		assertInt("", t2.isValid(LocalTimeYMD("2022-03-12")), true);

		LocalTimeRestrictedDate t3;
		t3.withOnlyOnDays(LocalTimeDayOfWeek::MASK_MONDAY);
		assertInt("", t3.isValid(LocalTimeYMD("2022-03-06")), false);
		assertInt("", t3.isValid(LocalTimeYMD("2022-03-07")), true);
		assertInt("", t3.isValid(LocalTimeYMD("2022-03-08")), false);
	}
	// LocalTimeRestrictedDate - Day Of Week JSON
	{
		LocalTimeRestrictedDate t1;
		t1.fromJson(readTestDataJson("testfiles/test09.json"));
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-06")), false);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-07")), true);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-08")), true);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-09")), true);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-10")), true);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-11")), true);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-12")), false);		
	}

	// LocalTimeRestrictedDate - Only on dates
	{
		LocalTimeRestrictedDate t2;
		t2.withOnlyOnDates({"2022-03-07", "2022-04-04"});
		assertInt("", t2.isValid(LocalTimeYMD("2022-03-06")), false);
		assertInt("", t2.isValid(LocalTimeYMD("2022-03-07")), true);
		assertInt("", t2.isValid(LocalTimeYMD("2022-03-08")), false);

		assertInt("", t2.isValid(LocalTimeYMD("2022-04-03")), false);
		assertInt("", t2.isValid(LocalTimeYMD("2022-04-04")), true);
		assertInt("", t2.isValid(LocalTimeYMD("2022-04-05")), false);

		LocalTimeRestrictedDate t3;
		t3.withOnlyOnDates({"2022-03-07"});
		assertInt("", t3.isValid(LocalTimeYMD("2022-03-06")), false);
		assertInt("", t3.isValid(LocalTimeYMD("2022-03-07")), true);
		assertInt("", t3.isValid(LocalTimeYMD("2022-03-08")), false);
	}
	// LocalTimeRestrictedDate - Only on dates - JSON
	{
		LocalTimeRestrictedDate t2;
		t2.fromJson(readTestDataJson("testfiles/test10.json"));
		assertInt("", t2.isValid(LocalTimeYMD("2022-03-06")), false);
		assertInt("", t2.isValid(LocalTimeYMD("2022-03-07")), true);
		assertInt("", t2.isValid(LocalTimeYMD("2022-03-08")), false);

		assertInt("", t2.isValid(LocalTimeYMD("2022-04-03")), false);
		assertInt("", t2.isValid(LocalTimeYMD("2022-04-04")), true);
		assertInt("", t2.isValid(LocalTimeYMD("2022-04-05")), false);
	}


	// LocalTimeRestrictions - Except Date
	{
		LocalTimeRestrictedDate t1;
		t1.withOnlyOnDays(LocalTimeDayOfWeek::MASK_WEEKDAY)
		  .withExceptDates({"2022-03-08"});

		assertInt("", t1.isValid(LocalTimeYMD("2022-03-06")), false);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-07")), true);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-08")), false);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-09")), true);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-10")), true);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-11")), true);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-12")), false);
	}
	// LocalTimeRestrictions - Except Date JSON
	{
		LocalTimeRestrictedDate t1;
		t1.fromJson(readTestDataJson("testfiles/test11.json"));

		assertInt("", t1.isValid(LocalTimeYMD("2022-03-06")), false);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-07")), true);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-08")), false);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-09")), true);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-10")), true);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-11")), true);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-12")), false);
	}
	// LocalTimeRestrictions - Only Date
	{
		LocalTimeRestrictedDate t1;
		t1.withOnlyOnDates({"2022-03-07", "2022-03-09"});

		assertInt("", t1.isValid(LocalTimeYMD("2022-03-06")), false);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-07")), true);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-08")), false);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-09")), true);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-10")), false);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-11")), false);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-12")), false);
	}
	// LocalTimeRestrictions - Only Date and only on days (adds additional day)
	{
		LocalTimeRestrictedDate t1;
		t1.withOnlyOnDays(LocalTimeDayOfWeek::MASK_TUESDAY);
		t1.withOnlyOnDates({"2022-03-07", "2022-03-09"});

		assertInt("", t1.isValid(LocalTimeYMD("2022-03-06")), false);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-07")), true);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-08")), true);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-09")), true);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-10")), false);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-11")), false);
		assertInt("", t1.isValid(LocalTimeYMD("2022-03-12")), false);

	}


	// Scheduling
	{
		LocalTimeSchedule schedule;
		schedule.withMinuteMultiple(5);



		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 16:10:30")).convert(); 
		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=16 tm_min=15 tm_sec=0 tm_wday=6");	

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 16:15:00")).convert();
		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=16 tm_min=20 tm_sec=0 tm_wday=6");	

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 16:15:01")).convert(); 
		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=16 tm_min=20 tm_sec=0 tm_wday=6");	

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 16:19:00")).convert();
		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=16 tm_min=20 tm_sec=0 tm_wday=6");	

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 23:58:00")).convert();
		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=5 tm_hour=0 tm_min=0 tm_sec=0 tm_wday=0");	
	}
	{
		// Every 15 minutes all day, at 05:00, 20:00, 35:00, 50:00
		LocalTimeSchedule schedule;
		schedule.withMinuteMultiple(15, LocalTimeRange(LocalTimeHMS("00:05:00"), LocalTimeHMS("23:59:59")));

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 16:10:00")).convert(); 
		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=16 tm_min=20 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=16 tm_min=35 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=16 tm_min=50 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=17 tm_min=5 tm_sec=0 tm_wday=6");	
	}

	{
		LocalTimeSchedule schedule;
		// Every 15 minutes between 9:00 AM and 5 PM local time (14:00 to 22:00 UTC)
		schedule.withMinuteMultiple(15, LocalTimeRange(LocalTimeHMS("09:00:00"), LocalTimeHMS("16:59:59")));

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 10:10:00")).convert(); // 05:10:52 local time
		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=14 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 13:10:52")).convert(); // 07:10:52 local time
		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=14 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=14 tm_min=15 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=14 tm_min=30 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=14 tm_min=45 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=15 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 21:55:00")).convert(); // 17:55:00 local time
		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=22 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=5 tm_hour=14 tm_min=0 tm_sec=0 tm_wday=0");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=5 tm_hour=14 tm_min=15 tm_sec=0 tm_wday=0");	

	}


	{
		LocalTimeSchedule schedule;
		// Every 15 minutes between 09:02:30 and 5 PM local time (14:02:30 to 22:00 UTC)
		schedule.withMinuteMultiple(15, LocalTimeRange(LocalTimeHMS("09:02:30"), LocalTimeHMS("16:59:59")));

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 10:10:00")).convert(); // 05:10:52 local time
		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=14 tm_min=2 tm_sec=30 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=14 tm_min=17 tm_sec=30 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=14 tm_min=32 tm_sec=30 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=14 tm_min=47 tm_sec=30 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=15 tm_min=2 tm_sec=30 tm_wday=6");	


	}
	{
		LocalTimeSchedule schedule;
		// Every 15 minutes between 9:00 AM and 5 PM local time (14:00 to 22:00 UTC)
		// Every hour otherwise
		schedule.withMinuteMultiple(15, LocalTimeRange(LocalTimeHMS("09:00:00"), LocalTimeHMS("16:59:59")));
		schedule.withMinuteMultiple(60);

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 10:10:52")).convert(); // 05:10:52 local time
		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=11 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=12 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=13 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=14 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=14 tm_min=15 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=14 tm_min=30 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=14 tm_min=45 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=15 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=15 tm_min=15 tm_sec=0 tm_wday=6");	

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 21:00:00")).convert(); // 04:00:00 local time
		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=21 tm_min=15 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=21 tm_min=30 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=21 tm_min=45 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=22 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=23 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=5 tm_hour=0 tm_min=0 tm_sec=0 tm_wday=0");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=5 tm_hour=1 tm_min=0 tm_sec=0 tm_wday=0");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=5 tm_hour=2 tm_min=0 tm_sec=0 tm_wday=0");	
	}

	{
		LocalTimeSchedule schedule;
		// Every 15 minutes between 9:00 AM and 5 PM local time (14:00 to 22:00 UTC) Monday - Friday
		// Every hour otherwise
		schedule.withMinuteMultiple(15, LocalTimeRange(LocalTimeHMS("09:00:00"), LocalTimeHMS("16:59:59"), LocalTimeRestrictedDate(LocalTimeDayOfWeek::MASK_WEEKDAY)));
		schedule.withMinuteMultiple(60);

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-03 10:10:52")).convert(); // 05:10:52 local time
		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=11 tm_min=0 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=12 tm_min=0 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=13 tm_min=0 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=14 tm_min=0 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=14 tm_min=15 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=14 tm_min=30 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=14 tm_min=45 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=15 tm_min=0 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=15 tm_min=15 tm_sec=0 tm_wday=5");	

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-03 21:40:52")).convert(); // 04:40:52 local time
		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=21 tm_min=45 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=22 tm_min=0 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=23 tm_min=0 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=0 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=1 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-06 13:40:52")).convert(); // 08:40:52 local time Monday
		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=6 tm_hour=14 tm_min=0 tm_sec=0 tm_wday=1");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=6 tm_hour=14 tm_min=15 tm_sec=0 tm_wday=1");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=6 tm_hour=14 tm_min=30 tm_sec=0 tm_wday=1");	

	}

	// JSON
	{
		LocalTimeSchedule schedule;
		// Every 15 minutes between 9:00 AM and 5 PM local time (14:00 to 22:00 UTC) Monday - Friday
		// Every hour otherwise
		schedule.fromJson(readTestDataJson("testfiles/test12.json"));

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-03 10:10:52")).convert(); // 05:10:52 local time
		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=11 tm_min=0 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=12 tm_min=0 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=13 tm_min=0 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=14 tm_min=0 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=14 tm_min=15 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=14 tm_min=30 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=14 tm_min=45 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=15 tm_min=0 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=15 tm_min=15 tm_sec=0 tm_wday=5");	

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-03 21:40:52")).convert(); // 04:40:52 local time
		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=21 tm_min=45 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=22 tm_min=0 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=23 tm_min=0 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=0 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=1 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-06 13:40:52")).convert(); // 08:40:52 local time Monday
		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=6 tm_hour=14 tm_min=0 tm_sec=0 tm_wday=1");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=6 tm_hour=14 tm_min=15 tm_sec=0 tm_wday=1");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=6 tm_hour=14 tm_min=30 tm_sec=0 tm_wday=1");	

	}
	{
		LocalTimeSchedule schedule;
		// Every 15 minutes between 9:00 AM and 5 PM local time (14:00 to 22:00 UTC) Monday - Friday
		//   Except 2021-12-06 (Monday), maybe it was a holiday
		// Every hour otherwise
		schedule.withMinuteMultiple(15, LocalTimeRange(LocalTimeHMS("09:00:00"), LocalTimeHMS("16:59:59"), 
			LocalTimeRestrictedDate(LocalTimeDayOfWeek::MASK_WEEKDAY, {}, {"2021-12-06"})));
		schedule.withMinuteMultiple(60);

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-03 10:10:52")).convert(); // 05:10:52 local time
		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=11 tm_min=0 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=12 tm_min=0 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=13 tm_min=0 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=14 tm_min=0 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=14 tm_min=15 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=14 tm_min=30 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=14 tm_min=45 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=15 tm_min=0 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=15 tm_min=15 tm_sec=0 tm_wday=5");	

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-03 21:40:52")).convert(); // 04:40:52 local time
		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=21 tm_min=45 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=22 tm_min=0 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=23 tm_min=0 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=0 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=1 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-06 13:40:52")).convert(); // 08:40:52 local time Monday
		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=6 tm_hour=14 tm_min=0 tm_sec=0 tm_wday=1");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=6 tm_hour=15 tm_min=0 tm_sec=0 tm_wday=1");

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=6 tm_hour=16 tm_min=0 tm_sec=0 tm_wday=1");

	}

	{
		LocalTimeSchedule schedule;
		// Every 15 minutes between 9:00 AM and 5 PM local time (14:00 to 22:00 UTC) Monday - Friday
		//   Except 2021-12-06 (Monday), maybe it was a holiday
		// Every hour otherwise
		schedule.fromJson(readTestDataJson("testfiles/test13.json"));

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-03 10:10:52")).convert(); // 05:10:52 local time
		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=11 tm_min=0 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=12 tm_min=0 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=13 tm_min=0 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=14 tm_min=0 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=14 tm_min=15 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=14 tm_min=30 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=14 tm_min=45 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=15 tm_min=0 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=15 tm_min=15 tm_sec=0 tm_wday=5");	

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-03 21:40:52")).convert(); // 04:40:52 local time
		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=21 tm_min=45 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=22 tm_min=0 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=23 tm_min=0 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=0 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=1 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-06 13:40:52")).convert(); // 08:40:52 local time Monday
		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=6 tm_hour=14 tm_min=0 tm_sec=0 tm_wday=1");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=6 tm_hour=15 tm_min=0 tm_sec=0 tm_wday=1");

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=6 tm_hour=16 tm_min=0 tm_sec=0 tm_wday=1");

	}

	{
		LocalTimeSchedule schedule;
		// Every 15 minutes between 9:00 AM and 5 PM local time (14:00 to 22:00 UTC) Monday - Friday
		//   Except 2021-12-06 (Monday), maybe it was a holiday
		// Every 4 hours otherwise (00:00, 04:00, ...)
		schedule.fromJson(readTestDataJson("testfiles/test16.json"));


		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-03 10:10:52")).convert(); // 05:10:52 EDT local time
		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=13 tm_min=0 tm_sec=0 tm_wday=5");	// 08:00:00 local time

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=14 tm_min=0 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=14 tm_min=15 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=14 tm_min=30 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=14 tm_min=45 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=15 tm_min=0 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=15 tm_min=15 tm_sec=0 tm_wday=5");	

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-03 21:40:52")).convert(); // 04:40:52 local time
		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=21 tm_min=45 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=22 tm_min=0 tm_sec=0 tm_wday=5");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=1 tm_min=0 tm_sec=0 tm_wday=6");	 // 20:00:00 local time

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=5 tm_min=0 tm_sec=0 tm_wday=6");	 // 00:00:00 local time

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-06 13:40:52")).convert(); // 08:40:52 local time Monday

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=6 tm_hour=17 tm_min=0 tm_sec=0 tm_wday=1");

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=6 tm_hour=21 tm_min=0 tm_sec=0 tm_wday=1");	
	}

	{
		LocalTimeSchedule schedule;
		// Using Newfoundland standard time -0330
		// Every 15 minutes between 9:00 AM and 5 PM local time (12:30 to 20:30 UTC) Monday - Friday
		//   Except 2021-12-06 (Monday), maybe it was a holiday
		// Every 4 hours otherwise (00:00, 04:00, ...)
		schedule.fromJson(readTestDataJson("testfiles/test16.json"));

		conv.withConfig(tzConfigNewfoundland).withTime(LocalTime::stringToTime("2021-12-03 10:15:00")).convert(); // 06:45:00 NST

		conv.atLocalTime(LocalTimeHMS("09:00:00"));
		assertTime2("", conv.time, "2021-12-03 12:30:00"); 

		conv.atLocalTime(LocalTimeHMS("17:00:00"));
		assertTime2("", conv.time, "2021-12-03 20:30:00"); 

		conv.atLocalTime(LocalTimeHMS("08:15:00"));

		conv.nextSchedule(schedule);
		assertTime2("", conv.time, "2021-12-03 12:30:00"); 

		conv.nextSchedule(schedule);
		assertTime2("", conv.time, "2021-12-03 12:45:00"); 

		conv.nextSchedule(schedule);
		assertTime2("", conv.time, "2021-12-03 13:00:00"); 

		conv.atLocalTime(LocalTimeHMS("16:50:00"));

		conv.nextSchedule(schedule);
		assertTime2("", conv.time, "2021-12-03 20:30:00"); 

		conv.nextSchedule(schedule);
		assertTime2("", conv.time, "2021-12-03 23:30:00"); // 20:00 NST

		conv.nextSchedule(schedule);
		assertTime2("", conv.time, "2021-12-04 03:30:00"); // 00:00 NST Saturday

		conv.nextSchedule(schedule);
		assertTime2("", conv.time, "2021-12-04 07:30:00"); // 04:00 NST

		conv.nextSchedule(schedule);
		assertTime2("", conv.time, "2021-12-04 11:30:00"); // 08:00 NST

		conv.nextSchedule(schedule);
		assertTime2("", conv.time, "2021-12-04 15:30:00"); // 12:00 NST 

	}


	{
		LocalTimeSchedule schedule;
		// Using UTC
		// Every 15 minutes between 09:00 and 17:00 UTC Monday - Friday
		//   Except 2021-12-06 (Monday), maybe it was a holiday
		// Every 4 hours otherwise (00:00, 04:00, ...)
		schedule.fromJson(readTestDataJson("testfiles/test16.json"));

		conv.withConfig("UTC").withTime(LocalTime::stringToTime("2021-12-03 08:15:00")).convert();

		conv.nextSchedule(schedule);
		assertTime2("", conv.time, "2021-12-03 09:00:00"); 

		conv.nextSchedule(schedule);
		assertTime2("", conv.time, "2021-12-03 09:15:00"); 

		conv.nextSchedule(schedule);
		assertTime2("", conv.time, "2021-12-03 09:30:00"); 

		conv.nextSchedule(schedule);
		assertTime2("", conv.time, "2021-12-03 09:45:00"); 

		conv.nextSchedule(schedule);
		assertTime2("", conv.time, "2021-12-03 10:00:00"); 

		conv.atLocalTime(LocalTimeHMS("16:50:00"));

		conv.nextSchedule(schedule);
		assertTime2("", conv.time, "2021-12-03 17:00:00"); 

		conv.nextSchedule(schedule);
		assertTime2("", conv.time, "2021-12-03 20:00:00"); 

		conv.nextSchedule(schedule);
		assertTime2("", conv.time, "2021-12-04 00:00:00"); // 00:00 Saturday

		conv.nextSchedule(schedule);
		assertTime2("", conv.time, "2021-12-04 04:00:00");

		conv.nextSchedule(schedule);
		assertTime2("", conv.time, "2021-12-04 08:00:00");

		conv.nextSchedule(schedule);
		assertTime2("", conv.time, "2021-12-04 12:00:00");

	}
	{
		// At specified times of the day (local time)
		LocalTimeSchedule schedule;
		schedule.withTimes({LocalTimeHMS("06:00"), LocalTimeHMS("18:30")}); // 11:00 UTC and 23:30 UTC

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 10:10:52")).convert(); // 05:10:52 local time
		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=11 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=23 tm_min=30 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=5 tm_hour=11 tm_min=0 tm_sec=0 tm_wday=0");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=5 tm_hour=23 tm_min=30 tm_sec=0 tm_wday=0");
	}
	{
		// At specified times of the day (local time) across spring forward time change
		LocalTimeSchedule schedule;
		schedule.withTimes({LocalTimeHMS("06:00"), LocalTimeHMS("18:30")}); // 11:00 UTC and 23:30 UTC

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2022-03-12 06:10:52")).convert(); // 1:10:52 AM EST
		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=122 tm_mon=2 tm_mday=12 tm_hour=11 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=122 tm_mon=2 tm_mday=12 tm_hour=23 tm_min=30 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=122 tm_mon=2 tm_mday=13 tm_hour=10 tm_min=0 tm_sec=0 tm_wday=0"); 

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=122 tm_mon=2 tm_mday=13 tm_hour=22 tm_min=30 tm_sec=0 tm_wday=0");

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=122 tm_mon=2 tm_mday=14 tm_hour=10 tm_min=0 tm_sec=0 tm_wday=1"); 

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=122 tm_mon=2 tm_mday=14 tm_hour=22 tm_min=30 tm_sec=0 tm_wday=1");
	}
		{
		// At specified times of the day (local time) across spring forward time change
		LocalTimeSchedule schedule;
		schedule.fromJson(readTestDataJson("testfiles/test14.json"));

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2022-03-12 06:10:52")).convert(); // 1:10:52 AM EST
		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=122 tm_mon=2 tm_mday=12 tm_hour=11 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=122 tm_mon=2 tm_mday=12 tm_hour=23 tm_min=30 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=122 tm_mon=2 tm_mday=13 tm_hour=10 tm_min=0 tm_sec=0 tm_wday=0"); 

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=122 tm_mon=2 tm_mday=13 tm_hour=22 tm_min=30 tm_sec=0 tm_wday=0");

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=122 tm_mon=2 tm_mday=14 tm_hour=10 tm_min=0 tm_sec=0 tm_wday=1"); 

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=122 tm_mon=2 tm_mday=14 tm_hour=22 tm_min=30 tm_sec=0 tm_wday=1");
	}
	{
		// At specified times of the day (local time) across fall back time change
		LocalTimeSchedule schedule;
		schedule.withTimes({LocalTimeHMS("06:00"), LocalTimeHMS("18:30")}); // 11:00 UTC and 23:30 UTC

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-11-07 05:10:52")).convert(); // 12:10:52 AM EDT

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=10 tm_mday=7 tm_hour=11 tm_min=0 tm_sec=0 tm_wday=0"); // 06:00:00 EST - 11:00:00 UTC

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=10 tm_mday=7 tm_hour=23 tm_min=30 tm_sec=0 tm_wday=0"); // 
	}



	{
		LocalTimeSchedule schedule;
		// Every 2 hours between 9:00 AM and 5 PM local time (14:00 to 22:00 UTC)
		schedule.withHourMultiple(2, LocalTimeRange(LocalTimeHMS("09:00:00"), LocalTimeHMS("17:00:00")));

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 10:10:52")).convert(); // 05:10:52 local time
		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=14 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=16 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=18 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=20 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=22 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=5 tm_hour=14 tm_min=0 tm_sec=0 tm_wday=0");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=5 tm_hour=16 tm_min=0 tm_sec=0 tm_wday=0");	
	}


	{
		LocalTimeSchedule schedule;
		// Every 2 hours between 9:00 AM and 5 PM local time (14:00 to 22:00 UTC)
		schedule.fromJson(readTestDataJson("testfiles/test15.json"));

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 10:10:52")).convert(); // 05:10:52 local time
		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=14 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=16 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=18 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=20 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=22 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=5 tm_hour=14 tm_min=0 tm_sec=0 tm_wday=0");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=5 tm_hour=16 tm_min=0 tm_sec=0 tm_wday=0");	
	}

	{
		LocalTimeSchedule schedule;
		// First Monday of the month at 9:00 AM local time
		schedule.fromJson(readTestDataJson("testfiles/test17.json"));

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2022-03-06 10:10:52")).convert(); // 05:10:52 local time
		bool bResult = conv.nextSchedule(schedule);
		assertInt("", bResult, true);
		assertTime2("", conv.time, "2022-03-07 14:00:00"); // Before DST switch 
	}

	{
		LocalTimeSchedule schedule;
		// Last day of the month at 5:00 PM local time
		schedule.fromJson(readTestDataJson("testfiles/test18.json"));

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2022-03-29 10:10:52")).convert(); // 05:10:52 local time
		bool bResult = conv.nextSchedule(schedule);
		assertInt("", bResult, true);
		assertTime2("", conv.time, "2022-03-31 21:00:00");
	}

	{
		LocalTimeSchedule schedule;
		// Every 15 minutes between 9:00 AM and 5 PM local time (14:00 to 22:00 UTC)
		// Every 2 hours from 1:00 AM otherwise
		schedule.withMinuteMultiple(15, LocalTimeRange(LocalTimeHMS("09:00:00"), LocalTimeHMS("16:59:59")));
		schedule.withHourMultiple(2, LocalTimeRange(LocalTimeHMS("01:00:00"), LocalTimeHMS("23:59:59")));

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 10:10:52")).convert(); // 05:10:52 local time

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=12 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=14 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=14 tm_min=15 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=14 tm_min=30 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=14 tm_min=45 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=15 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=15 tm_min=15 tm_sec=0 tm_wday=6");	

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 21:00:00")).convert(); // 04:00:00 local time
		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=21 tm_min=15 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=21 tm_min=30 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=21 tm_min=45 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=22 tm_min=0 tm_sec=0 tm_wday=6");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=5 tm_hour=0 tm_min=0 tm_sec=0 tm_wday=0");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=5 tm_hour=2 tm_min=0 tm_sec=0 tm_wday=0");	
	}

	{
		// Every 15 minutes over spring forward
		LocalTimeSchedule schedule;
		schedule.withMinuteMultiple(15);

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2022-03-13 19:10:52")).convert(); // 1:10:52 AM EST

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=122 tm_mon=2 tm_mday=13 tm_hour=19 tm_min=15 tm_sec=0 tm_wday=0");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=122 tm_mon=2 tm_mday=13 tm_hour=19 tm_min=30 tm_sec=0 tm_wday=0");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=122 tm_mon=2 tm_mday=13 tm_hour=19 tm_min=45 tm_sec=0 tm_wday=0");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=122 tm_mon=2 tm_mday=13 tm_hour=20 tm_min=0 tm_sec=0 tm_wday=0");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=122 tm_mon=2 tm_mday=13 tm_hour=20 tm_min=15 tm_sec=0 tm_wday=0");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=122 tm_mon=2 tm_mday=13 tm_hour=20 tm_min=30 tm_sec=0 tm_wday=0");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=122 tm_mon=2 tm_mday=13 tm_hour=20 tm_min=45 tm_sec=0 tm_wday=0");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=122 tm_mon=2 tm_mday=13 tm_hour=21 tm_min=0 tm_sec=0 tm_wday=0");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=122 tm_mon=2 tm_mday=13 tm_hour=21 tm_min=15 tm_sec=0 tm_wday=0");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=122 tm_mon=2 tm_mday=13 tm_hour=21 tm_min=30 tm_sec=0 tm_wday=0");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=122 tm_mon=2 tm_mday=13 tm_hour=21 tm_min=45 tm_sec=0 tm_wday=0");	

	}

	{
		// Every 15 minutes over fall back
		LocalTimeSchedule schedule;
		schedule.withMinuteMultiple(15);

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-11-07 04:10:52")).convert(); // 00:10:52 EDT

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=10 tm_mday=7 tm_hour=4 tm_min=15 tm_sec=0 tm_wday=0");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=10 tm_mday=7 tm_hour=4 tm_min=30 tm_sec=0 tm_wday=0");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=10 tm_mday=7 tm_hour=4 tm_min=45 tm_sec=0 tm_wday=0");	

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=10 tm_mday=7 tm_hour=5 tm_min=0 tm_sec=0 tm_wday=0");

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=10 tm_mday=7 tm_hour=5 tm_min=15 tm_sec=0 tm_wday=0");

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=10 tm_mday=7 tm_hour=5 tm_min=30 tm_sec=0 tm_wday=0");

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=10 tm_mday=7 tm_hour=5 tm_min=45 tm_sec=0 tm_wday=0");

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=10 tm_mday=7 tm_hour=6 tm_min=0 tm_sec=0 tm_wday=0");

		conv.nextSchedule(schedule);
		assertTime("", conv.time, "tm_year=121 tm_mon=10 tm_mday=7 tm_hour=6 tm_min=15 tm_sec=0 tm_wday=0");

	}

	// LocalTimeScheduleItem - low level tests
	// 
	{
		LocalTimeScheduleItem item;
		
		// Increment 15 minutes
		item.scheduleItemType = LocalTimeScheduleItem::ScheduleItemType::MINUTE_OF_HOUR;
		item.increment = 5;

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 16:10:52")).convert(); 

		item.getNextScheduledTime(conv);
		assertTime2("", conv.time, "2021-12-04 16:15:00");

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 16:15:00")).convert();
		item.getNextScheduledTime(conv);
		assertTime2("", conv.time, "2021-12-04 16:20:00");

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 16:15:01")).convert(); 
		item.getNextScheduledTime(conv);
		assertTime2("", conv.time, "2021-12-04 16:20:00");

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 16:19:59")).convert();
		item.getNextScheduledTime(conv);
		assertTime2("", conv.time, "2021-12-04 16:20:00");

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2021-12-04 23:58:52")).convert();
		item.getNextScheduledTime(conv);
		assertTime2("", conv.time, "2021-12-05 00:00:00");
	}

	{
		LocalTimeScheduleItem item;
		
		// Increment 15 minutes, starting at 00:05 past the hour
		item.scheduleItemType = LocalTimeScheduleItem::ScheduleItemType::MINUTE_OF_HOUR;
		item.increment = 15;
		item.timeRange = LocalTimeRange(LocalTimeHMS("00:05:00"), LocalTimeHMS("23:59:59"));

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2022-03-19 10:10:52")).convert(); 

		item.getNextScheduledTime(conv);
		assertTime2("", conv.time, "2022-03-19 10:20:00");

		item.getNextScheduledTime(conv);
		assertTime2("", conv.time, "2022-03-19 10:35:00");

		item.getNextScheduledTime(conv);
		assertTime2("", conv.time, "2022-03-19 10:50:00");

		item.getNextScheduledTime(conv);
		assertTime2("", conv.time, "2022-03-19 11:05:00");

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2022-03-19 23:50:52")).convert(); 

		item.getNextScheduledTime(conv);
		assertTime2("", conv.time, "2022-03-20 00:05:00");

	}

	{
		LocalTimeScheduleItem item;
		
		// Increment 3 hours
		item.scheduleItemType = LocalTimeScheduleItem::ScheduleItemType::HOUR_OF_DAY;
		item.increment = 3;

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2022-03-19 10:10:52")).convert(); // UTC; 06:10:52 local time

		item.getNextScheduledTime(conv);
		assertTime2("", conv.time, "2022-03-19 13:00:00"); // UTC; 9:00:00 local time

		item.getNextScheduledTime(conv);
		assertTime2("", conv.time, "2022-03-19 16:00:00"); // UTC; 12:00:00 local time

	}

	{
		LocalTimeScheduleItem item;
		
		// Increment 4 hours across EST->EDT switch
		item.scheduleItemType = LocalTimeScheduleItem::ScheduleItemType::HOUR_OF_DAY;
		item.increment = 3;

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2022-03-13 04:00:00")).convert(); // UTC; 23:00:00 local time

		item.getNextScheduledTime(conv);
		assertTime2("", conv.time, "2022-03-13 05:00:00"); // UTC; 00:00:00 EST local time

		item.getNextScheduledTime(conv);
		assertTime2("", conv.time, "2022-03-13 07:00:00"); // UTC; 3:00:00 EDT local time

		item.getNextScheduledTime(conv);
		assertTime2("", conv.time, "2022-03-13 10:00:00"); // UTC; 06:00:00 local time
	}

	{
		LocalTimeScheduleItem item;
		
		// Increment 4 hours across EST->EDT switch with 01:00:00 offset
		item.scheduleItemType = LocalTimeScheduleItem::ScheduleItemType::HOUR_OF_DAY;
		item.increment = 3;
		item.timeRange = LocalTimeRange(LocalTimeHMS("01:00:00"), LocalTimeHMS("23:59:59"));

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2022-03-13 04:00:00")).convert(); // UTC; 23:00:00 local time

		item.getNextScheduledTime(conv);
		assertTime2("", conv.time, "2022-03-13 06:00:00"); // UTC; 01:00:00 EDT local time

		item.getNextScheduledTime(conv);
		assertTime2("", conv.time, "2022-03-13 08:00:00"); // UTC; 04:00:00 EDT (this is right)

		item.getNextScheduledTime(conv);
		assertTime2("", conv.time, "2022-03-13 11:00:00"); // UTC; 07:00:00 EDT (this is right)
	}

	{
		LocalTimeScheduleItem item;
		
		// Increment 3 hours with offset of 1:15
		item.scheduleItemType = LocalTimeScheduleItem::ScheduleItemType::HOUR_OF_DAY;
		item.increment = 3;
		item.timeRange = LocalTimeRange(LocalTimeHMS("01:15:00"), LocalTimeHMS("23:59:59"));

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2022-03-19 10:10:52")).convert(); // UTC; 06:10:52 local time

		item.getNextScheduledTime(conv);
		assertTime2("", conv.time, "2022-03-19 11:15:00"); // UTC; 07:15:00 local time

		item.getNextScheduledTime(conv);
		assertTime2("", conv.time, "2022-03-19 14:15:00"); // UTC; 10:00:00 local time

		item.getNextScheduledTime(conv);
		assertTime2("", conv.time, "2022-03-19 17:15:00"); // UTC; 13:00:00 local time
	}

	/*
		April 2022       
	Su Mo Tu We Th Fr Sa  
					1  2  
	3  4  5  6  7  8  9  
	10 11 12 13 14 15 16  
	17 18 19 20 21 22 23  
	24 25 26 27 28 29 30 	

	      May 2022        
	Su Mo Tu We Th Fr Sa  
	1  2  3  4  5  6  7  
	8  9 10 11 12 13 14  
	15 16 17 18 19 20 21  
	22 23 24 25 26 27 28  
	29 30 31   
	*/
	{
		LocalTimeScheduleItem item;
		bool bResult;
		
		// First Friday of the month at 4:00 AM EDT 
		item.scheduleItemType = LocalTimeScheduleItem::ScheduleItemType::DAY_OF_WEEK_OF_MONTH;
		item.dayOfWeek = 5;
		item.increment = 1;
		item.timeRange = LocalTimeRange(LocalTimeHMS("04:00:00"));

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2022-04-01 04:00:00")).convert(); // UTC

		bResult = item.getNextScheduledTime(conv);
		assertInt("", bResult, true);
		assertTime2("", conv.time, "2022-04-01 08:00:00"); // UTC

		bResult = item.getNextScheduledTime(conv);
		assertInt("", bResult, false);
	}
	{
		LocalTimeScheduleItem item;
		bool bResult;
		
		// First Saturday of the month at 4:00 AM EDT 
		item.scheduleItemType = LocalTimeScheduleItem::ScheduleItemType::DAY_OF_WEEK_OF_MONTH;
		item.dayOfWeek = 6;
		item.increment = 1;
		item.timeRange = LocalTimeRange(LocalTimeHMS("04:00:00"));

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2022-04-01 04:00:00")).convert(); // UTC

		bResult = item.getNextScheduledTime(conv);
		assertInt("", bResult, true);
		assertTime2("", conv.time, "2022-04-02 08:00:00"); // UTC

		bResult = item.getNextScheduledTime(conv);
		assertInt("", bResult, false);
	}
	{
		LocalTimeScheduleItem item;
		bool bResult;
		
		// Second Sunday of the month at 4:00 AM EDT 
		item.scheduleItemType = LocalTimeScheduleItem::ScheduleItemType::DAY_OF_WEEK_OF_MONTH;
		item.dayOfWeek = 0;
		item.increment = 2;
		item.timeRange = LocalTimeRange(LocalTimeHMS("04:00:00"));

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2022-04-07 04:00:00")).convert(); // UTC

		bResult = item.getNextScheduledTime(conv);
		assertInt("", bResult, true);
		assertTime2("", conv.time, "2022-04-10 08:00:00"); // UTC

		bResult = item.getNextScheduledTime(conv);
		assertInt("", bResult, false);
	}
	{
		LocalTimeScheduleItem item;
		bool bResult;
		
		// Last Friday of the month at 11:59:59 PM EDT 
		item.scheduleItemType = LocalTimeScheduleItem::ScheduleItemType::DAY_OF_WEEK_OF_MONTH;
		item.dayOfWeek = 5;
		item.increment = -1;
		item.timeRange = LocalTimeRange(LocalTimeHMS("23:59:59"));

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2022-04-28 04:00:00")).convert(); // UTC

		bResult = item.getNextScheduledTime(conv);
		assertInt("", bResult, true);
		assertTime2("", conv.time, "2022-04-30 03:59:59"); // UTC

		bResult = item.getNextScheduledTime(conv);
		assertInt("", bResult, false);
	}
	{
		LocalTimeScheduleItem item;
		bool bResult;
		
		// First Sunday of the month at 4:00 AM EDT 
		item.scheduleItemType = LocalTimeScheduleItem::ScheduleItemType::DAY_OF_WEEK_OF_MONTH;
		item.dayOfWeek = 0;
		item.increment = 1;
		item.timeRange = LocalTimeRange(LocalTimeHMS("04:00:00"));

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2022-04-29 04:00:00")).convert(); // UTC

		bResult = item.getNextScheduledTime(conv);
		assertInt("", bResult, true);
		assertTime2("", conv.time, "2022-05-01 08:00:00"); // UTC

		bResult = item.getNextScheduledTime(conv);
		assertInt("", bResult, false);
	}
	{
		LocalTimeScheduleItem item;
		bool bResult;
		
		// 1st of the month at 4:00 AM EDT 
		item.scheduleItemType = LocalTimeScheduleItem::ScheduleItemType::DAY_OF_MONTH;
		item.increment = 1;
		item.timeRange = LocalTimeRange(LocalTimeHMS("04:00:00"));

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2022-04-29 04:00:00")).convert(); // UTC

		bResult = item.getNextScheduledTime(conv);
		assertInt("", bResult, true);
		assertTime2("", conv.time, "2022-05-01 08:00:00"); // UTC

		bResult = item.getNextScheduledTime(conv);
		assertInt("", bResult, false);
	}


	// LocalTimeRange JSON operations

	{
		
		const char *jsonStr = readTestData("testfiles/test08.json");
		JSONValue outerObj = JSONValue::parseCopy(jsonStr);
		free((void *)jsonStr);

		/*		
		{
			"t1": {},
			"t2": { "s": "09:00:00", "e": "16:59:59" },
			"t3": { "s": "10" }
		}
		*/

	    JSONObjectIterator iter(outerObj);
		while(iter.next()) {
			String key = (const char *) iter.name();
			if (key == "t1") {
				LocalTimeRange tr;

				tr.fromJson(iter.value());
				assertStr("", tr.hmsStart.toString(), "00:00:00");
				assertStr("", tr.hmsEnd.toString(), "23:59:59");
			}			
			else
			if (key == "t2") {
				LocalTimeRange tr;

				tr.fromJson(iter.value());
				assertStr("", tr.hmsStart.toString(), "09:00:00");
				assertStr("", tr.hmsEnd.toString(), "16:59:59");
			}			
			else
			if (key == "t3") {
				LocalTimeRange tr;

				tr.fromJson(iter.value());
				assertStr("", tr.hmsStart.toString(), "10:00:00");
				assertStr("", tr.hmsEnd.toString(), "23:59:59");
			}			
		}


	}


	{
		LocalTimeScheduleItem item;
		bool bResult;
		
		// On the 5th of the month at 4:00 AM local time
		item.scheduleItemType = LocalTimeScheduleItem::ScheduleItemType::DAY_OF_MONTH;
		item.increment = 5;
		item.timeRange = LocalTimeRange(LocalTimeHMS("04:00:00"));

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2022-03-05 07:10:52")).convert(); // UTC; 03:10:52 local time

		bResult = item.getNextScheduledTime(conv);
		assertInt("", bResult, true);
		assertTime2("", conv.time, "2022-03-05 09:00:00"); // UTC; 04:00:00 EST

		bResult = item.getNextScheduledTime(conv);
		assertInt("", bResult, false);

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2022-03-04 07:10:52")).convert(); // UTC; 03:10:52 local time
		bResult = item.getNextScheduledTime(conv);
		assertInt("", bResult, true);
		assertTime2("", conv.time, "2022-03-05 09:00:00"); // UTC; 04:00:00 EST

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2022-03-03 07:10:52")).convert(); // UTC; 03:10:52 local time
		bResult = item.getNextScheduledTime(conv);
		assertInt("", bResult, true);
		assertTime2("", conv.time, "2022-03-05 09:00:00"); // UTC; 04:00:00 EST

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2022-03-02 07:10:52")).convert(); // UTC; 03:10:52 local time
		bResult = item.getNextScheduledTime(conv);
		assertInt("", bResult, true);
		assertTime2("", conv.time, "2022-03-05 09:00:00"); // UTC; 04:00:00 EST

		conv.withConfig(tzConfig).withTime(LocalTime::stringToTime("2022-03-01 07:10:52")).convert(); // UTC; 03:10:52 local time
		bResult = item.getNextScheduledTime(conv);
		assertInt("", bResult, false);

	}

	// LocalTimeSchedule JSON operations


	// Make sure the closest minute multiple is used

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

	// dayOfWeekOfMonth
	{
		/*
			March 2022       
		Su Mo Tu We Th Fr Sa  
		      1  2  3  4  5  
		6  7  8  9 10 11 12  
		13 14 15 16 17 18 19  
		20 21 22 23 24 25 26  
		27 28 29 30 31 
		*/	
		assertInt("", LocalTime::dayOfWeekOfMonth(2022, 3, 2, 1), 1);
		assertInt("", LocalTime::dayOfWeekOfMonth(2022, 3, 3, 1), 2);
		assertInt("", LocalTime::dayOfWeekOfMonth(2022, 3, 4, 1), 3);
		assertInt("", LocalTime::dayOfWeekOfMonth(2022, 3, 5, 1), 4);
		assertInt("", LocalTime::dayOfWeekOfMonth(2022, 3, 6, 1), 5);
		assertInt("", LocalTime::dayOfWeekOfMonth(2022, 3, 0, 1), 6);
		assertInt("", LocalTime::dayOfWeekOfMonth(2022, 3, 1, 1), 7);
		assertInt("", LocalTime::dayOfWeekOfMonth(2022, 3, 2, 2), 8);
		assertInt("", LocalTime::dayOfWeekOfMonth(2022, 3, 3, 2), 9);
		assertInt("", LocalTime::dayOfWeekOfMonth(2022, 3, 2, 3), 15);
		assertInt("", LocalTime::dayOfWeekOfMonth(2022, 3, 2, 4), 22);
		assertInt("", LocalTime::dayOfWeekOfMonth(2022, 3, 2, 5), 29);
		assertInt("", LocalTime::dayOfWeekOfMonth(2022, 3, 2, 6), 0);


		assertInt("", LocalTime::dayOfWeekOfMonth(2022, 3, 0, -1), 27);
		assertInt("", LocalTime::dayOfWeekOfMonth(2022, 3, 0, -2), 20);
		assertInt("", LocalTime::dayOfWeekOfMonth(2022, 3, 0, -3), 13);
		assertInt("", LocalTime::dayOfWeekOfMonth(2022, 3, 0, -4), 6);
		assertInt("", LocalTime::dayOfWeekOfMonth(2022, 3, 0, -5), 0);

		assertInt("", LocalTime::dayOfWeekOfMonth(2022, 3, 4, -1), 31);
		assertInt("", LocalTime::dayOfWeekOfMonth(2022, 3, 3, -1), 30);
		assertInt("", LocalTime::dayOfWeekOfMonth(2022, 3, 2, -1), 29);
		assertInt("", LocalTime::dayOfWeekOfMonth(2022, 3, 1, -1), 28);
		assertInt("", LocalTime::dayOfWeekOfMonth(2022, 3, 0, -1), 27);
		assertInt("", LocalTime::dayOfWeekOfMonth(2022, 3, 6, -1), 26);
		assertInt("", LocalTime::dayOfWeekOfMonth(2022, 3, 5, -1), 25);
		assertInt("", LocalTime::dayOfWeekOfMonth(2022, 3, 4, -2), 24);

		/*
		February 2024      
		Su Mo Tu We Th Fr Sa  
					1  2  3  
		4  5  6  7  8  9 10  
		11 12 13 14 15 16 17  
		18 19 20 21 22 23 24  
		25 26 27 28 29  
		*/
		assertInt("", LocalTime::dayOfWeekOfMonth(2024, 2, 0, 1), 4);
		assertInt("", LocalTime::dayOfWeekOfMonth(2024, 2, 0, 2), 11);
		assertInt("", LocalTime::dayOfWeekOfMonth(2024, 2, 0, 3), 18);
		assertInt("", LocalTime::dayOfWeekOfMonth(2024, 2, 0, 4), 25);
		assertInt("", LocalTime::dayOfWeekOfMonth(2024, 2, 0, 5), 0);

		assertInt("", LocalTime::dayOfWeekOfMonth(2024, 2, 4, 1), 1);
		assertInt("", LocalTime::dayOfWeekOfMonth(2024, 2, 4, 2), 8);
		assertInt("", LocalTime::dayOfWeekOfMonth(2024, 2, 4, 3), 15);
		assertInt("", LocalTime::dayOfWeekOfMonth(2024, 2, 4, 4), 22);
		assertInt("", LocalTime::dayOfWeekOfMonth(2024, 2, 4, 5), 29);
		assertInt("", LocalTime::dayOfWeekOfMonth(2024, 2, 4, 6), 0);

		assertInt("", LocalTime::dayOfWeekOfMonth(2024, 2, 4, -1), 29);
		assertInt("", LocalTime::dayOfWeekOfMonth(2024, 2, 4, -2), 22);

	}

}


int monthStrToMonth(const char *token) {
	if (strcmp(token, "Jan") == 0) {
		return 1;
	}
	else
	if (strcmp(token, "Feb") == 0) {
		return 2;
	}
	else
	if (strcmp(token, "Mar") == 0) {
		return 3;
	}
	else
	if (strcmp(token, "Apr") == 0) {
		return 4;
	}
	else
	if (strcmp(token, "May") == 0) {
		return 5;
	}
	else
	if (strcmp(token, "Jun") == 0) {
		return 6;
	}
	else
	if (strcmp(token, "Jul") == 0) {
		return 7;
	}
	else
	if (strcmp(token, "Aug") == 0) {
		return 8;
	}
	else
	if (strcmp(token, "Sep") == 0) {
		return 9;
	}
	else
	if (strcmp(token, "Oct") == 0) {
		return 10;
	}
	else
	if (strcmp(token, "Nov") == 0) {
		return 11;
	}
	else
	if (strcmp(token, "Dec") == 0) {
		return 12;
	}

	return 0;
}

void printConv(LocalTimeConvert conv) {
	printf("position=%d\n", conv.position);
	printf("time utc=%s\n", LocalTime::timeToString(conv.time).c_str());
	printf("dstStartTimeInfo=%s\n", LocalTime::getTmString(&conv.dstStartTimeInfo).c_str());
	printf("standardStartTimeInfo=%s\n", LocalTime::getTmString(&conv.standardStartTimeInfo).c_str());

	/*
	Position position = Position::NO_DST;
    LocalTimePosixTimezone config;
    time_t time;
    LocalTimeValue localTimeValue;
    time_t dstStart;
    struct tm dstStartTimeInfo;
    time_t standardStart;
    struct tm standardStartTimeInfo;
	*/
}

void testFile(const char *configStr, const char *path) {
	LocalTimePosixTimezone tzConfig(configStr);

	char *testData = readTestData(path);
	
	int line = 1;
	char *token, *save = testData;
    while((token = strtok_r(save, "\n", &save)) != 0) {
		// Output from tzdump -v America/New_York
	 	// America/New_York  Sun Nov  3 06:00:00 2030 UTC = Sun Nov  3 01:00:00 2030 EST isdst=0
		// 0                 1   2    3 4        5    6   7 8   9    10 11      12   13  14

		char entry[64];
		snprintf(entry, sizeof(entry), "%s line %d", path, line);

		int utcMonth; // 2 
		int utcDayOfMonth; // 3
		LocalTimeHMS utcHMS; // 4 HH:MM:SSS
		int utcYear; // 5

		int localMonth; // 9 
		int localDayOfMonth; // 10
		LocalTimeHMS localHMS;  // 11 HH:MM:SS
		int localYear; // 12
		char localZone[10]; // 13

		bool dstFlag; // 14 ends with 1

		int index2 = 0;
		char *token2, *save2 = token;
		while((token2 = strtok_r(save2, " ", &save2)) != 0) {
			// printf("token2: '%s'\n", token2);
			switch(index2) {
			case 2:
				utcMonth = monthStrToMonth(token2);
				break;
			case 3:
				utcDayOfMonth = atoi(token2);
				break;
			case 4:
				utcHMS.parse(token2);
				break;
			case 5:
				utcYear = atoi(token2);
				break;
			case 9:
				localMonth = monthStrToMonth(token2);
				break;
			case 10:
				localDayOfMonth = atoi(token2);
				break;
			case 11:
				localHMS.parse(token2);
				break;
			case 12:
				localYear = atoi(token2);
				break;
			case 13:
				strcpy(localZone, token2);
				break;
			case 14:
				dstFlag = token2[strlen(token2) - 1] == '1';
				break;
			}

			index2++;
		}

		// printf("utcHMS=%s\n", utcHMS.toString().c_str());

		struct tm timeInfo;
		timeInfo.tm_year = utcYear - 1900;
		timeInfo.tm_mon = utcMonth - 1;
		timeInfo.tm_mday = utcDayOfMonth;
		timeInfo.tm_hour = utcHMS.hour;
		timeInfo.tm_min = utcHMS.minute;
		timeInfo.tm_sec = utcHMS.second;
		
		// printf("timeInfo=%s\n", LocalTime::getTmString(&timeInfo).c_str());


		LocalTimeConvert conv;
		conv.withConfig(tzConfig).withTime(timegm(&timeInfo)).convert();

		if (conv.localTimeValue.year() == localYear &&
			conv.localTimeValue.month() == localMonth &&		
			conv.localTimeValue.day() == localDayOfMonth &&
			conv.localTimeValue.hour() == localHMS.hour &&	
			conv.localTimeValue.minute() == localHMS.minute &&
			conv.localTimeValue.second() == localHMS.second &&
			conv.isDST() == dstFlag) {
			// Good

		}
		else {
			printf("got      year=%04d month=%02d day=%02d hour=%02d minute=%02d second=%02d dst=%d\n", 
				conv.localTimeValue.year(), conv.localTimeValue.month(), conv.localTimeValue.day(),
				conv.localTimeValue.hour(), conv.localTimeValue.minute(), conv.localTimeValue.second(),
				conv.isDST());

			printf("expected year=%04d month=%02d day=%02d hour=%02d minute=%02d second=%02d dst=%d\n", 
				localYear, localMonth, localDayOfMonth,
				localHMS.hour, localHMS.minute, localHMS.second, 
				dstFlag);

			// printf("standardStart=%d dstStart=%d time=%d\n", (int)conv.standardStart, (int)conv.dstStart, (int)conv.time);
			printf("standardStart=%s\n", Time.format(conv.standardStart, TIME_FORMAT_ISO8601_FULL).c_str());
			printf("dstStart=%s\n", Time.format(conv.dstStart, TIME_FORMAT_ISO8601_FULL).c_str());
			printf("time=%s\n", Time.format(conv.time, TIME_FORMAT_ISO8601_FULL).c_str());
			printf("line=%d path=%s\n", line, path);
			assert(false);
		}


		//printConv(conv);
		line++;
	}


	free(testData);
}

void testFiles() {
	
	// EST=UTC-5 EDT=UTC-4 
	testFile("EST5EDT,M3.2.0/02:00:00,M11.1.0/02:00:00", "testfiles/test01.txt"); // New York
	testFile("CST6CDT,M3.2.0/2:00:00,M11.1.0/2:00:00", "testfiles/test02.txt"); // Chicago
	testFile("MST7MDT,M3.2.0/2:00:00,M11.1.0/2:00:00", "testfiles/test03.txt"); // Denver
	testFile("PST8PDT,M3.2.0/2:00:00,M11.1.0/2:00:00", "testfiles/test04.txt"); // Los Angeles

	testFile("BST0GMT,M3.5.0/1:00:00,M10.5.0/2:00:00", "testfiles/test05.txt"); // London

	// AEST=UTC+10, AEDT=UTC+11
	testFile("AEST-10AEDT,M10.1.0/02:00:00,M4.1.0/03:00:00", "testfiles/test06.txt"); // Sydney Australia

	// ls /usr/share/zoneinfo
	// zdump -v Australia/Adelaide
	// ACST=UTC+9:30 ACDT=UTC+10:30
	testFile("ACST-9:30ACDT,M10.1.0/02:00:00,M4.1.0/03:00:00", "testfiles/test07.txt"); // Adelaide Australia


}

void test2() {
    LocalTime::instance().withConfig(LocalTimePosixTimezone("EST5EDT,M3.2.0/2:00:00,M11.1.0/2:00:00"));

	LocalTimeConvert conv;
	conv.withTime(LocalTime::stringToTime("2021-12-03 05:10:52")).convert();
	conv.nextLocalTime("06:00:00");

	assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=3 tm_hour=11 tm_min=0 tm_sec=0 tm_wday=5");	


	conv.nextDayOfWeek(6, LocalTimeHMS("3:00")); // every Saturday at 3:00 AM local time
	assertTime("", conv.time, "tm_year=121 tm_mon=11 tm_mday=4 tm_hour=8 tm_min=0 tm_sec=0 tm_wday=6");	

}

void test3() {
	// This is not actually the timezone specifier for Adelaide, Australia, just checking that 30 minute offsets work
    LocalTime::instance().withConfig(LocalTimePosixTimezone("ACST-9:30"));

	LocalTimeConvert conv;
	conv.withTime(LocalTime::stringToTime("2021-07-08 09:22:00")).convert();

	assertStr("", conv.format("%Y-%m-%d %H:%M:%S").c_str(), "2021-07-08 18:52:00");
}

int main(int argc, char *argv[]) {
	testLocalTimeChange();
	testLocalTimePosixTimezone();
	test1();
	test3();
	testFiles();

	// test2 sets the global timezone configuration
	test2();

	return 0;
}
