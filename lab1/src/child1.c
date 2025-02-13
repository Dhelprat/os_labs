#include <stdio.h>
#include <ctype.h>

int main() {
    char buffer[1024];
    int c;

    while ((c = getchar()) != EOF) {
        putchar(tolower(c));
    }

    return 0;
}
