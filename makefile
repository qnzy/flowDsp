CPP = g++
CPPFLAGS = -Wall -Wextra -Werror -pedantic

testIt: test.o test_flowdsp.o
	$(CPP) $(CPPFLAGS) *.o -o testrun 
	./testrun -d yes

test.o: test/test.cpp test/catch.hpp
	$(CPP) $(CPPFLAGS) -c test/test.cpp

test_%.o: test/test_%.cpp src/*.hpp test/catch.hpp
	$(CPP) $(CPPFLAGS) -Wall -c $<

lint:
	clang-tidy -checks="*,-llvm-header-guard,-fuchsia-default-arguments" -header-filter=".*" src/flowdsp.hpp

.PHONY: lint
