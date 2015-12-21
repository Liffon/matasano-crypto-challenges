#include "matasano.h"

int main(int argc, char *argv[]) {
	const char *input_filename = "1-6-input.txt";
	if(argc >= 2) {
		input_filename = argv[1];
	}

	buffer *raw_ciphertext = read_file(input_filename);
	if(!raw_ciphertext) {
		fprintf(stderr, "Unable to open file '%s'\n", input_filename);
		return 1;
	}
	buffer *ciphertext = base64_decode(raw_ciphertext);

	float normalized_hamming_distances[ciphertext->length / 2];

	int keysize;
	buffer *one = allocate_buffer(2);
	buffer *two = allocate_buffer(2);
	for(keysize = 2;
		keysize <= ciphertext->length / 2;
		keysize++)
	{
		one = cut_buffer(ciphertext, 0, keysize, one);
		two = cut_buffer(ciphertext, keysize, keysize, two);

		normalized_hamming_distances[keysize] = hamming_distance(one, two) / (float)keysize;
		printf("Normalized Hamming distance: %f\n", normalized_hamming_distances[keysize]);
	}

	free(ciphertext);
	return 0;
}