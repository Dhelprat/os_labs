#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include "common.h"

int main() {
    // Open semaphores
    sem_t *sem1 = sem_open(SEM_NAME1, 0);
    sem_t *sem2 = sem_open(SEM_NAME2, 0);
    if (sem1 == SEM_FAILED || sem2 == SEM_FAILED) {
        perror("Error opening semaphores");
        return EXIT_FAILURE;
    }

    // Open first mapped file (for input from parent)
    int fd1 = open(MAPPED_FILE1, O_RDWR);
    if (fd1 == -1) {
        perror("Error opening first mapped file");
        return EXIT_FAILURE;
    }

    // Open second mapped file (for output to child2)
    int fd2 = open(MAPPED_FILE2, O_RDWR);
    if (fd2 == -1) {
        perror("Error opening second mapped file");
        close(fd1);
        return EXIT_FAILURE;
    }

    // Map the files into memory
    struct SharedData* shared1 = (struct SharedData*)mmap(NULL, sizeof(struct SharedData),
                                         PROT_READ | PROT_WRITE,
                                         MAP_SHARED, fd1, 0);
    struct SharedData* shared2 = (struct SharedData*)mmap(NULL, sizeof(struct SharedData),
                                         PROT_READ | PROT_WRITE,
                                         MAP_SHARED, fd2, 0);
    if (shared1 == MAP_FAILED || shared2 == MAP_FAILED) {
        perror("Error mapping files");
        close(fd1);
        close(fd2);
        return EXIT_FAILURE;
    }

    // Initialize the second shared memory
    shared2->size = 0;
    shared2->done = false;
    msync(shared2, sizeof(struct SharedData), MS_SYNC);

    char buffer[MAX_LINE];

    // Process data until parent signals completion
    while (true) {
        bool is_done = false;
        bool has_data = false;

        // Check for new input data
        sem_wait(sem1);
        if (shared1->size > 0) {
            // Read data from parent
            strncpy(buffer, shared1->data, shared1->size);
            buffer[shared1->size] = '\0';
            shared1->size = 0;  // Mark as read
            msync(shared1, sizeof(struct SharedData), MS_SYNC);
            has_data = true;
        }

        is_done = shared1->done;
        sem_post(sem1);

        // Process the data if we got any
        if (has_data) {
            // Convert to lowercase
            for (size_t i = 0; buffer[i]; i++) {
                buffer[i] = tolower(buffer[i]);
            }

            // Send processed data to child2
            sem_wait(sem2);
            size_t len = strlen(buffer);
            strncpy(shared2->data, buffer, len);
            shared2->size = len;
            msync(shared2, sizeof(struct SharedData), MS_SYNC);
            sem_post(sem2);
        }

        // Check if we're done
        if (is_done && !has_data) {
            // Signal child2 that we're done
            sem_wait(sem2);
            shared2->done = true;
            msync(shared2, sizeof(struct SharedData), MS_SYNC);
            sem_post(sem2);
            break;
        }
    }

    // Clean up
    munmap(shared1, sizeof(struct SharedData));
    munmap(shared2, sizeof(struct SharedData));
    close(fd1);
    close(fd2);
    sem_close(sem1);
    sem_close(sem2);

    return 0;
}
