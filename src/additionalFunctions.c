//
//  additionalFunctions.c
//  
//
//  Created by Emma Barme on 05/05/2015.
//
//

#include "additionalFunctions.h"

void sha_hexa_to_bin(char *dest, char *src)
{
    long int i=0;
    
    printf("\nEquivalent binary value: ");
    for (i = 0; i < 40; i++){
        switch(src[i]){
            case '0': dest[i*4] = '0'; dest[i*4+1] = '0'; dest[i*4+2] = '0'; dest[i*4+3] = '0'; break;
            case '1': dest[i*4] = '0'; dest[i*4+1] = '0'; dest[i*4+2] = '0'; dest[i*4+3] = '1'; break;
            case '2': dest[i*4] = '0'; dest[i*4+1] = '0'; dest[i*4+2] = '1'; dest[i*4+3] = '0'; break;
            case '3': dest[i*4] = '0'; dest[i*4+1] = '0'; dest[i*4+2] = '1'; dest[i*4+3] = '1'; break;
            case '4': dest[i*4] = '0'; dest[i*4+1] = '1'; dest[i*4+2] = '0'; dest[i*4+3] = '0'; break;
            case '5': dest[i*4] = '0'; dest[i*4+1] = '1'; dest[i*4+2] = '0'; dest[i*4+3] = '1'; break;
            case '6': dest[i*4] = '0'; dest[i*4+1] = '1'; dest[i*4+2] = '1'; dest[i*4+3] = '0'; break;
            case '7': dest[i*4] = '0'; dest[i*4+1] = '1'; dest[i*4+2] = '1'; dest[i*4+3] = '1'; break;
            case '8': dest[i*4] = '1'; dest[i*4+1] = '0'; dest[i*4+2] = '0'; dest[i*4+3] = '0'; break;
            case '9': dest[i*4] = '1'; dest[i*4+1] = '0'; dest[i*4+2] = '0'; dest[i*4+3] = '1'; break;
            case 'A': dest[i*4] = '1'; dest[i*4+1] = '0'; dest[i*4+2] = '1'; dest[i*4+3] = '0'; break;
            case 'B': dest[i*4] = '1'; dest[i*4+1] = '0'; dest[i*4+2] = '1'; dest[i*4+3] = '1'; break;
            case 'C': dest[i*4] = '1'; dest[i*4+1] = '1'; dest[i*4+2] = '0'; dest[i*4+3] = '0'; break;
            case 'D': dest[i*4] = '1'; dest[i*4+1] = '1'; dest[i*4+2] = '0'; dest[i*4+3] = '1'; break;
            case 'E': dest[i*4] = '1'; dest[i*4+1] = '1'; dest[i*4+2] = '1'; dest[i*4+3] = '0'; break;
            case 'F': dest[i*4] = '1'; dest[i*4+1] = '1'; dest[i*4+2] = '1'; dest[i*4+3] = '1'; break;
            case 'a': dest[i*4] = '1'; dest[i*4+1] = '0'; dest[i*4+2] = '1'; dest[i*4+3] = '0'; break;
            case 'b': dest[i*4] = '1'; dest[i*4+1] = '0'; dest[i*4+2] = '1'; dest[i*4+3] = '1'; break;
            case 'c': dest[i*4] = '1'; dest[i*4+1] = '1'; dest[i*4+2] = '0'; dest[i*4+3] = '0'; break;
            case 'd': dest[i*4] = '1'; dest[i*4+1] = '1'; dest[i*4+2] = '0'; dest[i*4+3] = '1'; break;
            case 'e': dest[i*4] = '1'; dest[i*4+1] = '1'; dest[i*4+2] = '1'; dest[i*4+3] = '0'; break;
            case 'f': dest[i*4] = '1'; dest[i*4+1] = '1'; dest[i*4+2] = '1'; dest[i*4+3] = '1'; break;
            default:  printf("\nInvalid hexadecimal digit %c ",src[i]); return;
        }
    }
}