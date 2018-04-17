#include <iostream>
#include <cassert>
#include "ConcurrentHashMap.hpp"

using namespace std;

int main(void) {
    ConcurrentHashMap h;

    h.addAndInc("aguila");
    assert(h.member("aguila"));

    h.addAndInc("bisonte");
    assert(h.member("bisonte"));

    h.addAndInc("burro");
    assert(h.member("burro"));

    h.addAndInc("aguila");
    assert(h.member("aguila"));

    pair<string, int> max1 = h.maximum(1);
    assert(max1.first == "aguila");
    assert(max1.second == 2);

    h.addAndInc("bisonte");
    h.addAndInc("bisonte");

    pair<string, int> max2 = h.maximum(4);
    assert(max2.first == "bisonte");
    assert(max2.second == 3);

	return 0;
}

