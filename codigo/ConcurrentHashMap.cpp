#include "ConcurrentHashMap.hpp"

#include <fstream>
#include <iostream>

using namespace std;

// Public member functions

ConcurrentHashMap::ConcurrentHashMap() {
    for (int i = 0; i < 26; i++) {
        tabla[i] = new bucket();
        tabla_mutex[i] = new mutex;
    }
}

ConcurrentHashMap::~ConcurrentHashMap() {
    for (int i = 0; i < 26; i++) {
        delete tabla[i];
        delete tabla_mutex[i];
    }
}

ConcurrentHashMap &ConcurrentHashMap::operator=(const ConcurrentHashMap &obj) {
    int i;
    bucket::Iterador iterador;

    for (i = 0; i < 26; i++) {
        tabla_mutex[i]->lock();

        delete tabla[i];
        tabla[i] = new bucket();
        for (iterador = obj.tabla[i]->CrearIt(); iterador.HaySiguiente(); iterador.Avanzar()) {
            tabla[i]->push_front(iterador.Siguiente());
        }

        tabla_mutex[i]->unlock();
    }

    return *this;
}

void ConcurrentHashMap::addAndInc(string key) {
    bucket::Iterador it;

    unsigned int index = hash_key(&key);

    tabla_mutex[index]->lock();

    it = tabla[index]->CrearIt();
    while (it.HaySiguiente() && it.Siguiente().first != key) it.Avanzar();

    if (it.HaySiguiente()) it.Siguiente().second++;
    else tabla[index]->push_front(item(key.data(), 1));

    tabla_mutex[index]->unlock();
}

bool ConcurrentHashMap::member(string key) {
    bucket::Iterador it;

    unsigned int index = hash_key(&key);

    it = tabla[index]->CrearIt();
    while (it.HaySiguiente() && it.Siguiente().first != key) it.Avanzar();

    return it.HaySiguiente();
}

item ConcurrentHashMap::maximum(unsigned int nt) {
    // Preparo los argumentos de la funcion maximum_thread_function
    maximum_thread_args args;
    args.map = this;
    args.index = 0;
    args.general_maximum = item("", 0);

    // Creo los threads y espero a que terminen
    create_and_join_threads(nt, maximum_thread_function, &args);

    return args.general_maximum;
}

// Public static member functions

ConcurrentHashMap ConcurrentHashMap::count_words(string arch) {
    ConcurrentHashMap map;

    count_words(arch, &map);

    return map;
}

ConcurrentHashMap ConcurrentHashMap::count_words(list<string> archs) {
    count_words_thread_args args;
    args.map = new ConcurrentHashMap();
    args.archs_iterator = archs.begin();
    args.archs_iterator_end = archs.end();

    create_and_join_threads(archs.size(), count_words_thread_function, &args);

    return *args.map;
}

ConcurrentHashMap ConcurrentHashMap::count_words(unsigned int n, list<string> archs) {
    count_words_thread_args args;
    args.map = new ConcurrentHashMap();
    args.archs_iterator = archs.begin();
    args.archs_iterator_end = archs.end();

    create_and_join_threads(n, count_words_thread_function, &args);

    return *args.map;
}

item ConcurrentHashMap::maximum(unsigned int p_archivos, unsigned int p_maximos, list<string> archs) {
    count_words_thread_args args;
    args.map = new ConcurrentHashMap();
    args.archs_iterator = archs.begin();
    args.archs_iterator_end = archs.end();

    create_and_join_threads(p_archivos, count_words_thread_function, &args);
    item maximum = args.map->maximum(p_maximos);

    return maximum;
}

// Private member functions

unsigned int ConcurrentHashMap::hash_key(string *key) {
    return (unsigned int) key->at(0) - 97;
}

list<string> ConcurrentHashMap::words(string &arch) {
    list<string> res;
    string palabra;

    ifstream ifs(arch);
    if (!ifs) {
        cerr << "ERROR: Cannot open " << arch << endl;
        exit(1);
    }

    while (ifs >> palabra) {
        res.push_back(palabra);
    }

    return res;
}

void *ConcurrentHashMap::maximum_thread_function(void *thread_args) {
    struct maximum_thread_args *args;
    auto maximum = item("", 0);
    int index;

    args = (struct maximum_thread_args *) thread_args;

    for (index = args->index++; index < 26; index = args->index++) {
        args->map->tabla_mutex[index]->lock();

        for (auto it = args->map->tabla[index]->CrearIt(); it.HaySiguiente(); it.Avanzar()) {
        	if (it.Siguiente().second > maximum.second) maximum = it.Siguiente();
        }

        args->map->tabla_mutex[index]->unlock();
    }

    if (maximum != item("", 0)) {
	    args->general_maximum_mutex.lock();
	    if (maximum.second > args->general_maximum.second)
            args->general_maximum = maximum;
	    args->general_maximum_mutex.unlock();
	}

	return nullptr;
}

void *ConcurrentHashMap::count_words_thread_function(void *thread_args) {
    string arch;
    struct count_words_thread_args *args;

    args = (struct count_words_thread_args *) thread_args;

    bool not_over = true;
    while (not_over) {
        args->archs_iterator_mutex.lock();
        not_over = args->archs_iterator != args->archs_iterator_end;
        if (not_over) {
            arch = *args->archs_iterator;
            args->archs_iterator++;
        }
        args->archs_iterator_mutex.unlock();

        if (not_over) count_words(arch, args->map);
    }

	return nullptr;
}

void *ConcurrentHashMap::count_words_single_file_thread_function(void *thread_args) {
    struct count_words_thread_args *args;
    args = (struct count_words_thread_args *) thread_args;

    list<string>::iterator archs_iterator;
    bool not_over = true;

    args->archs_iterator_mutex.lock();

    archs_iterator = args->archs_iterator;
    if (archs_iterator == args->archs_iterator_end) not_over = false;
    else args->archs_iterator++;

    args->archs_iterator_mutex.unlock();

    if (not_over) count_words(*archs_iterator, args->map);

	return nullptr;
}

void ConcurrentHashMap::count_words(string arch, ConcurrentHashMap *map) {
    list<string> palabras = words(arch);
    list<string>::iterator it;

    for (it = palabras.begin(); it != palabras.end(); it++) {
        map->addAndInc(*it);
    }
}

void ConcurrentHashMap::create_and_join_threads(unsigned int n, void *thread_function(void*), void *thread_args) {
    pthread_t threads[n];
    for (unsigned int i = 0; i < n; ++i) {
        if (pthread_create(&threads[i], nullptr, thread_function, thread_args)) {
            cerr << "Error: unable to create thread" << endl;
            exit(-1);
        }
    }
    for (unsigned int i = 0; i < n; ++i) {
        if (pthread_join(threads[i], nullptr)) {
            cout << "Error: unable to join thread" << endl;
            exit(-1);
        }
    }
}