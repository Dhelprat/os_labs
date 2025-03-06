#include "library1.h"

int PrimeCount(int A, int B) {
    int count = 0;
    for (int i = A; i <= B; i++) {
        bool isPrime = true;
        if (i < 2) continue;
        for (int j = 2; j * j <= i; j++) {
            if (i % j == 0) {
                isPrime = false;
                break;
            }
        }
        if (isPrime) count++;
    }
    return count;
}

float Square(float A, float B) {
    return A * B;
}