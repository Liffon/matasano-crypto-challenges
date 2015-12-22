#include "matasano.h"

int main(int argc, char *argv[]) {
    const char *input_filename = "1-5-input.txt";
    const char *key = "ICE";
    size_t key_length = 3;
    bool output_base64 = false;
    if(argc >= 2) {
        input_filename = argv[1];
    }
    if(argc >= 4) {
        key = argv[2];
        key_length = atoi(argv[3]);
    }
    if(argc >= 5) {
        output_base64 = true;
    }

    buffer *plaintext = read_file(input_filename);
    if(!plaintext) {
        fprintf(stderr, "Unable to open file '%s'\n", input_filename);
        return 1;
    }

    buffer *long_key = allocate_buffer(plaintext->length);
    fill_buffer_repeating(long_key, (byte *)key, key_length);

    buffer *ciphertext = xor_buffers(plaintext, long_key);
    if(output_base64) {
        base64_print_buffer(ciphertext);
    }else {
        hex_print_buffer(ciphertext);
    }
    putchar('\n');

    free(plaintext);
    free(ciphertext);
    return 0;
}