#include "matasano.h"

buffer *pkcs7_pad(buffer *input, size_t new_length) {
    if(!input) {
        return NULL;
    }

    int length_difference = new_length - input->length;
    assert(length_difference < 256); // Otherwise it won't fit!
    input = resize_buffer(input, new_length);
    if(length_difference > 0) {
        for(size_t index = input->length - length_difference;
            index < input->length;
            index++)
        {
            input->bytes[index] = (byte)length_difference;
        }
    }

    return input;
}

int main() {
    buffer *input = read_hex_until_eol();
    input = pkcs7_pad(input, input->length + 20 - (input->length % 20));

    hex_print_buffer(input);
    return 0;
}