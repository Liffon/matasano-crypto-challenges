#include "matasano.h"

int main() {
	printf("Please enter hex encoded input:\n");
	buffer *input = read_hex_until_eol();

	byte best_key;
	buffer *best_plaintext;
	float best_score = find_best_single_byte_xor_score_with_distribution(input, &best_key, &best_plaintext, true);

	printf("Most probable key is 0x%02x (gave a score of %f).\nMost probable plaintext:\n", best_key, best_score);
	print_buffer(best_plaintext);
	putchar('\n');

	free(input);
	free(best_plaintext);
	return 0;	
}
