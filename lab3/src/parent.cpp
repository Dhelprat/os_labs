#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <vector>
#include <string>
#include "common.h"

void prepareFileForMapping(const char* filename, size_t size) {
    int fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        perror("Error creating mapped file");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(fd, size) == -1) {
        perror("Error setting file size");
        close(fd);
        exit(EXIT_FAILURE);
    }
    close(fd);
}

// Function to check and read from circular buffer
void readBufferedOutput(struct BufferedSharedData* shared3, std::vector<std::string>& results, sem_t* sem3) {
    sem_wait(sem3);

    // Read all available entries
    while (shared3->entry_count > 0) {
        // Copy data from the current read position
        shared3->buffers[shared3->read_index].data[shared3->buffers[shared3->read_index].size] = '\0';
        results.push_back(std::string(shared3->buffers[shared3->read_index].data));

        // Update read index and count
        shared3->read_index = (shared3->read_index + 1) % BUFFER_COUNT;
        shared3->entry_count--;
    }

    msync(shared3, sizeof(struct BufferedSharedData), MS_SYNC);
    sem_post(sem3);
}

int main() {
    // Clean up any existing semaphores
    sem_unlink(SEM_NAME1);
    sem_unlink(SEM_NAME2);
    sem_unlink(SEM_NAME3);

    // Create semaphores
    sem_t *sem1 = sem_open(SEM_NAME1, O_CREAT | O_EXCL, 0666, 1);
    sem_t *sem2 = sem_open(SEM_NAME2, O_CREAT | O_EXCL, 0666, 1);
    sem_t *sem3 = sem_open(SEM_NAME3, O_CREAT | O_EXCL, 0666, 1);

    if (sem1 == SEM_FAILED || sem2 == SEM_FAILED || sem3 == SEM_FAILED) {
        perror("Error creating semaphores");
        exit(EXIT_FAILURE);
    }

    // Prepare memory mapped files
    prepareFileForMapping(MAPPED_FILE1, sizeof(struct SharedData));
    prepareFileForMapping(MAPPED_FILE2, sizeof(struct SharedData));
    prepareFileForMapping(MAPPED_FILE3, sizeof(struct BufferedSharedData));

    // Open the memory mapped files
    int fd1 = open(MAPPED_FILE1, O_RDWR);
    int fd3 = open(MAPPED_FILE3, O_RDWR);

    if (fd1 == -1 || fd3 == -1) {
        perror("Error opening mapped files");
        exit(EXIT_FAILURE);
    }

    // Map the files into memory
    struct SharedData* shared1 = (struct SharedData*)mmap(NULL, sizeof(struct SharedData),
                                          PROT_READ | PROT_WRITE,
                                          MAP_SHARED, fd1, 0);
    struct BufferedSharedData* shared3 = (struct BufferedSharedData*)mmap(NULL, sizeof(struct BufferedSharedData),
                                          PROT_READ | PROT_WRITE,
                                          MAP_SHARED, fd3, 0);

    if (shared1 == MAP_FAILED || shared3 == MAP_FAILED) {
        perror("Error mapping files");
        exit(EXIT_FAILURE);
    }

    // Initialize shared memory
    shared1->size = 0;
    shared1->done = false;
    shared3->write_index = 0;
    shared3->read_index = 0;
    shared3->entry_count = 0;
    shared3->done = false;

    // Create first child process
    pid_t child1 = fork();
    if (child1 == -1) {
        perror("Error creating first child process");
        exit(EXIT_FAILURE);
    }

    if (child1 == 0) {
        // First child process
        execl("./child1", "child1", NULL);
        perror("Error executing child1");
        exit(EXIT_FAILURE);
    }

    // Create second child process
    pid_t child2 = fork();
    if (child2 == -1) {
        perror("Error creating second child process");
        exit(EXIT_FAILURE);
    }

    if (child2 == 0) {
        // Second child process
        execl("./child2", "child2", NULL);
        perror("Error executing child2");
        exit(EXIT_FAILURE);
    }

    // Parent process continues here
    printf("Введите строки (Ctrl+D для завершения):\n");

    // Store all results for later printing
    std::vector<std::string> results;

    char line[MAX_LINE];
    while (fgets(line, MAX_LINE, stdin) != NULL) {
        // Send data to child1 through first shared memory
        sem_wait(sem1);
        size_t len = strlen(line);
        strncpy(shared1->data, line, len);
        shared1->size = len;
        msync(shared1, sizeof(struct SharedData), MS_SYNC);
        sem_post(sem1);

        // readBufferedOutput(shared3, results, sem3);
    }

    // Signal end of input
    sem_wait(sem1);
    shared1->done = true;
    msync(shared1, sizeof(struct SharedData), MS_SYNC);
    sem_post(sem1);

    // Check for remaining output
    readBufferedOutput(shared3, results, sem3);

    // Wait for child processes to complete
    waitpid(child1, NULL, 0);
    waitpid(child2, NULL, 0);

    // Print all collected results
    printf("Преобразованный текст:\n");
    for (const auto& result : results) {
        printf("%s", result.c_str());
    }

    // Clean up
    munmap(shared1, sizeof(struct SharedData));
    munmap(shared3, sizeof(struct BufferedSharedData));
    close(fd1);
    close(fd3);

    sem_close(sem1);
    sem_close(sem2);
    sem_close(sem3);
    sem_unlink(SEM_NAME1);
    sem_unlink(SEM_NAME2);
    sem_unlink(SEM_NAME3);

    unlink(MAPPED_FILE1);
    unlink(MAPPED_FILE2);
    unlink(MAPPED_FILE3);

    printf("\nВсе процессы завершены.\n");
    return 0;
}
