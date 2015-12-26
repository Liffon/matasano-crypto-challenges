#include "matasano.h"
#include "crypto.h"

int main(int argc, char *argv[]) {
    const char *input_filename = "2-10-input.txt";
    const char *key = "YELLOW SUBMARINE";
    const char *iv = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    if(argc >= 2) {
        input_filename = argv[1];
    }
    if(argc >= 3) {
        key = argv[2];
    }

    buffer *raw_ciphertext = read_file(input_filename);
    buffer *ciphertext = base64_decode(raw_ciphertext);
    free(raw_ciphertext);

    buffer *plaintext = aes_128_cbc_decrypt(ciphertext, key, iv);

    literally_print_buffer(plaintext);

    free(ciphertext);
    free(plaintext);

    return 0;
}
