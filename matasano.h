#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "buffer.h"

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

byte *base64_encode(const byte *input, size_t input_length, size_t *output_length) {
    *output_length = (input_length + 2) / 3 * 4; // input_length / 3 rounded up * 4
    byte *output = (byte *) malloc(*output_length);
    if(!output) {
        *output_length = 0;
        return NULL;
    }

    size_t result_index = 0;
    for(size_t byte_index = 0;
        byte_index < input_length;
        byte_index += 3)
    {
        base256bytes input_chunk;
        input_chunk.bytes[0] = input[byte_index];
        input_chunk.bytes[1] = input[byte_index + 1];
        input_chunk.bytes[2] = input[byte_index + 2];

        int bytes_left = input_length - byte_index;
        int bytes_in_chunk = bytes_left < 3 ? bytes_left : 3;
        base64bytes result = base64_chunk(input_chunk, bytes_in_chunk);

        output[result_index++] = result.bytes[0];
        output[result_index++] = result.bytes[1];
        output[result_index++] = result.bytes[2];
        output[result_index++] = result.bytes[3];
    }

    return output;
}

buffer *read_until_eof() {
    const size_t chunk_size = 4096;
    buffer *result = allocate_buffer(chunk_size);
    if(!result) {
        return NULL;
    }

    size_t bytes_read = 0;
    size_t bytes_read_in_chunk;
    while(!feof(stdin)) {
        bytes_read_in_chunk = fread(&result[0] + bytes_read, 1, chunk_size, stdin);
        bytes_read += bytes_read_in_chunk;

        if(bytes_read_in_chunk == chunk_size && !feof(stdin)) {
            result = resize_buffer(result, result->length + chunk_size);
        }
        else if(ferror(stdin)) {
            free(result);
            return NULL;
        }
    }

    result = resize_buffer(result, bytes_read);

    return result;
}

buffer *read_until_eol() {
    const size_t chunk_size = 1;
    buffer *result = allocate_buffer(chunk_size);
    if(!result) {
        return NULL;
    }

    size_t bytes_read = 0;
    size_t bytes_read_in_chunk;
    bool found_eol = false;
    size_t eol_index = 0;
    while(!feof(stdin)) {
        bytes_read_in_chunk = fread(&result->bytes[0] + bytes_read, 1, chunk_size, stdin);
        bytes_read += bytes_read_in_chunk;

        for(; eol_index < bytes_read; eol_index++) {
            if(result->bytes[eol_index] == '\n') {
                found_eol = true;
                break;
            }
        }
        if(found_eol) {
            break;
        }

        if(bytes_read_in_chunk == chunk_size && !feof(stdin)) {
            result = resize_buffer(result, result->length + chunk_size);
        }
        else if(ferror(stdin)) {
            free(result);
            return NULL;
        }
    }

    if(!found_eol) {
        free(result);
        return NULL;
    } else {
        result = resize_buffer(result, eol_index);
        return result;
    }
}

buffer *parse_hex_buffer(const buffer *hex) {
    buffer *result = allocate_buffer(hex->length / 2);
    if(!result) {
        return NULL;
    }

    for(size_t index = 0;
        index < result->length;
        index++)
    {
        int size_read = sscanf(
                (const char *)&hex->bytes[2 * index],
                "%2" SCNx8,
                &result->bytes[index]);
        if(size_read == 0) {
            fprintf(stderr, "Invalid hex character. Aborting!\n");
            result->length = 0;
            return NULL;
        }
    }

    return result;
}

buffer *read_hex_until_eof() {
    buffer *input = read_until_eof();

    buffer *result;
    result = parse_hex_buffer(input);

    free(input);
    return result;
}

byte *xor_buffers(byte *one, size_t one_length, byte *two, size_t two_length) {
    assert(one_length <= two_length);

    byte *result = (byte *) malloc(one_length);
    if(!result) {
        return NULL;
    }

    for(size_t i = 0;
        i < one_length;
        i++)
    {
        result[i] = one[i] ^ two[i];
    }

    return result;
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

void print_buffer(buffer *chars) {
    for(size_t i = 0;
        i < chars->length;
        i++)
    {
        putchar(chars->bytes[i]);
    }

}

void hex_print_buffer(buffer *chars) {
    for(size_t i = 0;
        i < chars->length;
        i++)
    {
        printf("%x", chars->bytes[i]);
    }
}
