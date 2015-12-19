#include "matasano.h"
#include <math.h>

float score_plaintext(const buffer *plaintext) {
	int letter_frequencies[26] = {0};

	int letters = 0;

	for(size_t index = 0;
		index < plaintext->length;
		index++)
	{
		char letter = plaintext->bytes[index];
		if(is_letter(letter)) {
			letters++;
			letter_frequencies[letter_index(letter)]++;
		}
	}

	for(char letter = 'a';
		letter <= 'z';
		letter++)
	{
		float observed = 0;
		float expected = 0;
		float chiSquared = pow(observed - expected, 2) / expected;
	}

	return 0;
}

int main() {
	buffer *input = read_hex_until_eol();
	buffer *expanded_key = allocate_buffer(input->length);
	buffer *plaintext[256];

	int scores[256];
	float best_score = INFINITY;
	byte best_key = 0;
	for(byte key = 0;
		key <= 255;
		key++)
	{
		for(size_t index = 0;
			index < expanded_key->length;
			index++)
		{
			expanded_key->bytes[index] = key;
		}

		plaintext[key] = xor_buffers(input, expanded_key);
		scores[key] = score_plaintext(plaintext[key]); // lower is better

		if(scores[key] < best_score) {
			best_score = scores[key];
			best_key = key;
		}
	}

	print_buffer(plaintext[best_key]);

	for(byte key = 0;
		key <= 255;
		key++)
	{
		free(plaintext[key]);
	}
	free(input);
	return 0;	
}