#ifndef __UTIL_HPP__
#define __UTIL_HPP__
#include <sstream>
#include <string>
#include "main.hpp"


/** Calculate the number of bits needed to store x >= 0. */
int bitsRequired(const int x);

/** Sample a number from the unit interval uniformly at random.
 * \return A random double between 0 and 1. */
double rand01();


/** Sample an integer from a specified range uniformly at random.
 * \param end The end of the range (exclusive) to sample from.
 * \return A random integer greater than or equal to 0 and less than end. */
int randRange(int end);


/** Sample an integer from a specified range uniformly at random.
 * \param start The start of the range (inclusive) to sample from.
 * \param end The end of the range (exclusive) to sample from.
 * \return A random integer greater than or equal to start and less than end. */
int randRange(int start, int end);


/** Decodes a specified number of bits from the end of a symbol list.
 * \param symlist The list of symbols to decode from.
 * \param bits The number of bits from the end of the symbol list to decode.
 * \return The decoded value as an integer. */
interaction_t decode(symbol_list_t const& symbols, const int bits);


/** Encode a value onto the end of a list of symbols using a specified number
 * of bits.
 * \param symlist The list onto which to encode the value.
 * \param value The value to be encoded.
 * \param bits The number of bits of the value to encode onto the symbol list.*/
void encode(symbol_list_t &symbols, interaction_t value, const int bits);


/** Extract a value of type T (e.g. an integer) from a string.
 * \param str The string from which to extract the value.
 * \param value The variable into which to extract the value. */
template <typename T>
void fromString(std::string const& str, T &value) {
	std::istringstream iss(str);
	iss >> value;
}


/** Extract a value of type T from a string.
 * \param str The string from which to extract the value.
 * \return The extracted value. */
template <typename T>
T fromString(std::string const& str) {
	T value;
	fromString(str, value);
	return value;
}


/** Convert a value into a string. */
template <typename T>
std::string toString(const T val) {
	std::ostringstream s;
	s << val;
	return s.str();
}

/** Checks if an option is present in the options map. If not, the program exits.
 * \param options The options map.
 * \param option_name The option to search for. */
void requiredOption(options_t const& options, const std::string option_name);


/** Get the value of a required option. If the option is not present in the
 * options map the program exits.
 * \param options The options map.
 * \param opt The option to retrieve.
 * \return The value of the option. */
template <typename T>
T getRequiredOption(options_t const& options, const std::string option_name) {
	requiredOption(options, option_name);
	return fromString<T>(options.at(option_name));
}


/** Get the value of a required option. If the option is not present in the
 * options map the program exits.
 * \param options The options map.
 * \param option_name The option to retrieve.
 * \param val Stores the retrieved value. */
template <typename T>
void getRequiredOption(options_t const& options, const std::string option_name,
		               T &val) {
	val = getRequiredOption<T>(options, option_name);
}


/** Get the value of a non-required option.
 * \param options The options map.
 * \param option_name The option to retrieve.
 * \param default_value The default value of the option (if not present).
 * \param value Stores the retrieved value. */
template <typename T>
void getOption(options_t & options, const std::string option_name,
		       const T default_value, T &value) {

	if(options.count(option_name) > 0) { // Option is present, extract value
		fromString(options[option_name], value);
	} else {                             // Option is not present, use default
		value = default_value;
		options[option_name] = toString(value);
	}
}


/** Get the value of a non-required option.
 * \param options The options map.
 * \param option_name The option to retrieve.
 * \param default_value The default value of the option (if not present).
 * \return The retrieved value. */
template <typename T>
T getOption(options_t & options, const std::string option_name,
	           const T default_value) {
	T value;
	getOption(options, option_name, default_value, value);
	return value;
}

#endif // __UTIL_HPP__
