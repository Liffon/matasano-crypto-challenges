#include "matasano.h"
#include "crypto.h"

int main() {
    buffer *input = read_until_eol();

    const char *iv = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

    buffer *ciphertext = aes_128_cbc_encrypt(input, "YELLOW SUBMARINE", iv);

    printf("Ciphertext is:\n");
    base64_print_buffer(ciphertext);

    buffer *plaintext = aes_128_cbc_decrypt(ciphertext, "YELLOW SUBMARINE", iv);
    
    printf("\nDecrypted text is:\n");
    literally_print_buffer(plaintext);
    putchar('\n');

    return 0;
}
