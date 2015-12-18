#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef uint8_t byte;

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

byte *read_until_eof(size_t *input_length) {
    const size_t chunk_size = 4096;
    *input_length = chunk_size;
    byte *buffer = (byte *) malloc(*input_length);
    if(!buffer) {
        *input_length = 0;
        return NULL;
    }

    size_t bytes_read = 0;
    size_t bytes_read_in_chunk;
    while(!feof(stdin)) {
        bytes_read_in_chunk = fread(buffer + bytes_read, 1, chunk_size, stdin);
        bytes_read += bytes_read_in_chunk;

        if(bytes_read_in_chunk == chunk_size && !feof(stdin)) {
            *input_length += chunk_size;
            buffer = (byte *) realloc((void *) buffer, *input_length);
        }
        else if(ferror(stdin)) {
            free(buffer);
            return NULL;
        }
    }

    *input_length = bytes_read;
    buffer = (byte *) realloc((void *) buffer, *input_length);

    return buffer;
}

byte *read_until_eol(size_t *input_length) {
    const size_t chunk_size = 1;
    *input_length = chunk_size;
    byte *buffer = (byte *) malloc(*input_length);
    if(!buffer) {
        *input_length = 0;
        return NULL;
    }

    size_t bytes_read = 0;
    size_t bytes_read_in_chunk;
    bool found_eol = false;
    size_t eol_index = 0;
    while(!feof(stdin)) {
        bytes_read_in_chunk = fread(buffer + bytes_read, 1, chunk_size, stdin);
        bytes_read += bytes_read_in_chunk;

        for(; eol_index < bytes_read; eol_index++) {
            if(buffer[eol_index] == '\n') {
                found_eol = true;
                break;
            }
        }
        if(found_eol) {
            break;
        }

        if(bytes_read_in_chunk == chunk_size && !feof(stdin)) {
            *input_length += chunk_size;
            buffer = (byte *) realloc((void *) buffer, *input_length);
        }
        else if(ferror(stdin)) {
            free(buffer);
            return NULL;
        }
    }

    if(!found_eol) {
        free(buffer);
        *input_length = 0;
        return NULL;
    }

    *input_length = eol_index;
    buffer = (byte *) realloc((void *) buffer, *input_length);

    return buffer;
}

byte *parse_hex_buffer(byte *hex_buffer, size_t hex_length, size_t *result_length) {
    *result_length = hex_length / 2;
    byte *buffer = (byte *) malloc(*result_length);
    if(!buffer) {
        *result_length = 0;
        return NULL;
    }

    for(size_t index = 0;
        index < *result_length;
        index++)
    {
        if(0 == sscanf(hex_buffer, "%2hhx", &buffer[index])) {
            fprintf(stderr, "Invalid hex byteacter. Aborting!\n");
            *result_length = 0;
            return NULL;
        } else {
        hex_buffer += 2;
        }
    }

    return buffer;
}

byte *read_hex_until_eof(size_t *result_length) {
    size_t input_length;
    byte *input = read_until_eof(&input_length);

    byte *result;
    result = parse_hex_buffer(input, input_length, result_length);

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

void print_buffer(byte *buffer, size_t length) {
    for(size_t i = 0;
        i < length;
        i++)
    {
        putchar(buffer[i]);
    }

}

void hex_print_buffer(byte *buffer, size_t length) {
    for(size_t i = 0;
        i < length;
        i++)
    {
        printf("%x", buffer[i]);
    }
}