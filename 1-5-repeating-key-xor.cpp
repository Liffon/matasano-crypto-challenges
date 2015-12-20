#include "matasano.h"

int main(int argc, char *argv[]) {
    const char *input_filename = "1-5-input.txt";
    const char *key = "ICE";
    size_t key_length = 3;
    if(argc >= 2) {
        input_filename = argv[1];
    }
    if(argc >= 4) {
        key = argv[2];
        key_length = atoi(argv[3]);
    }

    buffer *plaintext = read_file(input_filename);
    if(!plaintext) {
        fprintf(stderr, "Unable to open file '%s'\n", input_filename);
        return 1;
    }

    buffer *long_key = allocate_buffer(plaintext->length);
    for(size_t index = 0;
        index < long_key->length;
        index++)
    {
        long_key->bytes[index] = key[index % key_length];
    }

    buffer *ciphertext = xor_buffers(plaintext, long_key);
    hex_print_buffer(ciphertext);
    putchar('\n');

    free(plaintext);
    return 0;
}