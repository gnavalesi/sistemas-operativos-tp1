#include <iostream>
#include <cstdlib>
#include <cassert>
#include "ConcurrentHashMap.hpp"
#include <list>

using namespace std;

#define NUM_THREADS 4

struct thread_args {
    ConcurrentHashMap* h;
    list<string>* words;
};

void *count_words(void* thr_args) {
    struct thread_args *args;
    list<string>::iterator it;

    args = (struct thread_args *) thr_args;

    for (it = args->words->begin(); it != args->words->end(); it++) {
        args->h->addAndInc(*it);
        assert(args->h->member(*it));
    }

    return nullptr;
}

int main(int argc, char **argv) {
    auto h = new ConcurrentHashMap;
    list<string> ls[] = {
            { "aguila", "bisonte", "burro", "conejo" },
            { "cebra", "aguila", "ballena", "delfin" },
            { "bisonte", "aguila", "conejo", "aguila" },
            { "aguila", "conejo", "burro", "delfin" }
    };

	int i;
    pthread_t threads[NUM_THREADS];
    thread_args args[NUM_THREADS];
    pair<string, unsigned int> max;

    for(i = 0; i < NUM_THREADS; i++) {
        args[i].h = h;
        args[i].words = &ls[i];

        if (pthread_create(&threads[i], nullptr, count_words, &args[i])) {
            cerr << "Error: unable to create thread" << endl;
            exit(-1);
        }
    }

    for (i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], nullptr)) {
            cout << "Error: unable to join thread" << endl;
            exit(-1);
        }
    }

    max = h->maximum(1);

    assert(max.first == "aguila");
    assert(max.second == 5);

    delete h;

	return 0;
}

