#include "matasano.h"

int main() {
    buffer *input = read_hex_until_eol();
    input = pkcs7_pad(input, input->length + 20 - (input->length % 20));

    hex_print_buffer(input);
    return 0;
}