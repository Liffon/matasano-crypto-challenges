#include <stdint.h>
#include <inttypes.h>

typedef uint8_t byte;

struct buffer {
    size_t length;
    byte bytes[1];

    byte& operator[] (size_t);
};

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
    fread(result->bytes, file_size, 1, fd);

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
