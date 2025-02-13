#include <stdio.h>
#include <stdbool.h>

int main() {
    char buffer[1024];
    int c;
    bool last_was_space = false;

    while ((c = getchar()) != EOF) {
        if (c == ' ' || c == '\t') {
            if (!last_was_space) {
                putchar(' ');
                last_was_space = true;
            }
        } else {
            putchar(c);
            last_was_space = false;
        }
    }

    return 0;
}
