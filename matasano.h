#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "buffer.h"
#include "letters.h"

const char *base64values = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

struct base256bytes {
    byte bytes[3];
};

struct base64bytes {
    byte bytes[4];
};

base64bytes base64_chunk(base256bytes input_chunk, int bytes = 3) {
    assert(bytes >= 1);
    assert(bytes <= 3);

    if(bytes < 3) {
        input_chunk.bytes[2] = 0;
    }
    if(bytes < 2) {
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

    if(bytes < 3) {
        result.bytes[3] = '=';
    }
    if(bytes < 2) {
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
        base64bytes result = base64_chunk(input_chunk, bytes_in_chunk);

        output->bytes[result_index++] = result.bytes[0];
        output->bytes[result_index++] = result.bytes[1];
        output->bytes[result_index++] = result.bytes[2];
        output->bytes[result_index++] = result.bytes[3];
    }

    return output;
}

buffer *parse_hex_buffer(const buffer *hex) {
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
