#include "ConcurrentHashMap.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
#include <list>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

#define NUM_THREADS 10
#define MAX_NUM_THREADS 10

struct thread_args_s {
	ConcurrentHashMap *map;
	ConcurrentHashMap *maps_i;
	list<string> *archs;
	list<string> *palabras;
};

void *thread_function(void *thread_args) {
	thread_args_s *args = (thread_args_s*) thread_args;

	*args->maps_i = ConcurrentHashMap::count_words(rand() % MAX_NUM_THREADS + 1, *args->archs);
	auto max = args->maps_i->maximum(1);

	for (auto it = args->palabras->begin(); it != args->palabras->end(); ++it) {
		args->map->addAndInc(*it); assert(args->map->member(*it));
		args->maps_i->addAndInc(*it); assert(args->maps_i->member(*it));
		args->map->maximum(rand() % MAX_NUM_THREADS);
		args->maps_i->maximum(rand() % MAX_NUM_THREADS);
	}

	return nullptr;
}

int main(int argc, char const *argv[]) {

	list<string> archs = { "corpus-0", "corpus-1", "corpus-2", "corpus-3", "corpus-4" };
	list<string> palabras;

	// Leo palabras de corpus
	ifstream ifs("corpus");
	string palabra;
	if (!ifs) { cerr << "Error: cannot open corpus" << endl; exit(1); }
	while (ifs >> palabra) palabras.push_back(palabra);

	// Testeo concurrencia
	pthread_t threads[NUM_THREADS];
	thread_args_s thread_args[NUM_THREADS];
	ConcurrentHashMap *map = new ConcurrentHashMap();
	ConcurrentHashMap *maps[NUM_THREADS];

	for (unsigned int i = 0; i < NUM_THREADS; ++i) {
		maps[i] = new ConcurrentHashMap();
		thread_args[i].map = map;
		thread_args[i].maps_i = maps[i];
		thread_args[i].archs = &archs;
		thread_args[i].palabras = &palabras;

		if (pthread_create(&threads[i], nullptr, thread_function, &thread_args[i])) {
			cerr << "Error: unable to create thread" << endl;
			exit(-1);
		}
	}

	// Espero a que terminen los hijos
	for (unsigned int i = 0; i < NUM_THREADS; ++i) {
		if (pthread_join(threads[i], nullptr)) {
			cerr << "Error: unable to join thread" << endl;
			exit(-1);
		}
	}

	// Corroboro resultados
	list<string> archs2;
	archs.push_back("corpus");
	auto max = ConcurrentHashMap::maximum(MAX_NUM_THREADS, MAX_NUM_THREADS, archs);

	for (unsigned int i = 0; i < NUM_THREADS; ++i) {
		assert(max == maps[i]->maximum(1));
		archs2.push_back("corpus");
		delete(maps[i]);
	}

	assert(ConcurrentHashMap::maximum(MAX_NUM_THREADS, MAX_NUM_THREADS, archs2) == map->maximum(1));
	delete(map);

	return 0;
}