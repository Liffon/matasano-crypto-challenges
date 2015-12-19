#include "matasano.h"
#include <math.h>

float score_plaintext(const buffer *plaintext) {
	int letter_frequencies[26] = {0};

	int letters = 0;

	for(size_t index = 0;
		index < plaintext->length;
		index++)
	{
		char maybe_letter = plaintext->bytes[index];
		if(is_letter(maybe_letter)) {
			letters++;
			letter_frequencies[letter_index(maybe_letter)]++;
		}
	}

	float chiSquared = 0;

	for(char letter = 'a';
		letter <= 'z';
		letter++)
	{
		float observed = letter_frequencies[letter_index(letter)];
		float expected = letters * letter_probabilities[letter_index(letter)];
		chiSquared += pow(observed - expected, 2) / expected;
	}

	int penalty = plaintext->length - letters;
	float score = chiSquared + penalty;

	return score;
}

int main() {
	printf("Please enter hex encoded input:\n");
	buffer *input = read_hex_until_eol();
	buffer *expanded_key = allocate_buffer(input->length);
	buffer *plaintext[256];

	float scores[256];
	float best_score = INFINITY;
	byte best_key = 0;
	for(int index = 0;
		index < 256;
		index++)
	{
		byte key = (byte)index;
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

			printf("Key 0x%02x gave \"", key);
			print_buffer(plaintext[key]);
			printf("\" with score %f\n", best_score);
		}
	}

	printf("Most probable key: 0x%02x\nMost probable plaintext:\n", best_key);
	print_buffer(plaintext[best_key]);
	putchar('\n');

	for(int key = 0;
		key <= 255;
		key++)
	{
		free(plaintext[key]);
	}
	free(input);
	free(expanded_key);
	return 0;	
}