#include "string_utils.h"
#include <string.h>
#include <ctype.h>

int string_length(const char* str) {
    if (str == NULL) {
        return 0;
    }
    return strlen(str);
}

char* reverse_string(char* str) {
    if (str == NULL) {
        return NULL;
    }
    
    int length = strlen(str);
    int i, j;
    char temp;
    
    for (i = 0, j = length - 1; i < j; i++, j--) {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
    }
    
    return str;
}

char* to_uppercase(char* str) {
    if (str == NULL) {
        return NULL;
    }
    
    char* p = str;
    while (*p) {
        *p = toupper((unsigned char)*p);
        p++;
    }
    
    return str;
} 