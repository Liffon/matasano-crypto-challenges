#include "matasano.h"
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>

void handle_errors() {
    ERR_print_errors_fp(stderr);
    abort();
}

buffer *aes_128_ecb_decrypt(buffer *ciphertext, const char *key) {
    assert(ciphertext);
    EVP_CIPHER_CTX *ctx;

    buffer *plaintext = allocate_buffer(ciphertext->length);

    int len;
    if(!(ctx = EVP_CIPHER_CTX_new())) {
        handle_errors();
    }
    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_ecb(), NULL,
                               (const unsigned char *)key, NULL)) {
        handle_errors();
    }

    if(1 != EVP_DecryptUpdate(ctx, plaintext->bytes, &len,
                              ciphertext->bytes, ciphertext->length)) {
        handle_errors();
    }
    plaintext->length = len;

    if(1 != EVP_DecryptFinal_ex(ctx, plaintext->bytes + len, &len)) {
        handle_errors();
    }
    plaintext->length += len;
    EVP_CIPHER_CTX_free(ctx);

    return plaintext;
}

int main(int argc, char *argv[]) {
    const char *input_filename = "1-7-input.txt";
    const char *key = "YELLOW SUBMARINE";
    if(argc >= 2) {
        input_filename = argv[1];
    }
    if(argc >= 3) {
        key = argv[2];
    }

    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    OPENSSL_config(NULL);

	buffer *raw_ciphertext = read_file(input_filename);
	buffer *ciphertext = base64_decode(raw_ciphertext);
	free(raw_ciphertext);

    buffer *plaintext = aes_128_ecb_decrypt(ciphertext, key);

    literally_print_buffer(plaintext);

    free(ciphertext);
    free(plaintext);

    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
    ERR_free_strings();
    return 0;
}