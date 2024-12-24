CXX = g++
CXXFLAGS = -Wall -g

test: mytest.o filesys.o
	$(CXX) $(CXXFLAGS) mytest.o filesys.o -o test

mytest.o: mytest.cpp filesys.h
	$(CXX) $(CXXFLAGS) -c mytest.cpp

mqueue.o: mqueue.cpp mqueue.h
	$(CXX) $(CXXFLAGS) -c filesys.cpp

clean:
	rm -f *.o
	rm -f test
	rm -f *~

run: test
	./test

val: test
	valgrind ./test

val1:
	valgrind --track-origins=yes ./test

leak:
	valgrind --leak-check=full ./test
