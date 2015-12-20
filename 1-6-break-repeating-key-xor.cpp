#include "matasano.h"

int main(int argc, char *argv[]) {
	const char *input_filename = "1-6-input.txt";
	if(argc >= 2) {
		input_filename = argv[1];
	}

	buffer *ciphertext = read_file(input_filename);
	if(!plaintext) {
		fprintf(stderr, "Unable to open file '%s'\n", input_filename);
		return 1;
	}

	printf("Give me two strings to compute hamming distance on:\n");

	buffer *one = read_hex_until_eol();
	buffer *two = read_hex_until_eol();

	printf("Hamming distance between inputs is %d\n", hamming_distance(one, two));

	free(ciphertext);
	return 0;
}