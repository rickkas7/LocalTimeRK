

all : TimeTest
	export TZ='UTC' && ./TimeTest

TimeTest : TimeTest.cpp ../src/LocalTimeRK.cpp ../src/LocalTimeRK.h libwiringgcc
	gcc TimeTest.cpp ../src/LocalTimeRK.cpp UnitTestLib/libwiringgcc.a -DUNITTEST -std=c++11 -lc++ -IUnitTestLib -I../src -o TimeTest

check : TimeTest.cpp ../src/LocalTimeRK.cpp ../src/LocalTimeRK.h libwiringgcc
	gcc TimeTest.cpp ../src/LocalTimeRK.cpp UnitTestLib/libwiringgcc.a -g -O0 -std=c++11 -lc++ -IUnitTestLib -I ../src -o TimeTest && valgrind --leak-check=yes ./TimeTest 

libwiringgcc :
	cd UnitTestLib && make libwiringgcc.a 	
	
.PHONY: libwiringgcc
