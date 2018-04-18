#include "ConcurrentHashMap.hpp"

#include <fstream>
#include <iostream>

using namespace std;

// Public member functions

ConcurrentHashMap::ConcurrentHashMap() {
    tabla_mutex = new mutex;
    for (unsigned int i = 0; i < NUM_BUCKETS; i++) {
        tabla[i] = new bucket();
        bucket_mutex[i] = new mutex;
    }
}

ConcurrentHashMap::~ConcurrentHashMap() {
    for (unsigned int i = 0; i < NUM_BUCKETS; i++) {
        delete tabla[i];
        delete bucket_mutex[i];
    }
}

ConcurrentHashMap &ConcurrentHashMap::operator=(const ConcurrentHashMap &obj) {
    bucket::Iterador iterador;

    for (unsigned int i = 0; i < NUM_BUCKETS; i++) {
        bucket_mutex[i]->lock();

        delete tabla[i];
        tabla[i] = new bucket();
        for (iterador = obj.tabla[i]->CrearIt(); iterador.HaySiguiente(); iterador.Avanzar()) {
            tabla[i]->push_front(iterador.Siguiente());
        }

        bucket_mutex[i]->unlock();
    }

    return *this;
}

void ConcurrentHashMap::addAndInc(string key) {

    unsigned int index = hash_key(&key);

    tabla_mutex->lock();
    bucket_mutex[index]->lock();

    // Busco la clave
    bucket::Iterador it = tabla[index]->CrearIt();
    while (it.HaySiguiente() && it.Siguiente().first != key) it.Avanzar();

    // Si la encuentro, incremento la cuenta. Si no, la inicializo en 1.
    if (it.HaySiguiente()) it.Siguiente().second++;
    else tabla[index]->push_front(item(key.data(), 1));

    bucket_mutex[index]->unlock();
    tabla_mutex->unlock();
}

bool ConcurrentHashMap::member(string key) {
    unsigned int index = hash_key(&key);

    // Busco la clave
    bucket::Iterador it = tabla[index]->CrearIt();
    while (it.HaySiguiente() && it.Siguiente().first != key) it.Avanzar();

    // Devuelvo si la encontre o no
    return it.HaySiguiente();
}

item ConcurrentHashMap::maximum(unsigned int nt) {
    // Preparo los argumentos de los threads
    maximum_thread_args args;
    args.map = this;
    args.index = 0;
    args.general_maximum = item("", 0);

    tabla_mutex->lock();

    // Creo los threads y espero a que terminen
    create_and_join_threads(nt, maximum_thread_function, &args);

    tabla_mutex->unlock();

    return args.general_maximum;
}

// Public static member functions

ConcurrentHashMap ConcurrentHashMap::count_words(string arch) {
    ConcurrentHashMap map;
    count_words(arch, &map);
    return map;
}

ConcurrentHashMap ConcurrentHashMap::count_words(list<string> archs) {
    // Preparo los argumentos de los threads
    count_words_thread_args args;
    args.map = new ConcurrentHashMap();
    args.archs_iterator = archs.begin();
    args.archs_iterator_end = archs.end();

    // Creo los threads y espero a que terminen
    create_and_join_threads(archs.size(), count_words_single_file_thread_function, &args);

    return *args.map;
}

ConcurrentHashMap ConcurrentHashMap::count_words(unsigned int n, list<string> archs) {
    // Preparo los argumentos de los threads
    count_words_thread_args args;
    args.map = new ConcurrentHashMap();
    args.archs_iterator = archs.begin();
    args.archs_iterator_end = archs.end();

    // Creo los threads y espero a que terminen
    create_and_join_threads(n, count_words_thread_function, &args);

    return *args.map;
}

item ConcurrentHashMap::maximum(unsigned int p_archivos, unsigned int p_maximos, list<string> archs) {
    unsigned int i;
    list<ConcurrentHashMap*> maps;
    list<ConcurrentHashMap*>::iterator maps_iterator;
    item current_item;
    bucket::Iterador bucket_iterator;
    ConcurrentHashMap general_map;

    for (i = 0; i < archs.size(); i++) {
        maps.push_back(new ConcurrentHashMap());
    }

    // Preparo los argumentos de los threads
    maximum_count_words_thread_args args;
    args.maps_iterator = maps.begin();
    args.archs_iterator = archs.begin();
    args.archs_iterator_end = archs.end();

    // Creo los threads y espero a que terminen
    create_and_join_threads(p_archivos, maximum_count_words_thread_function, &args);

    for (maps_iterator = maps.begin(); maps_iterator != maps.end(); maps_iterator++) {
        for (i = 0; i < NUM_BUCKETS; i++) {
            bucket_iterator = (*maps_iterator)->tabla[i]->CrearIt();

            for (; bucket_iterator.HaySiguiente(); bucket_iterator.Avanzar())
                general_map.addAndSum(bucket_iterator.Siguiente());
        }
    }

    // Devuelvo el maximo
    return general_map.maximum(p_maximos);
}

item ConcurrentHashMap::maximum2(unsigned int p_archivos, unsigned int p_maximos, list<string> archs) {
    return count_words(p_archivos, archs).maximum(p_maximos);
}

// Private member functions

unsigned int ConcurrentHashMap::hash_key(string *key) {
    return (unsigned int) key->at(0) - 97;
}

list<string> ConcurrentHashMap::words(string &arch) {
    list<string> palabras;
    string palabra;

    // Abro el archivo
    ifstream ifs(arch);
    if (!ifs) {
        cerr << "Error: cannot open " << arch << endl;
        exit(1);
    }

    // Leo cada palabra y las guardo en la lista
    while (ifs >> palabra) palabras.push_back(palabra);

    return palabras;
}

void *ConcurrentHashMap::maximum_thread_function(void *thread_args) {
    struct maximum_thread_args *args;
    item maximum = item("", 0);
    int index;

    args = (struct maximum_thread_args *) thread_args;

    // Obtengo el indice de un bucket y guardo el maximo local
    for (index = args->index++; index < NUM_BUCKETS; index = args->index++) {
        for (auto it = args->map->tabla[index]->CrearIt(); it.HaySiguiente(); it.Avanzar()) {
        	if (it.Siguiente().second > maximum.second) maximum = it.Siguiente();
        }
    }

    // Actualizo el maximo global si encontre un maximo local
    if (maximum != item("", 0)) {
	    args->general_maximum_mutex.lock();
	    if (maximum.second > args->general_maximum.second) args->general_maximum = maximum;
	    args->general_maximum_mutex.unlock();
	}

	return nullptr;
}

void *ConcurrentHashMap::count_words_thread_function(void *thread_args) {
    string arch;
    struct count_words_thread_args *args;

    args = (struct count_words_thread_args *) thread_args;

    // Obtengo un archivo de la lista y hago count_words mientras queden archivos
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

void *ConcurrentHashMap::maximum_count_words_thread_function(void *thread_args) {
    string arch;
    ConcurrentHashMap* map = nullptr;
    struct maximum_count_words_thread_args *args;

    args = (struct maximum_count_words_thread_args *) thread_args;

    // Obtengo un archivo de la lista y hago count_words mientras queden archivos
    bool not_over = true;
    while (not_over) {
        args->iterators_mutex.lock();
        not_over = args->archs_iterator != args->archs_iterator_end;
        if (not_over) {
            arch = *args->archs_iterator;
            args->archs_iterator++;

            map = *args->maps_iterator;
            args->maps_iterator++;
        }
        args->iterators_mutex.unlock();

        if (not_over) count_words(arch, map);
    }

    return nullptr;
}

void *ConcurrentHashMap::count_words_single_file_thread_function(void *thread_args) {
    struct count_words_thread_args *args;
    args = (struct count_words_thread_args *) thread_args;

    list<string>::iterator archs_iterator;
    bool not_over = true;

    // Obtengo un archivo de la lista si quedan
    args->archs_iterator_mutex.lock();
    archs_iterator = args->archs_iterator;
    if (archs_iterator == args->archs_iterator_end) not_over = false;
    else args->archs_iterator++;
    args->archs_iterator_mutex.unlock();

    // Llamo count_words si consegui un archivo
    if (not_over) count_words(*archs_iterator, args->map);

	return nullptr;
}

void ConcurrentHashMap::count_words(string arch, ConcurrentHashMap *map) {
    // Obtengo una lista de las palabras del archivo
    list<string> palabras = words(arch);

    // Cuento las palabras de la lista
    for (auto it = palabras.begin(); it != palabras.end(); it++) {
        map->addAndInc(*it);
    }
}

void ConcurrentHashMap::create_and_join_threads(unsigned int n, void *thread_function(void*), void *thread_args) {
    pthread_t threads[n];

    // Creo los threads
    for (unsigned int i = 0; i < n; ++i) {
        if (pthread_create(&threads[i], nullptr, thread_function, thread_args)) {
            cerr << "Error: unable to create thread" << endl;
            exit(-1);
        }
    }

    // Espero a que terminen los threads
    for (unsigned int i = 0; i < n; ++i) {
        if (pthread_join(threads[i], nullptr)) {
            cout << "Error: unable to join thread" << endl;
            exit(-1);
        }
    }
}

void ConcurrentHashMap::addAndSum(item current_item) {
    unsigned int index = hash_key(&current_item.first);

    // Busco la clave
    bucket::Iterador it = tabla[index]->CrearIt();
    while (it.HaySiguiente() && it.Siguiente().first != current_item.first) it.Avanzar();

    // Si la encuentro, incremento la cuenta. Si no, la inicializo en el valor del item.
    if (it.HaySiguiente()) it.Siguiente().second += current_item.second;
    else tabla[index]->push_front(item(current_item.first, current_item.second));
}
