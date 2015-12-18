const char *letter_frequencies = "etaoinsrhldcumfpgwybvkxjqz";

char to_uppercase(char maybe_letter) {
	char result = maybe_letter;

	if((maybe_letter >= 'a') && (maybe_letter <= 'z')) {
		result += 'A' - 'a';
	}

	return result;
}

char to_lowercase(char maybe_letter) {
	char result = maybe_letter;

	if((maybe_letter >= 'A') && (maybe_letter <= 'Z')) {
		result -= 'A' - 'a';
	}

	return result;
}