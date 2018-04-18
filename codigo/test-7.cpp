#include "ConcurrentHashMap.hpp"

#include <cassert>
#include <iostream>
#include <list>
#include <time.h>

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

int main(int argc, char const *argv[]) {

	if (argc != 4) {
		cout << "uso: " << argv[0] << "cant_archivos p_archivos p_maximos" << endl;
		exit(0);
	}

	unsigned int cant_archivos = stoi(argv[1]);
	unsigned int p_archivos = stoi(argv[2]);
	unsigned int p_maximos = stoi(argv[3]);

	list<string> archs;
	for (unsigned int i = 0; i < cant_archivos; ++i) archs.push_back("corpus-" + to_string(i));

	timespec start1 = gettime();
	item max1 = ConcurrentHashMap::maximum(p_archivos, p_maximos, archs);
	timespec end1 = gettime();

	timespec start2 = gettime();
	item max2 = ConcurrentHashMap::maximum2(p_archivos, p_maximos, archs);
	timespec end2 = gettime();

	assert(max1.second == max2.second);

	cout << timediff(start1, end1) << "," << timediff(start2, end2) << endl;

	return 0;
}