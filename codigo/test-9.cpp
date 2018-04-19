#include "ConcurrentHashMap.hpp"
#include "tiempo.hpp"

#include <cassert>
#include <iostream>
#include <list>

using namespace std;

struct thread_args_s {
	ConcurrentHashMap *map;
	list<string> *palabras;
	unsigned int cant_palabras;
};

void *thread_function(void *thread_args) {
	thread_args_s *args = (thread_args_s *) thread_args;
	auto map = args->map;
	auto palabras = args->palabras;
	auto cant_palabras = args->cant_palabras;

	if (args->palabras == nullptr) {
		for (unsigned int i = 0; i < cant_palabras; ++i) map->maximum(2);
	} else {
		for (auto it = palabras->begin(); it != palabras->end(); ++it)
			map->addAndInc(*it);
	}

	return nullptr;
}

int main(int argc, char const *argv[]) {
	// Parseo argumentos
	if (argc != 2) {
		cout << "uso: " << argv[0] << " cantidad_de_palabras" << endl;
	}

	unsigned int cant_palabras = stoi(argv[1]);

	// Armo listas con palabras
	list<string> palabras;
	for (unsigned int i = 0; i < cant_palabras; ++i) palabras.push_back("a");

	timespec tiempo;


	// Corrida secuencial

	ConcurrentHashMap map_sec = ConcurrentHashMap::count_words("corpus");

	tiempo = gettime();

	for (auto it = palabras.begin(); it != palabras.end(); ++it) map_sec.addAndInc(*it);
	for (unsigned int i = 0; i < cant_palabras; ++i) map_sec.maximum(2);

	auto tiempo_sec = timediff(tiempo, gettime());


	// Corrida concurrente

	ConcurrentHashMap map_con = ConcurrentHashMap::count_words("corpus");
	pthread_t threads_con[2];
	thread_args_s thread_args_con[2];
	thread_args_con[0].palabras = &palabras;
	thread_args_con[1].palabras = nullptr;

	tiempo = gettime();

	for (int i = 0; i < 2; ++i) {
		thread_args_con[i].map = &map_sec;
		thread_args_con[i].cant_palabras = cant_palabras;

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