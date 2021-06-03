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

	tc.parse("M3.2.0");
	assert(!tc.valid);

	tc.parse("");
	assert(!tc.valid);

	tc.parse(0);
	assert(!tc.valid);

	tc.parse("3.2.0/2:00:00");
	assert(!tc.valid);

}

int main(int argc, char *argv[]) {
	testLocalTimeChange();
	return 0;
}
