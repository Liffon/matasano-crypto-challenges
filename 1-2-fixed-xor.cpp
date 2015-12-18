#include <stdio.h>
#include "matasano.h"

int main() {
    printf("Enter two hex strings on separate lines:\n");

    buffer *one_hex = read_until_eol();
    buffer *one = parse_hex_buffer(one_hex);
    
    buffer *two_hex = read_until_eol();
    buffer *two = parse_hex_buffer(two_hex);

    buffer *xor_result = xor_buffers(one, two);

    hex_print_buffer(xor_result);
    putchar('\n');

    free(one_hex);
    free(one);
    free(two_hex);
    free(two);
    free(xor_result);
    return 0;
}
