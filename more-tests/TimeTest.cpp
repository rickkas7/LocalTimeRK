#include "Particle.h"
#include "LocalTimeRK.h"

#include <time.h>

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

#define assertInt(msg, expected, got) _assertInt(msg, expected, got, __LINE__)
void _assertInt(const char *msg, int expected, int got, int line) {
	if (expected != got) {
		printf("assertion failed %s line %d\n", msg, line);
		printf("expected: %d\n", expected);
		printf("     got: %d\n", got);
		assert(false);
	}
}

#define assertStr(msg, expected, got) _assertStr(msg, expected, got, __LINE__)
void _assertStr(const char *msg, const char *expected, const char *got, int line) {
	if (strcmp(expected, got) != 0) {
		printf("assertion failed %s line %d\n", msg, line);
		printf("expected: %s\n", expected);
		printf("     got: %s\n", got);
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
	
	// Wednesday, December 3, 2021 11:10:52 PM
	conv.withConfig(tzConfig).withTime(1638573052).convert();
	assertInt("position", (int)conv.position, (int)LocalTimeConvert::Position::AFTER_DST);
	assertStr("dstStart", LocalTime::getTmString(&conv.dstStartTimeInfo).c_str(), "tm_year=121 tm_mon=2 tm_mday=14 tm_hour=7 tm_min=0 tm_sec=0 tm_wday=0");
	assertInt("dstStart", (int)conv.dstStart, 1615705200);
	assertStr("standardStart", LocalTime::getTmString(&conv.standardStartTimeInfo).c_str(), "tm_year=121 tm_mon=10 tm_mday=7 tm_hour=6 tm_min=0 tm_sec=0 tm_wday=0");
	assertInt("standardStart", (int)conv.standardStart, 1636264800);

}

int main(int argc, char *argv[]) {
	testLocalTimeChange();
	testLocalTimePosixTimezone();
	test1();
	return 0;
}
