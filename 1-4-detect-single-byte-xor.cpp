#include "matasano.h"

int main(int argc, char *argv[]) {
	buffer **inputs = (buffer **)malloc(sizeof(buffer *));
	FILE *input_file = fopen("1-4-input.txt", "r");
	size_t number_of_inputs = 0;
	while(!feof(input_file)) {
		inputs = (buffer **) realloc(inputs, (number_of_inputs + 1) * sizeof(buffer *));
		inputs[number_of_inputs] = read_hex_until_eol(input_file);
		if (inputs[number_of_inputs]) {
			// hex_print_buffer(inputs[number_of_inputs]);
			// putchar('\n');
			number_of_inputs++;
		}
	}
	fclose(input_file);

	printf("Read %zu lines.\n", number_of_inputs);

	float *scores = (float *)malloc(number_of_inputs * sizeof(float));
	float best_score = INFINITY;
	size_t best_index = number_of_inputs;
	buffer *current_plaintext;
	buffer *best_plaintext = 0;
	byte key = 0;
	for(size_t index = 0;
		index < number_of_inputs;
		index++)
	{
		scores[index] = find_best_single_byte_xor_score_with_distribution(inputs[index], &key, &current_plaintext);
		if(scores[index] < best_score) {
			if(best_plaintext) {
				free(best_plaintext);
			}
			best_plaintext = current_plaintext;
			best_score = scores[index];
			best_index = index;
			printf("Candidate: \"");
			print_buffer(best_plaintext);
			printf("\" with a score of %f.\n", scores[index]);
		} else {
			free(current_plaintext);
		}
	}

	printf("\nBest match is\n");
	hex_print_buffer(inputs[best_index]);
	printf("\nwhich when xored with 0x%02x repeating becomes\n\"", key);
	print_buffer(best_plaintext);
	printf("\".\n");
	free(best_plaintext);
	free(scores);
	for(size_t index = 0;
		index < number_of_inputs;
		index++)
	{
		free(inputs[index]);
	}
	free(inputs);

	return 0;
}