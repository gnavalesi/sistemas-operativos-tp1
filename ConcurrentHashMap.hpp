#ifndef CONCURRENT_HASH_MAP_H__
#define CONCURRENT_HASH_MAP_H__

#include <string>
#include <list>
#include "ListaAtomica.hpp"

using namespace std;

class ConcurrentHashMap {
private:


public:
    ConcurrentHashMap();
    ~ConcurrentHashMap();

    Lista<pair<string, unsigned int>>* tabla[];

    void addAndInc(string key);
    bool member(string key);
    pair<string, unsigned int> maximum(unsigned int nt);

    static ConcurrentHashMap count_words(string arch);
    static ConcurrentHashMap count_words(list<string> archs);
    static ConcurrentHashMap count_words(unsigned int n, list<string> archs);
    static pair<string, unsigned int> maximum(unsigned int p_archivos, unsigned int p_maximos, list<string> archs);
};

#endif /* CONCURRENT_HASH_MAP_H__*/
