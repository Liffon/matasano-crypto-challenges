#include "matasano.h"

int main(int argc, char **argv) {
    int read_hex = 1;
    int encode = 1;

    size_t input_length;
    byte *input = read_until_eof(&input_length);

    size_t parsed_length;
    byte *parsed;
    if(read_hex) {
        parsed = parse_hex_buffer(input, input_length, &parsed_length);
    } else {
        parsed = input;
        parsed_length = input_length;
    }

    size_t result_length;
    byte *result;
    if(encode) {
        result = base64_encode(parsed, parsed_length, &result_length);
    } else {
        result = parsed;
        result_length = parsed_length;
    }
    print_buffer(result, result_length);

    free(result);
    
    return 0;
}
