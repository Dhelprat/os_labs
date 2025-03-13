#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <cstring>
#include <chrono>


// Структура для передачи данных в поток
struct ThreadData {
    int thread_id;
    int n;                         // размер матрицы
    std::vector<std::vector<int> > matrix;
    int start_col;                 // начальный столбец для потока
    int end_col;                   // конечный столбец для потока
    double result;                 // результат вычислений потока
};

sem_t* semaphore = nullptr;        // семафор для ограничения числа потоков

// Функция для нахождения детерминанта подматрицы
double determinant(const std::vector<std::vector<int> >& matrix, int n) {
    // Если размер матрицы 1x1, возвращаем единственный элемент
    if (n == 1) {
        return matrix[0][0];
    }

    // Если размер матрицы 2x2, используем формулу ad-bc
    if (n == 2) {
        return matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0];
    }

    double det = 0;
    int sign = 1;

    // Вычисляем детерминант через разложение по первой строке
    for (int i = 0; i < n; i++) {
        // Создаем подматрицу (n-1) x (n-1)
        std::vector<std::vector<int> > submatrix(n - 1, std::vector<int>(n - 1));

        // Заполняем подматрицу
        for (int j = 1; j < n; j++) {
            int col_idx = 0;
            for (int k = 0; k < n; k++) {
                if (k == i) continue;
                submatrix[j-1][col_idx++] = matrix[j][k];
            }
        }

        // Суммируем с учётом знака
        det += sign * matrix[0][i] * determinant(submatrix, n - 1);
        sign = -sign;
    }

    return det;
}

// Функция для потока
void* calculate_determinant_thread(void* arg) {
    ThreadData* data = (ThreadData*)arg;

    // Вычисляем детерминант для своего диапазона столбцов
    data->result = 0;
    int sign = (data->start_col % 2 == 0) ? 1 : -1;  // Корректируем знак для начального столбца

    for (int i = data->start_col; i < data->end_col; i++) {
        // Создаем подматрицу (n-1) x (n-1)
        std::vector<std::vector<int> > submatrix(data->n - 1, std::vector<int>(data->n - 1));

        // Заполняем подматрицу (исключаем первую строку и i-й столбец)
        for (int j = 1; j < data->n; j++) {
            int col_idx = 0;
            for (int k = 0; k < data->n; k++) {
                if (k == i) continue;
                submatrix[j-1][col_idx++] = data->matrix[j][k];
            }
        }

        // Вычисляем вклад текущего элемента в детерминант
        double term = sign * data->matrix[0][i] * determinant(submatrix, data->n - 1);
        data->result += term;

        sign = -sign;
    }

    // Освобождаем семафор, позволяя другому потоку запуститься
    sem_post(semaphore);

    return nullptr;
}

// Функция для вывода матрицы (для отладки)
    void print_matrix(const std::vector<std::vector<int> >& matrix, int n) {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                std::cout << matrix[i][j] << "\t";
            }
            std::cout << std::endl;
        }
    }

int main(int argc, char* argv[]) {
    // Проверка аргументов командной строки
    if (argc != 3) {
        std::cerr << "Использование: " << argv[0] << " <размер_матрицы> <макс_число_потоков>" << std::endl;
        return 1;
    }

    // Парсинг аргументов
    int n = atoi(argv[1]);            // размер матрицы
    int max_threads = atoi(argv[2]);  // максимальное число потоков

    // Проверка валидности аргументов
    if (n <= 0) {
        std::cerr << "Размер матрицы должен быть положительным числом" << std::endl;
        return 1;
    }
    if (max_threads <= 0) {
        std::cerr << "Максимальное число потоков должно быть положительным числом" << std::endl;
        return 1;
    }

    // Инициализация генератора случайных чисел
    srand(time(nullptr));

    // Создание и заполнение матрицы случайными числами от 1 до 1000
    std::vector<std::vector<int> > matrix(n, std::vector<int>(n));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            matrix[i][j] = rand() % 1000 + 1;
        }
    }

    auto start = std::chrono::high_resolution_clock::now();
    // Инициализация семафора с нужным количеством потоков
    semaphore = new sem_t;
    sem_init(semaphore, 0, max_threads);

    // Специальная обработка для маленьких матриц
    if (n == 1) {
        std::cout << "Детерминант: " << matrix[0][0] << std::endl;
        sem_destroy(semaphore);
        delete semaphore;
        return 0;
    }

    // Определяем фактическое число потоков
    int num_threads = std::min(max_threads, n);  // Не больше, чем размер матрицы

    // Создаем потоки
    pthread_t* threads = new pthread_t[num_threads];
    ThreadData* thread_data = new ThreadData[num_threads];

    // Равномерно распределяем столбцы между потоками
    int cols_per_thread = n / num_threads;
    int remaining_cols = n % num_threads;

    int start_col = 0;
    for (int i = 0; i < num_threads; i++) {
        // Определяем диапазон столбцов для потока
        int cols = cols_per_thread + (i < remaining_cols ? 1 : 0);

        // Инициализируем данные для потока
        thread_data[i].thread_id = i;
        thread_data[i].n = n;
        thread_data[i].matrix = matrix;
        thread_data[i].start_col = start_col;
        thread_data[i].end_col = start_col + cols;
        thread_data[i].result = 0;

        // Ждем доступный слот (семафор)
        // Это уменьшает счетчик семафора. Если счетчик становится
        // отрицательным, поток блокируется до вызова sem_post
        sem_wait(semaphore);

        // Создаем поток
        pthread_create(&threads[i], nullptr, calculate_determinant_thread, &thread_data[i]);

        // Обновляем начальный столбец для следующего потока
        start_col += cols;
    }

    // Ожидаем завершения всех потоков и суммируем результаты
    double det = 0;
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], nullptr);
        det += thread_data[i].result;
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    std::cout << "Заняло: " << duration.count() << " секунд\n";
    // Выводим результат
    std::cout << "Детерминант: " << det << std::endl;

    // Освобождаем ресурсы
    sem_destroy(semaphore);
    delete semaphore;
    delete[] threads;
    delete[] thread_data;

    return 0;
}
