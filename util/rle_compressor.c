#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    int current_byte;
    int previous_byte = EOF;
    int error_occured;

    if (argc != 1) {
        fprintf(stderr, "Syntax: %s\n", argv[0]);
        return EXIT_FAILURE;
    }

    while ((current_byte = getchar()) != EOF) {
        putchar(current_byte);

        if (current_byte == previous_byte) {
            unsigned int run_length = 0;

            while ((current_byte = getchar()) == previous_byte && run_length < 255) {
                ++run_length;
            }

            putchar(run_length);
            if (current_byte != EOF) {
                putchar(current_byte);
            }
        }

        previous_byte = current_byte;
    }

    error_occured = ferror(stdin) || ferror(stdout);

    if (error_occured) {
        perror("An I/O error occured");
    }

    return error_occured;
}
