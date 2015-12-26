#include "matasano.h"
#include "crypto.h"

int main(int argc, char *argv[]) {
    const char *input_filename = "1-7-input.txt";
    const char *key = "YELLOW SUBMARINE";
    if(argc >= 2) {
        input_filename = argv[1];
    }
    if(argc >= 3) {
        key = argv[2];
    }

	buffer *raw_ciphertext = read_file(input_filename);
	buffer *ciphertext = base64_decode(raw_ciphertext);
	free(raw_ciphertext);

    buffer *plaintext = aes_128_ecb_decrypt(ciphertext, key, 1);

    literally_print_buffer(plaintext);

    free(ciphertext);
    free(plaintext);

    return 0;
}
