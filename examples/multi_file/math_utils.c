#include "math_utils.h"

int add(int a, int b) {
    return a + b;
}

int subtract(int a, int b) {
    return a - b;
}

int multiply(int a, int b) {
    return a * b;
}

float divide(int a, int b) {
    // Avoid division by zero
    if (b == 0) {
        return 0.0f;
    }
    return (float)a / (float)b;
}

int square(int n) {
    return n * n;
}

int cube(int n) {
    return n * n * n;
}

int max(int a, int b) {
    return (a > b) ? a : b;
} 