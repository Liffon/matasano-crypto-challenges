#ifndef CRYPTO_H
#define CRYPTO_H

#include "matasano.h"
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>

static bool openssl_inited;

void openssl_init() {
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    OPENSSL_config(NULL);
    openssl_inited = true;
}

void openssl_cleanup() {
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
    ERR_free_strings();
}

void handle_openssl_errors() {
    ERR_print_errors_fp(stderr);
    abort();
}

buffer *aes_128_ecb_encrypt(buffer *plaintext, const char *key) {
    assert(plaintext);
    //assert(plaintext->length % 16 == 0);
    if(!openssl_inited) {
        openssl_init();
        atexit(openssl_cleanup);
    }
    EVP_CIPHER_CTX *ctx;

    buffer *ciphertext = allocate_buffer(plaintext->length);

    int chunk_length, total_length;
    if(!(ctx = EVP_CIPHER_CTX_new())) {
        handle_openssl_errors();
    }
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), NULL,
                               (const unsigned char *)key, NULL)) {
        handle_openssl_errors();
    }
    EVP_CIPHER_CTX_set_padding(ctx, 0);

    if(1 != EVP_EncryptUpdate(ctx, ciphertext->bytes, &chunk_length,
                              plaintext->bytes, plaintext->length)) {
        handle_openssl_errors();
    }
    total_length = chunk_length;

    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext->bytes + chunk_length, &chunk_length)) {
        handle_openssl_errors();
    }
    total_length += chunk_length;

    ciphertext = resize_buffer(ciphertext, total_length);

    EVP_CIPHER_CTX_free(ctx);

    return ciphertext;
}

buffer *aes_128_ecb_decrypt(buffer *ciphertext, const char *key, int padded = 0) {
    assert(ciphertext);
    if(!openssl_inited) {
        openssl_init();
        atexit(openssl_cleanup);
    }
    EVP_CIPHER_CTX *ctx;

    buffer *plaintext = allocate_buffer(ciphertext->length + 16);
    memset(plaintext->bytes, 0, plaintext->length);

    int chunk_length, total_length;
    if(!(ctx = EVP_CIPHER_CTX_new())) {
        handle_openssl_errors();
    }
    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_ecb(), NULL,
                               (const unsigned char *)key, NULL)) {
        handle_openssl_errors();
    }
    EVP_CIPHER_CTX_set_padding(ctx, padded);

    if(1 != EVP_DecryptUpdate(ctx, plaintext->bytes, &chunk_length,
                              ciphertext->bytes, ciphertext->length)) {
        handle_openssl_errors();
    }
    total_length = chunk_length;

    if(1 != EVP_DecryptFinal_ex(ctx, plaintext->bytes + chunk_length, &chunk_length)) {
        handle_openssl_errors();
    }
    total_length += chunk_length;
    assert(total_length > 0);

    plaintext = resize_buffer(plaintext, total_length);
    EVP_CIPHER_CTX_free(ctx);

    return plaintext;
}

buffer *aes_128_cbc_encrypt(buffer *plaintext, const char *key, const char *iv) {
    assert(plaintext);

    buffer *ciphertext_block = allocate_buffer(16);
    memcpy(ciphertext_block->bytes, iv, 16);

    plaintext = pkcs7_pad_to_multiple_of(plaintext, 16);
    buffer *ciphertext = allocate_buffer(plaintext->length);

    buffer *plaintext_block = allocate_buffer(16);

    for(size_t index = 0;
        index < plaintext->length;
        index += 16)
    {
        plaintext_block = cut_buffer(plaintext, index, 16, plaintext_block);
        xor_buffers_into_first(plaintext_block, ciphertext_block);

        ciphertext_block = aes_128_ecb_encrypt(plaintext_block, key);
        memcpy(&ciphertext->bytes[index], ciphertext_block->bytes, 16);
    }

    free(plaintext_block);
    free(ciphertext_block);

    return ciphertext;
}

buffer *aes_128_cbc_decrypt(buffer *ciphertext, const char *key, const char *iv) {
    assert(ciphertext);

    buffer *ciphertext_block = allocate_buffer(16);
    buffer *previous_ciphertext_block = allocate_buffer(16);
    memcpy(previous_ciphertext_block->bytes, iv, 16);

    buffer *plaintext = allocate_buffer(ciphertext->length);
    buffer *plaintext_block = allocate_buffer(16);

    int round = 0;
    for(size_t index = 0;
        index < ciphertext->length;
        index += 16)
    {
        ciphertext_block = cut_buffer(ciphertext, index, 16, ciphertext_block);
        plaintext_block = aes_128_ecb_decrypt(ciphertext_block, key);

        xor_buffers_into_first(plaintext_block, previous_ciphertext_block);
        memcpy(&plaintext->bytes[index], plaintext_block->bytes, 16);

        // Swap the blocks
        buffer *tmp = previous_ciphertext_block;
        previous_ciphertext_block = ciphertext_block;
        ciphertext_block = tmp;
    }

    // Remove padding
    plaintext = resize_buffer(plaintext, plaintext->length - plaintext->bytes[plaintext->length - 1]);

    free(plaintext_block);
    free(ciphertext_block);
    free(previous_ciphertext_block);

    return plaintext;
}

#endif
