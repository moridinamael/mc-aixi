#include <cassert>
#include <cstdlib>
#include <iostream>
#include "util.hpp"

// Count the number of bits needed to store x >= 0
int bitsRequired(const int x) {
	assert(x >= 0);
	int bits = 1;
	for(int y = 2; y <= x; y *= 2) bits++;
	return bits;
}


// Return a number uniformly between [0, 1]
double rand01() {
	return double(rand()) / double(RAND_MAX);
}


// Return a random integer between [0, end)
int randRange(int end) {
	assert(0 <= end && end <= RAND_MAX);

	// Generate an integer between [0, end) uniformly using rejection sampling.
	int r = rand();
	const int remainder = RAND_MAX % end;
	while (r < remainder) r = rand();
	return r % end;
}


// Return a random integer between [start, end)
int randRange(int start, int end) {
	assert(start < end);
	return start + randRange(end - start);
}


// Decodes the value encoded on the end of a list of symbols. Each symbol
// is a bit in the binary representation of the value, with more significant
// bits at the end of the list.
interaction_t decode(const symbol_list_t &symbols, const int bits) {
	assert(0 <= bits && bits <= 31); // since the value is decoded into an int
	assert(bits <= symbols.size());

	interaction_t value = 0;
	symbol_list_t::const_reverse_iterator it = symbols.rbegin();
	symbol_list_t::const_reverse_iterator end = it + bits;
	for( ; it != end; ++it) {
		value = (*it ? 1 : 0) + 2 * value;
	}

	return value;
}


// Encodes a value onto the end of a symbol list using "bits" symbols
void encode(symbol_list_t &symbols, interaction_t value, const int bits) {
	assert(0 <= bits && bits <= 31); // since the value is being encoded from an int
	assert(0 <= value);

	for (int i = 0; i < bits; i++, value /= 2) {
		symbols.push_back((value % 2) == 1);
	}
}


// Prints an error and exists if a particular option is not specified.
void requiredOption(options_t const& options, const std::string option_name) {
	if(options.count(option_name) == 0) {
		std::cerr << "ERROR: Environment requires option '" << option_name
			<< "' but" << " it was not present." << std::endl;
		exit(EXIT_FAILURE);
	}
}
