#include "matasano.h"

int main() {
	printf("Give me two strings to compute hamming distance on:\n");

	buffer *one = read_until_eol();
	buffer *two = read_until_eol();

	printf("Hamming distance between inputs is %ld.\n", hamming_distance(one, two));

	return 0;
}