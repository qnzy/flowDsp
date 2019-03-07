testIt: test.o test_flowdsp.o
	g++ *.o -o testrun 
	./testrun -d yes

test.o: test/test.cpp test/catch.hpp
	g++ -Wall -c test/test.cpp

test_%.o: test/test_%.cpp src/*.hpp test/catch.hpp
	g++ -Wall -c $<

