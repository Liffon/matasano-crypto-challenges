#ifndef MATASANO_H
#define MATASANO_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "buffer.h"
#include "letters.h"

const char base64values[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

struct base256bytes {
    byte bytes[3];
    int significant_bytes;
};

struct base64bytes {
    byte bytes[4];
};

base64bytes base256_to_base64(base256bytes input_chunk) {
    assert(input_chunk.significant_bytes >= 1);
    assert(input_chunk.significant_bytes <= 3);

    if(input_chunk.significant_bytes < 3) {
        input_chunk.bytes[2] = 0;
    }
    if(input_chunk.significant_bytes < 2) {
        input_chunk.bytes[1] = 0;
    }

    base64bytes result;
    result.bytes[0] = input_chunk.bytes[0] >> 2;
    result.bytes[1] = ((input_chunk.bytes[0] & 3) << 4)
                     | (input_chunk.bytes[1] >> 4);
    result.bytes[2] = ((input_chunk.bytes[1] & 15) << 2)
                     | (input_chunk.bytes[2] >> 6);
    result.bytes[3] = input_chunk.bytes[2] & 63;

    result.bytes[0] = base64values[result.bytes[0]];
    result.bytes[1] = base64values[result.bytes[1]];
    result.bytes[2] = base64values[result.bytes[2]];
    result.bytes[3] = base64values[result.bytes[3]];

    if(input_chunk.significant_bytes < 3) {
        result.bytes[3] = '=';
    }
    if(input_chunk.significant_bytes < 2) {
        result.bytes[2] = '=';
    }

    return result;
}

buffer *base64_encode(buffer *input) {
    size_t output_length = (input->length + 2) / 3 * 4; // input->length / 3 rounded up * 4
    buffer *output = allocate_buffer(output_length);

    if(!output) {
        return NULL;
    }

    size_t result_index = 0;
    for(size_t byte_index = 0;
        byte_index < input->length;
        byte_index += 3)
    {
        base256bytes input_chunk;
        input_chunk.bytes[0] = input->bytes[byte_index];
        input_chunk.bytes[1] = input->bytes[byte_index + 1];
        input_chunk.bytes[2] = input->bytes[byte_index + 2];

        int bytes_left = input->length - byte_index;
        int bytes_in_chunk = bytes_left < 3 ? bytes_left : 3;

        input_chunk.significant_bytes = bytes_in_chunk;
        base64bytes result = base256_to_base64(input_chunk);

        output->bytes[result_index++] = result.bytes[0];
        output->bytes[result_index++] = result.bytes[1];
        output->bytes[result_index++] = result.bytes[2];
        output->bytes[result_index++] = result.bytes[3];
    }

    return output;
}

base256bytes base64_to_base256(base64bytes input_chunk) {
    // 000000 111111 222222 333333
    // aaaaaa aabbbb bbbbcc cccccc

    base256bytes result = {};

    if(input_chunk.bytes[2] == '=') {
        result.significant_bytes = 1;
    } else if(input_chunk.bytes[3] == '=') {
        result.significant_bytes = 2;
    } else {
        result.significant_bytes = 3;
    }

    for(size_t byte_index = 0;
        byte_index < 4 && input_chunk.bytes[byte_index] != '=';
        byte_index++)
    {
        for(size_t raw_index = 0;
            raw_index < (sizeof(base64values) / sizeof(base64values[0]) - 1);
            raw_index++)
        {
            if(input_chunk.bytes[byte_index] == base64values[raw_index]) {
                input_chunk.bytes[byte_index] = raw_index;
                break;
            } else if(raw_index >= sizeof(base64values) / sizeof(base64values[0]) - 1) {
                assert(0);
            }
        }
    }

    result.bytes[0] = (input_chunk.bytes[0] << 2) | ((input_chunk.bytes[1] >> 4) & 3);
    if(result.significant_bytes >= 2) {
        result.bytes[1] = (input_chunk.bytes[1] << 4) | ((input_chunk.bytes[2] >> 2) & 15);
    }
    if(result.significant_bytes == 3) {
        result.bytes[2] = (input_chunk.bytes[2] << 6) | (input_chunk.bytes[3] & 63);
    }

    return result;
}

buffer *base64_decode(buffer *raw_input) {
    buffer *input = remove_whitespace(copy_buffer(raw_input));

    assert(input->length % 4 == 0);
    size_t output_length = input->length / 4 * 3;
    buffer *output = allocate_buffer(output_length);

    if(!output) {
        return NULL;
    }

    base64bytes input_chunk;

    size_t result_index = 0;
    for(size_t input_index = 0;
        input_index < input->length;
        input_index += 4)
    {
        memcpy(input_chunk.bytes, &input->bytes[input_index], 4);

        base256bytes result = base64_to_base256(input_chunk);

        assert(result.significant_bytes <= 3);
        assert(result.significant_bytes >= 1);
        memcpy(&output->bytes[input_index / 4 * 3], result.bytes, result.significant_bytes);

        if(result.significant_bytes < 3) {
            output = resize_buffer(output, output->length + result.significant_bytes - 3);
            break;
        }
    }

    free(input);
    return output;
}

buffer *parse_hex_buffer(const buffer *raw_hex) {
    buffer *hex = remove_whitespace(copy_buffer(hex));

    buffer *result = allocate_buffer(hex->length / 2);
    if(!result) {
        return NULL;
    }

    // sscanf needs a null-terminated string apparently.
    char *temp_string = (char *)malloc(hex->length + 1);
    if(!temp_string) {
        free(result);
        return NULL;
    }
    memcpy(temp_string, hex->bytes, hex->length);
    temp_string[hex->length] = 0;

    for(size_t index = 0;
        index < result->length;
        index++)
    {
        int size_read = sscanf(&temp_string[2*index], "%2" SCNx8, &result->bytes[index]);
        if(size_read == 0) {
            fprintf(stderr, "Invalid hex character. Aborting!\n");
            result->length = 0;
            return NULL;
        }
    }
    free(temp_string);
    free(hex);

    return result;
}

buffer *read_hex_until_eol(FILE *fd = stdin) {
    buffer *input = read_until_eol(fd);

    if(input) {
        buffer *result = parse_hex_buffer(input);
        free(input);
        return result;
    } else {
        return input;
    }
}

buffer* xor_buffers(buffer *one, buffer *two) {
    if(!one || !two) {
        return NULL;
    }
    assert(one->length == two->length);

    buffer *result = allocate_buffer(one->length);
    if(!result) {
        return result;
    }
    for(size_t i = 0;
        i < result->length;
        i++)
    {
        result->bytes[i] = one->bytes[i] ^ two->bytes[i];
    }

    return result;
}

float find_best_single_byte_xor_score_with_distribution(buffer *ciphertext, byte *best_key_out, buffer **best_plaintext_out, bool log = false) {
    buffer *expanded_key = allocate_buffer(ciphertext->length);
    buffer *plaintext[256];

    float scores[256];
    float best_score = INFINITY;
    byte best_key = 0;
    for(int index = 0;
        index < 256;
        index++)
    {
        byte key = (byte)index;
        for(size_t index = 0;
            index < expanded_key->length;
            index++)
        {
            expanded_key->bytes[index] = key;
        }

        plaintext[key] = xor_buffers(ciphertext, expanded_key);
        scores[key] = score_plaintext_with_distribution(plaintext[key]);


        if(scores[key] < best_score) {
            best_score = scores[key];
            best_key = key;

            if(log) {
                printf("Key 0x%02x gave \"", key);
                print_buffer(plaintext[key]);
                printf("\" with score %f\n", best_score);
            }
        }
    }

    if(best_key_out) {
        *best_key_out = best_key;
    }
    if(best_plaintext_out) {
        *best_plaintext_out = plaintext[best_key];
    }

    for(int key = 0;
        key <= 255;
        key++)
    {
        if(!best_plaintext_out || key != best_key) {
            free(plaintext[key]);
        }
    }
    free(expanded_key);

    return best_score;
}

long int hamming_distance(buffer *one, buffer *two) {
    buffer *difference = xor_buffers(one, two);
    if(!difference) {
        return -1;
    }

    long int result = 0;
    for(size_t index = 0;
        index < difference->length;
        index++)
    {
        for(int i = 0;
            i < 8;
            i++)
        {
            result += (difference->bytes[index] >> i) & 1;
        }
    }
    free(difference);
    return result;
}
#endif