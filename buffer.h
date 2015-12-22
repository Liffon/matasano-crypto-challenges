#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>
#include <inttypes.h>
#include "matasano.h"

typedef uint8_t byte;

struct buffer {
    size_t length;
    byte bytes[1];

    byte& operator[] (size_t);
};

buffer *base64_encode(buffer *input);

buffer *allocate_buffer(size_t length) {
    buffer *result = (buffer *)malloc(sizeof(size_t) + length);
    if(result) {
        result->length = length;
    }
    return result;
}

buffer *resize_buffer(buffer *old_buffer, size_t length) {
    if(length) {
        buffer *result = (buffer *)realloc(old_buffer, sizeof(size_t) + length);
        if(result) {
            result->length = length;
        }
        return result;
    }
    else {
        free(old_buffer);
        return NULL;
    }
}

buffer *copy_buffer(const buffer *old_buffer) {
    size_t size = sizeof(size_t) + old_buffer->length;
    buffer *result = (buffer *)malloc(size);
    if(!result) {
        return NULL;
    }
    memcpy(result, old_buffer, size);

    return result;
}

buffer *cut_buffer(const buffer *source, size_t start, size_t length, buffer *destination = 0) {
    if(!destination) {
        destination = allocate_buffer(length);
    } else if(destination->length != length) {
        destination = resize_buffer(destination, length);
    }
    if(!destination || !source) {
        return NULL;
    }

    memcpy(destination->bytes, source->bytes + start, length);
    return destination;
}

buffer *take_every_nth_byte(const buffer *source, size_t start, size_t interval) {
    if(!source) {
        return NULL;
    }

    // length / interval rounded up
    size_t result_length = (source->length + interval - 1) / interval;
    buffer *result = allocate_buffer(result_length);
    if(!result) {
        return NULL;
    }

    byte *write_destination = result->bytes;
    for(size_t i = start;
        i < source->length;
        i += interval)
    {
        *(write_destination++) = source->bytes[i];
    }

    result = resize_buffer(result, write_destination - result->bytes);
    return result;
}

buffer *read_until_eol(FILE *fd = stdin) {
    const size_t chunk_size = 1;
    buffer *result = allocate_buffer(chunk_size);
    if(!result) {
        return NULL;
    }

    size_t bytes_read = 0;
    size_t bytes_read_in_chunk;
    bool found_eol = false;
    size_t eol_index = 0;
    while(!feof(fd)) {
        // TODO: rewrite using fgets
        bytes_read_in_chunk = fread(&result->bytes[0] + bytes_read, 1, chunk_size, fd);
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

        if(bytes_read_in_chunk == chunk_size && !feof(fd)) {
            result = resize_buffer(result, result->length + chunk_size);
        }
        else if(ferror(fd)) {
            free(result);
            return NULL;
        }
    }

    if(!found_eol && !feof(fd)) {
        free(result);
        return NULL;
    } else {
        result = resize_buffer(result, eol_index);
        return result;
    }
}

buffer *read_file(const char *filename) {
    FILE *fd = fopen(filename, "r");
    if(!fd) {
        return NULL;
    }

    fseek(fd, 0, SEEK_END);
    size_t file_size = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    buffer *result = allocate_buffer(file_size);
    size_t bytes_read = fread(result->bytes, 1, file_size, fd);

    assert(bytes_read == file_size);

    fclose(fd);
    return result;
}


void print_buffer(buffer *chars) {
    for(size_t i = 0;
        i < chars->length;
        i++)
    {
        if(chars->bytes[i] == '\\') {
            printf("\\\\");
        } else if(chars->bytes[i] >= 32) {
            putchar(chars->bytes[i]);
        } else if(chars->bytes[i] == '\n') {
            printf("\\n");
        }
    }
}

void literally_print_buffer(buffer *chars) {
    for (size_t i = 0;
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
        printf("%02x", chars->bytes[i]);
        if(i % 40 == 39) {
            putchar('\n');
        }
    }
}

void base64_print_buffer(buffer *input) {
    buffer *base64_input = base64_encode(input);

    for(size_t i = 0;
        i < base64_input->length;
        i++)
    {
        putchar(base64_input->bytes[i]);
        if(i % 60 == 59) {
            putchar('\n');
        }
    }  

    free(base64_input);
}

buffer *remove_whitespace(buffer *input) {
    assert(input);

    size_t number_of_whitespaces = 0;
    size_t index;
    for(index = 0;
        index < input->length;
        index++)
    {
        input->bytes[index - number_of_whitespaces] = input->bytes[index];

        if(input->bytes[index] == '\n'
           || input->bytes[index] == '\r'
           || input->bytes[index] == '\t'
           || input->bytes[index] == ' ') {
            number_of_whitespaces++;
        }
    }

    buffer *result = resize_buffer(input, input->length - number_of_whitespaces);
    return result;
}

void fill_buffer_repeating(buffer *buffer_to_fill, byte *data, size_t data_length) {
    for(size_t index = 0;
        index < buffer_to_fill->length;
        index++)
    {
        buffer_to_fill->bytes[index] = data[index % data_length];
    }
}

#endif