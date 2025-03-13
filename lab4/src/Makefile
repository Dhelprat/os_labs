all: program1 program2 liblibrary1.so liblibrary2.so

# Program 1 (статическая линковка)
program1: program1.o prime_naive.o area_rect.o
	g++ -o program1 program1.o prime_naive.o area_rect.o

program1.o: program1.cpp
	g++ -c program1.cpp

prime_naive.o: prime_naive.cpp
	g++ -c prime_naive.cpp

area_rect.o: area_rect.cpp
	g++ -c area_rect.cpp

# Program 2 (динамическая загрузка)
program2: program2.o
	g++ -o program2 program2.o -ldl

program2.o: program2.cpp
	g++ -c program2.cpp

# Динамические библиотеки
liblibrary1.so: prime_naive.o area_rect.o
	g++ -shared -o liblibrary1.so prime_naive.o area_rect.o

liblibrary2.so: prime_erat.o area_triangle.o
	g++ -shared -o liblibrary2.so prime_erat.o area_triangle.o

prime_erat.o: prime_erat.cpp
	g++ -c -fPIC prime_erat.cpp

area_triangle.o: area_triangle.cpp
	g++ -c -fPIC area_triangle.cpp

clean:
	rm -f program1 program2 *.o *.so
