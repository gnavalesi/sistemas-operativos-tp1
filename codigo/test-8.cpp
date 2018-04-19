#include "ConcurrentHashMap.hpp"
#include "tiempo.hpp"

#include <cassert>
#include <iostream>
#include <list>

using namespace std;

struct thread_args_s {
	ConcurrentHashMap *map;
	list<string> *palabras;
};

void *thread_function(void *thread_args) {
	thread_args_s *args = (thread_args_s *) thread_args;

	for (auto it = args->palabras->begin(); it != args->palabras->end(); ++it)
		args->map->addAndInc(*it);

	return nullptr;
}

int main(int argc, char const *argv[]) {
	// Parseo argumentos
	if (argc != 2) {
		cout << "uso: " << argv[0] << " cantidad_de_palabras" << endl;
	}

	unsigned int cant_palabras = stoi(argv[1]);

	// Armo listas con palabras
	list<string> palabras[2];
	for (unsigned int j = 0; j < cant_palabras; ++j) {
		palabras[0].push_back("a");
		palabras[1].push_back("b");
	}

	timespec tiempo;


	// addAndInc secuencial

	ConcurrentHashMap map_sec;
	pthread_t threads_sec[2];
	thread_args_s thread_args_sec[2];

	tiempo = gettime();

	for (int i = 0; i < 2; ++i) {
		thread_args_sec[i].map = &map_sec;
		thread_args_sec[i].palabras = &palabras[0];

		if (pthread_create(&threads_sec[i], nullptr, thread_function, &thread_args_sec[i])) {
			cerr << "Error: unable to create thread" << endl;
			exit(1);
		}
	}

	for (int i = 0; i < 2; ++i) {
		if (pthread_join(threads_sec[i], nullptr)) {
			cerr << "Error: unable to create thread" << endl;
			exit(1);
		}
	}

	auto tiempo_sec = timediff(tiempo, gettime());


	// addAndInc concurrente

	ConcurrentHashMap map_con;
	pthread_t threads_con[2];
	thread_args_s thread_args_con[2];

	tiempo = gettime();

	for (int i = 0; i < 2; ++i) {
		thread_args_con[i].map = &map_sec;
		thread_args_con[i].palabras = &palabras[i];

		if (pthread_create(&threads_con[i], nullptr, thread_function, &thread_args_con[i])) {
			cerr << "Error: unable to create thread" << endl;
			exit(1);
		}
	}

	for (int i = 0; i < 2; ++i) {
		if (pthread_join(threads_con[i], nullptr)) {
			cerr << "Error: unable to create thread" << endl;
			exit(1);
		}
	}

	auto tiempo_con = timediff(tiempo, gettime());


	cout << "Secuencial:  " << tiempo_sec << endl;
	cout << "Concurrente: " << tiempo_con << endl;
	cout << "Concurrente/Secuencial: " << (double) tiempo_con / (double) tiempo_sec << endl;

	return 0;
}