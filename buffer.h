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

byte& buffer::operator[] (size_t index) {
    return this->bytes[index];
}
