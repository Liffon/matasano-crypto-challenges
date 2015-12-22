#include "matasano.h"

int main(int argc, char *argv[]) {
    const char *input_filename = "1-6-input.txt";
    int max_keysize = 40;
    if(argc >= 2) {
        input_filename = argv[1];
    }
    if(argc >= 3) {
        max_keysize = atoi(argv[2]);
    }

    buffer *raw_ciphertext = read_file(input_filename);
    if(!raw_ciphertext) {
        fprintf(stderr, "Unable to open file '%s'\n", input_filename);
        return 1;
    }
    buffer *ciphertext = base64_decode(raw_ciphertext);
    free(raw_ciphertext);

	float normalized_hamming_distances[max_keysize];

	int keysizes[max_keysize];
	int best_keysizes[3];

	buffer *one = allocate_buffer(2);
	buffer *two = allocate_buffer(2);
	buffer *three = allocate_buffer(2);
	buffer *four = allocate_buffer(2);
	for(int keysize = 2;
		keysize <= max_keysize;
		keysize++)
	{
		keysizes[keysize] = keysize;
		one = cut_buffer(ciphertext, 0, keysize, one);
		two = cut_buffer(ciphertext, keysize, keysize, two);
		three = cut_buffer(ciphertext, 2 * keysize, keysize, three);
		four = cut_buffer(ciphertext, 3 * keysize, keysize, four);

        float distances[6];
        distances[0] = hamming_distance(one, two);
        distances[1] = hamming_distance(one, three);
        distances[2] = hamming_distance(one, four);
        distances[3] = hamming_distance(two, three);
        distances[4] = hamming_distance(two, four);
        distances[5] = hamming_distance(three, four);

        float average = (distances[0] + distances[1] + distances[2]
                         + distances[3] + distances[4] + distances[5]) / 6.0;
		normalized_hamming_distances[keysize] = average / (float)keysize;
	}
    free(one);
    free(two);
    free(three);
    free(four);

    buffer *ciphertext_block = allocate_buffer(ciphertext->length);
    buffer *long_key = allocate_buffer(ciphertext->length);
    buffer *key_guesses[max_keysize];
    int most_probable_keysize = -1;

    float best_average_score = INFINITY;
	for(int index = 0;
		index < sizeof(best_keysizes) / sizeof(best_keysizes[0]);
        index++)
    {
        int best_keysize = -1;
        float best_hamming_distance = INFINITY;

        for(int keysize = 2;
            keysize <= max_keysize;
            keysize++)
        {
            if(normalized_hamming_distances[keysize] < best_hamming_distance) {
                best_keysize = keysize;
                best_hamming_distance = normalized_hamming_distances[keysize];
            }
        }
        best_keysizes[index] = best_keysize;
        normalized_hamming_distances[best_keysize] = INFINITY;

        int keysize_guess = best_keysize;

        key_guesses[keysize_guess] = allocate_buffer(keysize_guess);

        float average_score = 0;
        for(int block_index = 0;
            block_index < keysize_guess;
            block_index++)
        {
            ciphertext_block = take_every_nth_byte(ciphertext,
                                                   block_index, keysize_guess,
                                                   ciphertext_block);

            float score =
                    find_best_single_byte_xor_score_with_distribution(
                            ciphertext_block,
                            &key_guesses[keysize_guess]->bytes[block_index], 0);
            average_score += score;

        }
        average_score = average_score / (float)keysize_guess;
        if(average_score < best_average_score) {
            best_average_score = average_score;
            printf("Average score for key \"");
            print_buffer(key_guesses[keysize_guess]);
            printf("\" is %f.\n", average_score);
            most_probable_keysize = keysize_guess;
        }
    }

    printf("Conclusion:\n===========\n");
    printf("The best key guess is of length %d: \"", most_probable_keysize);
    print_buffer(key_guesses[most_probable_keysize]);
    printf("\"\n");

    fill_buffer_repeating(long_key,
                          key_guesses[most_probable_keysize]->bytes,
                          most_probable_keysize);
    buffer *plaintext = xor_buffers(ciphertext, long_key);

    printf("This gives the following plaintext:\n");
    literally_print_buffer(plaintext);

	free(ciphertext);
    free(ciphertext_block);
    free(long_key);
    free(plaintext);
    for(int index = 0;
        index < sizeof(best_keysizes) / sizeof(best_keysizes[0]);
        index++)
    {
        free(key_guesses[best_keysizes[index]]);
    }
	return 0;
}