#include "matasano.h"

int main() {
    buffer *input = read_hex_until_eol();
    buffer *result = base64_encode(input);

    print_buffer(result);

    free(input);
    free(result);
    
    return 0;
}
