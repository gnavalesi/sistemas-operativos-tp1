#ifndef CONCURRENT_HASH_MAP_H__
#define CONCURRENT_HASH_MAP_H__

#include <string>
#include "ListaAtomica.hpp"

using namespace std;

class ConcurrentHashMap {
private:


public:
    ConcurrentHashMap();
    ~ConcurrentHashMap();

    void addAndInc(string key);
    bool member(string key);
    pair<string, unsigned int> maximum(unsigned int nt);
};

#endif /* CONCURRENT_HASH_MAP_H__*/
