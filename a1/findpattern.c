#include "findpattern.h"
#include <stdlib.h>
/*******************************************************************
 * CMPUT 379 Assignment 1
 * Due:
 *
 * Group: Mackenzie Bligh & Justin Barclay
 * CCIDs: bligh & jbarclay
 * *****************************************************************
 * Notes: this section should be removed
 * - findpattern() cannot send any output to stdout 
 * - Must impliment a way to print the patmatch struct
 *   */

unsigned int findpattern (unsigned char *pattern, unsigned int patlength,\
        struct patmatch *locations, unsigned int loclength){
    int pattern_occurrances = 0;  
    /*  Body of function goes here */
    char* currentAddress = 0x0;
    bool read;
    bool write;
    
    while(currentAddress < 0xffffffff){
        
        read = canRead(currentAddress);
        write = canWrite(currentAddress);
        currentAddress++;
        
        if(read){
            // If the address can be read
            
            // Create a patmatch struct
            struct patmatch *match;
            match = malloc(sizeof(struct patmatch));

            if(write){
            // And the address can be written to 
            }
            // The address can only be read
        } else {
            //if can't read go to next page
            currentAddress = nextPage(currentAddress);
        }
    }
    return pattern_occurrances;
}

