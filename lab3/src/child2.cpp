#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include "common.h"

int main() {
    // Open semaphores
    sem_t *sem2 = sem_open(SEM_NAME2, 0);
    sem_t *sem3 = sem_open(SEM_NAME3, 0);
    if (sem2 == SEM_FAILED || sem3 == SEM_FAILED) {
        perror("Error opening semaphores");
        return EXIT_FAILURE;
    }

    // Open second mapped file (for input from child1)
    int fd2 = open(MAPPED_FILE2, O_RDWR);
    if (fd2 == -1) {
        perror("Error opening mapped file");
        return EXIT_FAILURE;
    }

    // Open third mapped file (for output to parent)
    int fd3 = open(MAPPED_FILE3, O_RDWR);
    if (fd3 == -1) {
        perror("Error opening mapped file");
        close(fd2);
        return EXIT_FAILURE;
    }

    // Map the files into memory
    struct SharedData* shared2 = (struct SharedData*)mmap(NULL, sizeof(struct SharedData),
                                         PROT_READ | PROT_WRITE,
                                         MAP_SHARED, fd2, 0);
    struct BufferedSharedData* shared3 = (struct BufferedSharedData*)mmap(NULL, sizeof(struct BufferedSharedData),
                                         PROT_READ | PROT_WRITE,
                                         MAP_SHARED, fd3, 0);

    if (shared2 == MAP_FAILED || shared3 == MAP_FAILED) {
        perror("Error mapping files");
        close(fd2);
        close(fd3);
        return EXIT_FAILURE;
    }

    // Initialize shared memory for output to parent
    sem_wait(sem3);
    shared3->write_index = 0;
    shared3->read_index = 0;
    shared3->entry_count = 0;
    shared3->done = false;
    msync(shared3, sizeof(struct BufferedSharedData), MS_SYNC);
    sem_post(sem3);

    char buffer[MAX_LINE];
    char output[MAX_LINE];

    // Process data until child1 signals completion
    while (true) {
        bool is_done = false;
        bool has_data = false;

        // Check for new input data from child1
        sem_wait(sem2);
        if (shared2->size > 0) {
            // Read data from child1
            strncpy(buffer, shared2->data, shared2->size);
            buffer[shared2->size] = '\0';
            shared2->size = 0;  // Mark as read
            msync(shared2, sizeof(struct SharedData), MS_SYNC);
            has_data = true;
        }

        is_done = shared2->done;
        sem_post(sem2);

        // Process the data if we got any
        if (has_data) {
            // Normalize whitespace
            bool last_was_space = false;
            size_t j = 0;

            for (size_t i = 0; buffer[i]; i++) {
                if (buffer[i] == ' ' || buffer[i] == '\t') {
                    if (!last_was_space) {
                        output[j++] = ' ';
                        last_was_space = true;
                    }
                } else {
                    output[j++] = buffer[i];
                    last_was_space = false;
                }
            }
            output[j] = '\0';

            // Send processed data to parent using circular buffer
            sem_wait(sem3);
//            // Wait if buffer is full
//            while (shared3->entry_count >= BUFFER_COUNT) {
//                sem_post(sem3);
//                usleep(1000); // Brief wait to avoid spin-lock
//                sem_wait(sem3);
//            }

            // Add to circular buffer
            size_t len = strlen(output);
            strncpy(shared3->buffers[shared3->write_index].data, output, len);
            shared3->buffers[shared3->write_index].size = len;

            // Update write index and count
            shared3->write_index = (shared3->write_index + 1) % BUFFER_COUNT;
            shared3->entry_count++;

            msync(shared3, sizeof(struct BufferedSharedData), MS_SYNC);
            sem_post(sem3);
        }

        // Check if we're done
        if (is_done && !has_data) {
            // Signal parent that we're done
            sem_wait(sem3);
            shared3->done = true;
            msync(shared3, sizeof(struct BufferedSharedData), MS_SYNC);
            sem_post(sem3);
            break;
        }
    }

    // Clean up
    munmap(shared2, sizeof(struct SharedData));
    munmap(shared3, sizeof(struct BufferedSharedData));
    close(fd2);
    close(fd3);
    sem_close(sem2);
    sem_close(sem3);

    return 0;
}
