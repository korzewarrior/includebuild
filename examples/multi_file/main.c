/**
 * IncludeBuild Multi-File Example - Main Program
 * 
 * This program demonstrates how to use multiple source files
 * with IncludeBuild by utilizing the math and string utilities.
 */

#include <stdio.h>
#include "math_utils.h"
#include "string_utils.h"

int main() {
    printf("========== IncludeBuild Multi-File Demo ==========\n\n");
    
    // Use the math utilities
    printf("Math Utilities:\n");
    printf("  Square of 5: %d\n", square(5));
    printf("  Cube of 3: %d\n", cube(3));
    printf("  Sum of 10 and 20: %d\n", add(10, 20));
    printf("  Max of 42 and 7: %d\n\n", max(42, 7));
    
    // Use the string utilities
    printf("String Utilities:\n");
    char text[100] = "Hello, IncludeBuild!";
    printf("  Original: \"%s\"\n", text);
    
    to_upper(text);
    printf("  To upper: \"%s\"\n", text);
    
    to_lower(text);
    printf("  To lower: \"%s\"\n", text);
    
    printf("  Length: %d characters\n", string_length(text));
    
    printf("\n=================================================\n");
    
    return 0;
} 