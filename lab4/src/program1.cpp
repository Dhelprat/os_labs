#include <iostream>
#include "library1.h"

int main() {
    int choice;
    while (true) {
        std::cout << "Enter a command (1 or 2): ";
        std::cin >> choice;

        if (choice == 1) {
            int A, B;
            std::cout << "Enter A and B to count prime numbers: ";
            std::cin >> A >> B;
            std::cout << "Number of primes: " << PrimeCount(A, B) << std::endl;
        } else if (choice == 2) {
            float A, B;
            std::cout << "Enter sides A and B: ";
            std::cin >> A >> B;
            std::cout << "Area: " << Square(A, B) << std::endl;
        } else {
            break;
        }
    }
    return 0;
}