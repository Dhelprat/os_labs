#include <iostream>
#include <dlfcn.h>

typedef int (*PrimeCountFunc)(int, int);
typedef float (*SquareFunc)(float, float);

int main() {
    void* hLib = nullptr;
    PrimeCountFunc PrimeCount = nullptr;
    SquareFunc Square = nullptr;
    int currentLibrary = 1;

    hLib = dlopen("./liblibrary1.so", RTLD_LAZY);
    if (!hLib) {
        std::cerr << "Failed to load library1.so: " << dlerror() << std::endl;
        return 1;
    }
    PrimeCount = (PrimeCountFunc)dlsym(hLib, "PrimeCount");
    Square = (SquareFunc)dlsym(hLib, "Square");

    int choice;
    while (true) {
        std::cout << "Enter a command (0, 1, or 2) or 4 for exit: ";
        std::cin >> choice;
        std::cout << choice;

        if (choice == 0) {
            dlclose(hLib);
            currentLibrary = (currentLibrary == 1) ? 2 : 1;
            hLib = dlopen(currentLibrary == 1 ? "./liblibrary1.so" : "./liblibrary2.so", RTLD_LAZY);
            if (!hLib) {
                std::cerr << "Failed to load the library: " << dlerror() << std::endl;
                return 1;
            }
            PrimeCount = (PrimeCountFunc)dlsym(hLib, "PrimeCount");
            Square = (SquareFunc)dlsym(hLib, "Square");
            std::cout << "Switched to " << (currentLibrary == 1 ? "library1.so" : "library2.so") << std::endl;
        } else if (choice == 1) {
            int A, B;
            std::cout << "Enter A and B to count prime numbers: ";
            std::cin >> A >> B;
            std::cout << "Number of primes: " << PrimeCount(A, B) << std::endl;
        } else if (choice == 2) {
            float A, B;
            std::cout << "Enter sides A and B: ";
            std::cin >> A >> B;
            std::cout << "Area: " << Square(A, B) << std::endl;
        } else if (choice == 4) {
            break;
        }
    }

    dlclose(hLib);
    return 0;
}