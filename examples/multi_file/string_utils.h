#ifndef STRING_UTILS_H
#define STRING_UTILS_H

/**
 * Returns the length of a string
 * @param str The input string
 * @return Length of the string (number of characters)
 */
int string_length(const char* str);

/**
 * Reverses a string in-place
 * @param str The string to reverse
 * @return Pointer to the reversed string (same as input)
 */
char* reverse_string(char* str);

/**
 * Converts a string to uppercase in-place
 * @param str The string to convert
 * @return Pointer to the uppercase string (same as input)
 */
char* to_uppercase(char* str);

#endif /* STRING_UTILS_H */ 