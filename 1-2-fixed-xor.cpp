#include <stdio.h>
#include "matasano.h"

int main() {
    printf("Enter two hex strings on separate lines:\n");

    size_t one_length, two_length;
    byte *one_hex = read_until_eol(&one_length);
    byte *one = parse_hex_buffer(one_hex, one_length, &one_length);
    putchar('\n');
    
    byte *two_hex = read_until_eol(&two_length);
    byte *two = parse_hex_buffer(two_hex, two_length, &two_length);
    putchar('\n');

    return 0;
}
