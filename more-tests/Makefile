

all : TimeTest
	./TimeTest

TimeTest : TimeTest.cpp ../src/LocalTimeRK.cpp ../src/LocalTimeRK.h libwiringgcc
	gcc TimeTest.cpp ../src/LocalTimeRK.cpp gcclib/libwiringgcc.a -DUNITTEST -std=c++11 -lc++ -Igcclib -I../src -o TimeTest

check : TimeTest.cpp ../src/LocalTimeRK.cpp ../src/LocalTimeRK.h libwiringgcc
	gcc TimeTest.cpp ../src/LocalTimeRK.cpp gcclib/libwiringgcc.a -g -O0 -std=c++11 -lc++ -Igcclib -I ../src -o TimeTest && valgrind --leak-check=yes ./TimeTest 

libwiringgcc :
	cd gcclib && make libwiringgcc.a 	
	
.PHONY: libwiringgcc
