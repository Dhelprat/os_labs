#include <iostream>
#include <vector>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <cstdlib>

struct ThreadData {
    std::vector<std::vector<int> > matrix;
    size_t size;
    size_t column;
    int nThreads;
    sem_t* semaphore;
    int result;
};

int calculateDeterminant(const std::vector<std::vector<int> >& matrix, size_t size, size_t nThreads);

void* calculateSubDeterminant(void* param) {
    ThreadData* data = static_cast<ThreadData*>(param);
    size_t newSize = data->size - 1;

    std::vector<std::vector<int> > subMatrix(newSize, std::vector<int>(newSize));

    for (int i = 1; i < data->size; i++) {
        int colIndex = 0;
        for (int j = 0; j < data->size; ++j) {
            if (j == data->column) {
                continue;
            }
            subMatrix[i - 1][colIndex] = data->matrix[i][j];
            colIndex++;
        }
    }

    data->result = calculateDeterminant(subMatrix, newSize, data->nThreads);
    sem_post(data->semaphore);
    return nullptr;
}

int calculateDeterminant(const std::vector<std::vector<int> >& matrix, size_t size, size_t nThreads) {
    if (size == 1) {
        return matrix[0][0];
    }

    sem_t semaphore;
    sem_init(&semaphore, 0, nThreads);

    int determinant = 0;
    pthread_t* threads = new pthread_t[size];
    ThreadData* data = new ThreadData[size];

    for (int col = 0; col < size; ++col) {
        sem_wait(&semaphore);

        data[col].matrix = matrix;
        data[col].size = size;
        data[col].column = col;
        data[col].semaphore = &semaphore;
        data[col].nThreads = nThreads;
        data[col].result = 0;

        pthread_create(&threads[col], nullptr, calculateSubDeterminant, &data[col]);
    }

    for (int col = 0; col < size; ++col) {
        pthread_join(threads[col], nullptr);
        int sign = (col % 2 == 0) ? 1 : -1;
        determinant += sign * matrix[0][col] * data[col].result;
    }

    delete[] threads;
    delete[] data;
    sem_destroy(&semaphore);

    return determinant;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <max_threads>" << std::endl;
        return 1;
    }

    size_t maxThreads = std::atoi(argv[1]);
    if (maxThreads < 1) {
        std::cerr << "Invalid number of threads. Must be at least 1." << std::endl;
        return 1;
    }

    size_t size;
    std::cout << "Enter matrix size: ";
    std::cin >> size;

    std::vector<std::vector<int> > matrix(size, std::vector<int>(size));
    std::cout << "Enter matrix elements: ";
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            std::cin >> matrix[i][j];
        }
    }

    int determinant = calculateDeterminant(matrix, size, maxThreads);
    std::cout << "Determinant: " << determinant << std::endl;
    return 0;
}
