#include "utils.h"

void upper(char str[], size_t n) {
    int i = 0;
    for (i=0; i<n; i++) {

        if (str[i] == 0) {
            break;
        }

        if(str[i] >= 'a' && str[i] <= 'z') { 
            str[i] = str[i] - ('a' - 'A');
        } 

    } 
}
