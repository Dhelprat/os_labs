#include <vector>

extern "C" int PrimeCount(int A, int B) {
    if (B < 2) return 0;

    std::vector<bool> isPrime(B + 1, true);
    isPrime[0] = isPrime[1] = false;

    for (int i = 2; i * i <= B; i++) {
        if (isPrime[i]) {
            for (int j = i * i; j <= B; j += i) {
                isPrime[j] = false;
            }
        }
    }

    int count = 0;
    for (int i = A; i <= B; i++) {
        if (isPrime[i]) count++;
    }
    return count;
}