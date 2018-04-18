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

	list<string> archs = { "corpus-0", "corpus-1", "corpus-2", "corpus-3", "corpus-4" };

	if (argc != 3) {
		cout << "uso: " << argv[0] << " p_archivos p_maximos" << endl;
		exit(0);
	}

	unsigned int p_archivos = stoi(argv[1]);
	unsigned int p_maximos = stoi(argv[2]);

	timespec start1 = gettime();
	ConcurrentHashMap::maximum(p_archivos, p_maximos, archs);
	cout << timediff(start1, gettime()) << ",";

	timespec start2 = gettime();
	ConcurrentHashMap::maximum2(p_archivos, p_maximos, archs);
	cout << timediff(start2, gettime()) << endl;

	return 0;
}