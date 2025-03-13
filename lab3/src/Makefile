all: parent child1 child2

parent: parent.cpp common.h
	g++ -o parent parent.cpp

child1: child1.cpp common.h
	g++ -o child1 child1.cpp

child2: child2.cpp common.h
	g++ -o child2 child2.cpp

clean:
	rm -f parent child1 child2
