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
}

void ConcurrentHashMap::addAndInc(string key) {
    bool encontrado = false;
    bucket::Iterador it;

    unsigned int index = hash_key(&key);

    tabla_mutex[index]->lock();

    for (it = tabla[index]->CrearIt(); it.HaySiguiente(); it.Avanzar()) {
        encontrado = it.Siguiente().first == key;
        if (encontrado) {
            it.Siguiente().second++;
            break;
        }
    }

    if (!encontrado) tabla[index]->push_front(item(key.data(), 1));

    tabla_mutex[index]->unlock();
}

bool ConcurrentHashMap::member(string key) {
    bool encontrado = false;
    bucket::Iterador it;

    unsigned int index = hash_key(&key);

    for (it = tabla[index]->CrearIt(); it.HaySiguiente(); it.Avanzar()) {
        encontrado = it.Siguiente().first == key;
        if (encontrado) break;
    }

    return encontrado;
}

item ConcurrentHashMap::maximum(unsigned int nt) {
    int rc;
    int i;
    pthread_attr_t attr;
    pthread_t threads[nt];
    void *status;
    maximum_thread_args args;

    // Preparo los atributos de los threads
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    // Preparo los argumentos de la funcion maximum_thread_function
    args.map = this;
    args.index = 0;
    args.general_maximum = item("", 0);

    // Creo los threads
    for (i = 0; i < nt; i++) {
        rc = pthread_create(&threads[i], nullptr, maximum_thread_function, (void *) &args);

        if (rc) {
            cerr << "Error:unable to create thread," << rc << endl;
            exit(-1);
        }
    }

    // Espero por cada uno de los threads
    pthread_attr_destroy(&attr);
    for (i = 0; i < nt; i++) {
        rc = pthread_join(threads[i], &status);
        if (rc) {
            cout << "Error:unable to join," << rc << endl;
            exit(-1);
        }
    }

    return args.general_maximum;
}

// Public static member functions

ConcurrentHashMap ConcurrentHashMap::count_words(string arch) {
    ConcurrentHashMap map;

    count_words(arch, &map);

    return map;
}

ConcurrentHashMap ConcurrentHashMap::count_words(list<string> archs) {
    auto map = new ConcurrentHashMap();
    int rc;
    int i;
    pthread_t threads[archs.size()];
    pthread_attr_t attr;
    void *status;
    count_words_single_file_thread_args args;

    args.map = map;
    args.archs_iterator = archs.begin();

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    for (i = 0; i < archs.size(); i++) {
        rc = pthread_create(&threads[i], nullptr, count_words_single_file_thread_function, &args);

        if (rc) {
            cerr << "Error:unable to create thread," << rc << endl;
            exit(-1);
        }
    }

    pthread_attr_destroy(&attr);
    for (i = 0; i < archs.size(); i++) {
        rc = pthread_join(threads[i], &status);
        if (rc) {
            cout << "Error:unable to join," << rc << endl;
            exit(-1);
        }
    }

    return *map;
}

ConcurrentHashMap ConcurrentHashMap::count_words(unsigned int n, list<string> archs) {
    auto map = new ConcurrentHashMap();
    int rc;
    int i;
    pthread_t threads[n];
    pthread_attr_t attr;
    void *status;

    count_words_thread_args args;

    args.map = map;
    args.archs_iterator = archs.begin();
    args.archs_iterator_end = archs.end();

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    for (i = 0; i < n; i++) {
        rc = pthread_create(&threads[i], nullptr, count_words_thread_function, &args);

        if (rc) {
            cerr << "Error:unable to create thread," << rc << endl;
            exit(-1);
        }
    }

    pthread_attr_destroy(&attr);
    for (i = 0; i < n; i++) {
        rc = pthread_join(threads[i], &status);
        if (rc) {
            cout << "Error:unable to join," << rc << endl;
            exit(-1);
        }
    }

    return *map;
}

item ConcurrentHashMap::maximum(unsigned int p_archivos, unsigned int p_maximos, list<string> archs) {
    int rc, i;
    list<string>::iterator it;

    pthread_t archivos_threads[p_archivos];
    count_words_thread_args archivos_threads_args;

    pthread_attr_t attr;
    void *status;

    auto map = new ConcurrentHashMap;

    // Preparo los atributos de los threads
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    archivos_threads_args.map = map;
    archivos_threads_args.archs_iterator = archs.begin();
    archivos_threads_args.archs_iterator_end = archs.end();

    for (i = 0; i < archs.size(); i++) {
        rc = pthread_create(&archivos_threads[i], nullptr, count_words_thread_function, &archivos_threads_args);

        if (rc) {
            cerr << "Error:unable to create thread," << rc << endl;
            exit(-1);
        }
    }

    for (i = 0; i < archs.size(); i++) {
        rc = pthread_join(archivos_threads[i], &status);
        if (rc) {
            cout << "Error:unable to join," << rc << endl;
            exit(-1);
        }
    }

    item maximum = map->maximum(p_maximos);

    return maximum;
}

// Private member functions

unsigned int ConcurrentHashMap::hash_key(string *key) {
    return (unsigned int) key->at(0) - 97;
}

list<string> ConcurrentHashMap::words(string &arch) {
    list<string> res;
    string line;

    ifstream ifs(arch);
    if (!ifs) {
        cerr << "ERROR: Cannot open " << arch << endl;
        exit(1);
    }

    while (getline(ifs, line)) {
        res.push_back(line);
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

        bucket::Iterador iterador = args->map->tabla[index]->CrearIt();
        while (iterador.HaySiguiente()) {
            if (iterador.Siguiente().second > maximum.second) maximum = iterador.Siguiente();
            iterador.Avanzar();
        }

        args->map->tabla_mutex[index]->unlock();
    }

    args->general_maximum_mutex.lock();
    if (maximum.second > args->general_maximum.second) {
        args->general_maximum.first = maximum.first;
        args->general_maximum.second = maximum.second;
    }
    args->general_maximum_mutex.unlock();
}

void *ConcurrentHashMap::count_words_thread_function(void *thread_args) {
    string arch;
    bool is_over;
    struct count_words_thread_args *args;

    args = (struct count_words_thread_args *) thread_args;

    while (true) {
        args->archs_iterator_mutex.lock();
        if (!(is_over = (args->archs_iterator == args->archs_iterator_end))) {
            arch = *args->archs_iterator;
            args->archs_iterator++;
        }

        args->archs_iterator_mutex.unlock();
        if (!is_over) {
            count_words(arch, args->map);
        } else {
            break;
        }
    }
}

void *ConcurrentHashMap::count_words_single_file_thread_function(void *thread_args) {
    struct count_words_single_file_thread_args *args;
    args = (struct count_words_single_file_thread_args *) thread_args;

    string arch;

    args->archs_iterator_mutex.lock();
    arch = *args->archs_iterator;
    args->archs_iterator++;
    args->archs_iterator_mutex.unlock();

    count_words(arch, args->map);
}

void ConcurrentHashMap::count_words(string arch, ConcurrentHashMap *map) {
    list<string> palabras = words(arch);
    list<string>::iterator it;

    for (it = palabras.begin(); it != palabras.end(); it++) {
        map->addAndInc(*it);
    }
}
