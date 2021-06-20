//
// Created by Rostislav Davydov on 09.06.2021.
//
#include <string.h>
char* substring(char *destination, const char *source, int beg, int n)
{
    // extracts `n` characters from the source string starting from `beg` index
    // and copy them into the destination string
    while (n > 0)
    {
        *destination = *(source + beg);

        destination++;
        source++;
        n--;
    }

    // null terminate destination string
    *destination = '\0';

    // return the destination string
    return destination;
}
const int index_of(char * str, char* substr){
    char *result = strstr(str, substr);
    if (result==NULL) return -1;
    int position = result - str;
    return position;
}



