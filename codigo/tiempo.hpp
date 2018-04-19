#include <time.h>
#include <iostream>

using namespace std;

timespec gettime() {
	timespec time;
	if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time) == -1) {
		cerr << "Error: unable to get time" << endl;
		exit(1);
	}
	return time;
}

long timediff(const timespec &start, const timespec &end) {
	long s = end.tv_sec - start.tv_sec;
	long ns = end.tv_nsec - start.tv_nsec;
	return s * 1e9 + ns;
}