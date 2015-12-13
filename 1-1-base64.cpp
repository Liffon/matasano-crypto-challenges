#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

char *base64_encode(const char *input, size_t input_length, size_t *output_length) {
    assert(input_length >= 0);

    *output_length = (input_length + 2) / 3 * 4; // input_length / 3 rounded up * 4
    char *output = (char *) malloc(*output_length);
    if(!output) {
        *output_length = 0;
        return NULL;
    }

    int result_index = 0;
    for(int byte_index = 0;
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

#define CHUNK_SIZE 4096
char *read_until_eof(size_t *input_length) {
    *input_length = CHUNK_SIZE;
    char *buffer = (char *) malloc(*input_length);
    if(!buffer) {
        *input_length = 0;
        return NULL;
    }

    size_t bytes_read = 0;
    size_t bytes_read_in_chunk;
    while(!feof(stdin)) {
        bytes_read_in_chunk = fread(buffer + bytes_read, 1, CHUNK_SIZE, stdin);
        bytes_read += bytes_read_in_chunk;

        if(bytes_read_in_chunk == CHUNK_SIZE && !feof(stdin)) {
            *input_length += CHUNK_SIZE;
            buffer = (char *) realloc((void *) buffer, *input_length);
        }
        else if(ferror(stdin)) {
            free(buffer);
            return NULL;
        }
    }

    *input_length = bytes_read;
    buffer = (char *) realloc((void *) buffer, *input_length);

    return buffer;
}

char *read_hex_buffer(char *hex_buffer, size_t hex_length, size_t *result_length) {
    *result_length = hex_length / 2;
    char *buffer = (char *) malloc(*result_length);
    if(!buffer) {
        *result_length = 0;
        return NULL;
    }

    for(int index = 0;
        index < *result_length;
        index++)
    {
        if(0 == sscanf(hex_buffer, "%2hhx", &buffer[index])) {
            fprintf(stderr, "Invalid hex character. Aborting!\n");
            *result_length = 0;
            return NULL;
        } else {
        hex_buffer += 2;
        }
    }

    return buffer;
}

int main(int argc, char **argv) {
    int read_hex = 1;
    int encode = 1;

    size_t input_length;
    char *input = read_until_eof(&input_length);

    size_t parsed_length;
    char *parsed;
    if(read_hex) {
        parsed = read_hex_buffer(input, input_length, &parsed_length);
    } else {
        parsed = input;
        parsed_length = input_length;
    }

    size_t result_length;
    char *result;
    if(encode) {
        result = base64_encode(parsed, parsed_length, &result_length);
    } else {
        result = parsed;
        result_length = parsed_length;
    }
    for(int i = 0;
        i < result_length;
        i++)
    {
        putchar(result[i]);
    }

    free(result);
    
    return 0;
}
