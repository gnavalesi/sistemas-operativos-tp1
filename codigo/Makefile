.POSIX:
.SUFFIXES:
.SUFFIXES: .o .cpp

CXX = g++
CXXFLAGS = -std=c++11 -Wall
LDLIBS = -lpthread -lrt

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

BIN = test-1 test-2 test-3 test-4 test-5 test-6 test-7 test-8 test-9
OBJ = ConcurrentHashMap.o
RUN = test-1-run test-2-run test-3-run test-4-run test-5-run test-6-run test-7-run test-8-run test-9-run

all: $(BIN)

test: $(RUN)

$(BIN): ListaAtomica.hpp

test-1: $(OBJ) test-1.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ test-1.cpp $(OBJ) $(LDLIBS)

test-1-run: test-1
	./test-1

test-2: $(OBJ) test-2.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ test-2.cpp $(OBJ) $(LDLIBS)

test-2-run: test-2
	awk -f corpus.awk corpus | sort >corpus-post
	./test-2 | sort | diff -u - corpus-post
	rm -f corpus-post

test-3: $(OBJ) test-3.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ test-3.cpp $(OBJ) $(LDLIBS)

test-3-run: test-3
	awk -f corpus.awk corpus | sort >corpus-post
	for i in 0 1 2 3 4; do sed -n "$$((i * 500 + 1)),$$(((i + 1) * 500))p" corpus >corpus-"$$i"; done
	for i in 0 1 2 3 4; do ./test-3 $$((i + 1)) | sort | diff -u - corpus-post; done
	rm -f corpus-post corpus-[0-4]

test-4: $(OBJ) test-4.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ test-4.cpp $(OBJ) $(LDLIBS)

test-4-run: test-4
	./test-4

test-5: $(OBJ) test-5.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ test-5.cpp $(OBJ) $(LDLIBS)

test-5-run: test-5
	awk -f corpus.awk corpus | sort -nk 2 | tail -n 1 >corpus-max
	cat corpus-max
	for i in 0 1 2 3 4; do sed -n "$$((i * 500 + 1)),$$(((i + 1) * 500))p" corpus >corpus-"$$i"; done
	for i in 0 1 2 3 4; do for j in 0 1 2 3 4; do \
		./test-5 $$((i + 1)) $$((j + 1)) | diff -u - corpus-max; \
	done; done
	rm -f corpus-max corpus-[0-4]

test-6: $(OBJ) test-6.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ test-6.cpp $(OBJ) $(LDLIBS)

test-6-run: test-6
	for i in 0 1 2 3 4; do sed -n "$$((i * 500 + 1)),$$(((i + 1) * 500))p" corpus >corpus-"$$i"; done
	./test-6
	rm -f corpus-[0-4]

test-7: $(OBJ) test-7.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ test-7.cpp $(OBJ) $(LDLIBS)

test-7-run: test-7
	for i in $$(seq 0 999); do sed -n "$$((i * 500 + 1)),$$(((i + 1) * 500))p" corpus >corpus-"$$i"; done
	echo "p_archivos,p_maximos,maximum_time,maximum2_time" > test-7.csv
	for i in $$(seq 1 5); do for j in $$(seq 1 5); do echo -n "$$i,$$j,"; ./test-7 1000 $$i $$j; done; done >> test-7.csv
	rm -f corpus-*

test-8: $(OBJ) test-8.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ test-8.cpp $(OBJ) $(LDLIBS)

test-8-run: test-8
	./test-8 100000

test-9: $(OBJ) test-9.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ test-9.cpp $(OBJ) $(LDLIBS)

test-9-run: test-9
	./test-9 10000

clean:
	rm -f $(BIN) $(OBJ)
	rm -f corpus-*
