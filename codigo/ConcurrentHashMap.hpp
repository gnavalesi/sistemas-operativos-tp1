#ifndef CONCURRENT_HASH_MAP_H__
#define CONCURRENT_HASH_MAP_H__

#include <list>
#include <mutex>
#include <string>
#include <vector>

#include "ListaAtomica.hpp"

using namespace std;

typedef pair<string, unsigned int> item;
typedef Lista<item> bucket;

class ConcurrentHashMap {
private:
    mutex *tabla_mutex[26];

    struct maximum_thread_args {
        ConcurrentHashMap *map = nullptr;
        atomic<int> index;
        item general_maximum;
        mutex general_maximum_mutex;
    };

    struct count_words_thread_args {
        ConcurrentHashMap *map = nullptr;
        list<string>::iterator archs_iterator;
        list<string>::iterator archs_iterator_end;
        mutex archs_iterator_mutex;
    };

    static unsigned int hash_key(string *key);
    static list<string> words(string &arch);
    static void *maximum_thread_function(void *thread_args);
    static void *count_words_thread_function(void *thread_args);
    static void *count_words_single_file_thread_function(void *thread_args);
    static void count_words(string arch, ConcurrentHashMap *map);

public:
    bucket* tabla[26];

    ConcurrentHashMap();
    ~ConcurrentHashMap();
    ConcurrentHashMap& operator= (const ConcurrentHashMap& obj);
    void addAndInc(string key);
    bool member(string key);
    item maximum(unsigned int nt);
    static ConcurrentHashMap count_words(string arch);
    static ConcurrentHashMap count_words(list <string> archs);
    static ConcurrentHashMap count_words(unsigned int n, list <string> archs);
    static item maximum(unsigned int p_archivos, unsigned int p_maximos, list <string> archs);
};

#endif /* CONCURRENT_HASH_MAP_H__*/
