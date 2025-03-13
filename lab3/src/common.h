#ifndef COMMON_H
#define COMMON_H

#define MAX_LINE 1024
#define SHARED_MEM_SIZE (MAX_LINE * 100)
#define BUFFER_COUNT 30  // Number of buffer entries in mmf3
#define MAPPED_FILE1 "/tmp/mapped_file1"
#define MAPPED_FILE2 "/tmp/mapped_file2"
#define MAPPED_FILE3 "/tmp/mapped_file3" // For output back to parent
#define SEM_NAME1 "/sem1_lab"
#define SEM_NAME2 "/sem2_lab"
#define SEM_NAME3 "/sem3_lab"

// Standard SharedData for mmf1 and mmf2
struct SharedData {
    char data[SHARED_MEM_SIZE];
    size_t size;
    bool done;
};

// Enhanced SharedData for mmf3 with circular buffer
struct BufferedSharedData {
    struct {
        char data[MAX_LINE];
        size_t size;
    } buffers[BUFFER_COUNT];

    int write_index;  // Where child2 will write next
    int read_index;   // Where parent will read next
    int entry_count;  // Current number of entries
    bool done;
};

#endif
