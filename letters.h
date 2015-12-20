#include <math.h>

const char *letter_frequencies = "etaoinsrhldcumfpgwybvkxjqz";

const float letter_probabilities[26] = {
	0.0804,	// A
	0.0148,	// B
	0.0334,	// C
	0.0382,	// D
	0.1249,	// E
	0.0240,	// F
	0.0187,	// G
	0.0505,	// H
	0.0757,	// I
	0.0016,	// J
	0.0054,	// K
	0.0407,	// L
	0.0251,	// M
	0.0723,	// N
	0.0764,	// O
	0.0214,	// P
	0.0012,	// Q
	0.0628,	// R
	0.0651,	// S
	0.0928,	// T
	0.0273,	// U
	0.0105,	// V
	0.0168,	// W
	0.0023,	// X
	0.0166,	// Y
	0.0009	// Z
}; // Source: http://norvig.com/mayzner.html

bool is_lowercase(char maybe_letter) {
	return (maybe_letter >= 'a') && (maybe_letter <= 'z');
}

bool is_uppercase(char maybe_letter) {
	return (maybe_letter >= 'A') && (maybe_letter <= 'Z');
}

bool is_letter(char maybe_letter) {
	return is_lowercase(maybe_letter) || is_uppercase(maybe_letter);
}

size_t letter_index(char letter) {
	if(is_lowercase(letter)) {
		return letter - 'a';
	} else if(is_uppercase(letter)) {
		return letter - 'A';
	} else {
		assert(0);
	}
}

char to_uppercase(char maybe_letter) {
	char result = maybe_letter;

	if(is_lowercase(maybe_letter)) {
		result += 'A' - 'a';
	}

	return result;
}

char to_lowercase(char maybe_letter) {
	char result = maybe_letter;

	if(is_uppercase(maybe_letter)) {
		result += 'a' - 'A';
	}

	return result;
}

// Lower score means higher probability that the input buffer is ascii-encoded English
float score_plaintext_with_distribution(const buffer *plaintext) {
	int letter_frequencies[26] = {0};

	int letters = 0;
	int non_printables = 0;
	int extended_asciis = 0;
	int spaces = 0;

	for(size_t index = 0;
		index < plaintext->length;
		index++)
	{
		char maybe_letter = plaintext->bytes[index];
		if(is_letter(maybe_letter)) {
			letters++;
			letter_frequencies[letter_index(maybe_letter)]++;
		}
		if(maybe_letter < ' ' && maybe_letter != '\n') {
			non_printables++;
		}
		if(maybe_letter > 127) {
			extended_asciis++;
		}
		if(maybe_letter == ' ') {
			spaces++;
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

	int penalty = 10 * (plaintext->length - letters - spaces) + 50 * non_printables + 10 * extended_asciis;
	float score = chiSquared + penalty;

	return score;
}
