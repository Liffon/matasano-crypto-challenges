#include "matasano.h"

int main(int argc, char **argv) {
    int read_hex = 1;
    int encode = 1;

    size_t input_length;
    char *input = read_until_eof(&input_length);

    size_t parsed_length;
    char *parsed;
    if(read_hex) {
        parsed = read_hex_buffer(input, input_length, &parsed_length);
    } else {
        parsed = input;
        parsed_length = input_length;
    }

    size_t result_length;
    char *result;
    if(encode) {
        result = base64_encode(parsed, parsed_length, &result_length);
    } else {
        result = parsed;
        result_length = parsed_length;
    }
    for(int i = 0;
        i < result_length;
        i++)
    {
        putchar(result[i]);
    }

    free(result);
    
    return 0;
}
