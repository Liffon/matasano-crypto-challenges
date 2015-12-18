#include "matasano.h"

int score_plaintext(buffer *plaintext) {
	// TODO: implement this with character frequency
	return 0;
}

int main() {
	buffer *input = read_hex_until_eol();
	buffer *expanded_key = allocate_buffer(input->length);
	buffer *plaintext[256];

	int scores[256];
	int best_score = 0;
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
		scores[key] = score_plaintext(plaintext[key]);

		if(scores[key] > best_score) {
			best_score = scores[key];
			best_key = key;
		}
	}

	print_buffer(plaintext[best_key]);

	free(input);
	return 0;	
}