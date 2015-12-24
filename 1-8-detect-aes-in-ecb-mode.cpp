#include "matasano.h"

int main(int argc, char *argv[]) {
    buffer **inputs = (buffer **)malloc(sizeof(buffer *));
    FILE *input_file = fopen("1-8-input.txt", "r");
    size_t number_of_inputs = 0;
    while(!feof(input_file)) {
        inputs = (buffer **) realloc(inputs, (number_of_inputs + 1) * sizeof(buffer *));
        inputs[number_of_inputs] = read_hex_until_eol(input_file);
        if (inputs[number_of_inputs]) {
            number_of_inputs++;
        }
    }
    fclose(input_file);

    printf("Read %zu lines.\n", number_of_inputs);

    int scores[number_of_inputs];
    uint64_t *a1, *a2, *b1, *b2;

    for(size_t input_index = 0;
        input_index < number_of_inputs;
        input_index++)
    {
        scores[input_index] = 0;
        buffer *input = inputs[input_index];

        for(size_t base_index = 0;
            base_index < input->length;
            base_index += 16)
        {
            a1 = (uint64_t *)&input->bytes[base_index];
            a2 = (uint64_t *)&input->bytes[base_index + 8];
            for(size_t moving_index = base_index + 16;
                moving_index < input->length;
                moving_index += 16)
            {
                b1 = (uint64_t *)&input->bytes[moving_index];
                b2 = (uint64_t *)&input->bytes[moving_index + 8];

                if(*a1 == *b1 && *a2 == *b2) {
                    scores[input_index]++;
                }
            }
        }
        if(scores[input_index]) {
            printf("Line %zu: score %d.\n", input_index, scores[input_index]);
        }
    }

    for(size_t index = 0;
        index < number_of_inputs;
        index++)
    {
        free(inputs[index]);
    }
    free(inputs);

    return 0;
}