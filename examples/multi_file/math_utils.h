#ifndef MATH_UTILS_H
#define MATH_UTILS_H

/**
 * Adds two integers
 * @param a First integer
 * @param b Second integer
 * @return Sum of a and b
 */
int add(int a, int b);

/**
 * Subtracts second integer from first
 * @param a First integer
 * @param b Second integer
 * @return Difference (a - b)
 */
int subtract(int a, int b);

/**
 * Multiplies two integers
 * @param a First integer
 * @param b Second integer
 * @return Product of a and b
 */
int multiply(int a, int b);

/**
 * Divides first integer by second
 * @param a First integer (numerator)
 * @param b Second integer (denominator)
 * @return Quotient of a and b as a float
 */
float divide(int a, int b);

/**
 * Calculates the square of a number
 * @param n The number to square
 * @return n squared
 */
int square(int n);

/**
 * Calculates the cube of a number
 * @param n The number to cube
 * @return n cubed
 */
int cube(int n);

/**
 * Returns the maximum of two integers
 * @param a First integer
 * @param b Second integer
 * @return The larger of a and b
 */
int max(int a, int b);

#endif /* MATH_UTILS_H */