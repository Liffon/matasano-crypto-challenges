#include <stdio.h>
#include "matasano.h"

int main() {
    printf("First hex string:\n");

    size_t one_length;
    char *one = read_hex_until_eof(&one_length);

    printf("Second hex string:\n");

    size_t two_length;
    char *two = read_hex_until_eof(&two_length);

    char *result = xor_buffers(one, one_length, two, two_length);

    printf("Xor of them:\n");
    for(int i = 0;
        i < one_length;
        i++)
    {
        printf("%2hhx", result[i]);
    }
}
