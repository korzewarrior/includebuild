/**
 * IncludeBuild Library Example - The Magic Solution
 * 
 * This file demonstrates how IncludeBuild can build a complete
 * library with static and dynamic versions plus tests with
 * just TWO LINES of code.
 */

#include "../../build.h"

int main(int argc, char** argv) {
    // Initialize IncludeBuild
    ib_init();
    
    // Enable verbose output to see what's happening
    ib_set_verbose(true);
    
    // Build the library - this handles everything: static, dynamic, tests
    return ib_build_library("logger", argc, argv) ? 0 : 1;
}