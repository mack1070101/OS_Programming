/*******************************************************************
 * CMPUT 379 Assignment 3
 * Due: April 12th, 2017
 *
 * Group: Mackenzie Bligh & Justin Barclay
 * CCIDs: bligh & jbarclay
 * Sources:
 * http://www.exploringbinary.com/ten-ways-to-check-if-an-integer-is-a-power-of-two-in-c/
 * Synopsis:
 * The program first reads in the flags from the command line, and verifies them
 * for correctness. Then it creates an array of structs to track the statistics
 * of each inputted tracefile. Next, it creates a hash table for use as the page
 * table, allocate memory for virtual memory. Then it reads all the references
 * from files by calling readRefsFromFiles. It flushes the TLB if necessary, and
 * then performs the necessary additions/lookups by calling addToMemory. Once
 * computation has been completed, it displays it and performs cleanup for
 * termination of the program.
 *
 * *****************************************************************/


/*  Imports */
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "fileIO.h"
#include "linkedlist.h"
#include "memory.h"

/*  Macros */
#define MIN_CLI_ARGS 7

/*  Local Function Declarations */
int isPowerOfTwo(int x);
int getPowerOfTwo(int number);

int main(int argc, char *argv[]){
    int pgsize, tlbentries, quantum, physpages = 0;
    char uniformity, evictionPolicy;
    FILE *tracefiles[argc-MIN_CLI_ARGS];
    int numTraceFiles = argc - MIN_CLI_ARGS;
    int i, z = 0;

    // Initialization struct for collecting tracefile stats
    const struct tracefileStat statInit = {
        .tlbHits =0, .pageFaults = 0, .pageOuts = 0, .average = 0.0
    };

    if(argc < MIN_CLI_ARGS){
        printf("Insufficient number of command line arguments provided\n");
        exit(0);
    }

    // Read command line input, and assign values
    sscanf(argv[1], "%d", &pgsize);
    sscanf(argv[2], "%d", &tlbentries);
    sscanf(argv[4], "%d", &quantum);
    sscanf(argv[5], "%d", &physpages);
    uniformity = *argv[3];
    evictionPolicy = *argv[6];

    // Gather Tracefile filenames
    for(i = MIN_CLI_ARGS; i < argc; i++){
        tracefiles[z] = fopen(argv[i], "rb");
        if(tracefiles[z] == NULL){
            printf("Error opening file\n");
            exit(0);
        }
        z++;
    }

    // Create array to track tracefileStats
    struct tracefileStat traceFileTracker[numTraceFiles];

    for(i = 0; i < numTraceFiles; i++ ){
        traceFileTracker[i] = statInit;
    }

    // Perform error checking on user input
    if(!isPowerOfTwo(pgsize)){
        printf("Page size is not a power of two\n");
        exit(0);
    } else if( pgsize < 16 || pgsize > 65536){
        printf("Page size is out of acceptable range\n");
        exit(0);
    }

    if(!isPowerOfTwo(tlbentries)){
        printf("tlbentries is not a power of two\n");
        exit(0);
    } else if( tlbentries < 8 || tlbentries > 256){
        printf("tlbentries is out of acceptable range\n");
        exit(0);
    }

    if(quantum == 0){
        printf("quantum cannot equal 0\n");
        exit(0);
    }

    if(physpages == 0){
        printf("physpages cannot equal 0\n");
        exit(0);
    } else if(physpages > 1000000){
        printf("physpages larger than spec\n");
        exit(0);
    }

    if(uniformity != 'p' && uniformity != 'g'){
        printf("Must specify process specific or global (p or g)\n");
        exit(0);
    }

    if(evictionPolicy != 'f' && evictionPolicy != 'l'){
        printf("Eviction policy must be f or l\n");
        exit(0);
    }

    // Setup Data structures
    int POLICY = evictionPolicy == 'l'; // LRU = 1, FIFO = 0

    doubleLL* tlb = calloc(1, sizeof(doubleLL));
    doubleLL* virtualMemory = calloc(1, sizeof(doubleLL));
    doubleLL* pageTable;
    doubleLL* pageTables[numTraceFiles];
    int pid =0;
    uint32_t currentReferences[quantum+1];
    node* frameBuffer[physpages];
    int shiftBy = getPowerOfTwo(pgsize);
    int j=0;



    // Allocate pageTables as null
    // add to them as we reach the tables
    for(j=0; j< numTraceFiles; j++){
        pageTables[j] = NULL;
    }

    tlb->maxSize = tlbentries;
    tlb->policy = policyFIFO;

    virtualMemory->maxSize = physpages;
    virtualMemory->policy = policyFIFO;

    newList(tlb);
    newList(virtualMemory);

    int pageNum;
    while(readRefsFromFiles(quantum, tracefiles, numTraceFiles, &pid, currentReferences)){
        // pid = -1 then we have a table we can delete
        if(pid > -1){
            // If tlb is process specific clear it every quantum
            if(uniformity == 'p'){
                deleteList(tlb);
                newList(tlb);
            }
            // Dynamically allocating tables
            // check to see if table has been made yet
            // If table is null make a new one
            if(pageTables[pid] == NULL){
                pageTable = calloc(1, sizeof(doubleLL));
                pageTable->maxSize = pgsize;
                pageTable->policy = policyFIFO;
                newList(pageTable);
                pageTables[pid] = pageTable;
            }


            // Iterate through current references
            for(i = 0; i < quantum; i++){
                //convert endianess
                pageNum = htonl(currentReferences[i]) >> shiftBy;
                // Try adding to memory
                addToMemory(pageNum, pid, POLICY, tlb, pageTables, frameBuffer,
                            virtualMemory, traceFileTracker);
            }
        } else{

            // As a note if a we abandon memory in virtualMemory(we don't go in to clean it up
            // but it easily gets cleaned up in FIFO or LRU
            // and we look for NULL containers in invalidate Frame
            // Get pid of table to delete
            pid = getRecentlyClosed();
            // safety measure
            if(pageTables[pid] != NULL){
                // delete table
                deleteList(pageTables[pid]);
                free(pageTables[pid]);
                pageTables[pid] = NULL;
            }
        }
    }

    // Display output
    for(i = 0; i < numTraceFiles; i++){
                printf("%d\t%d\t%d\t%d\n", traceFileTracker[i].tlbHits, traceFileTracker[i].pageFaults,
                    traceFileTracker[i].pageOuts,(int) traceFileTracker[i].average);
    }
    
    // Clean up after ourselves
    deleteList(tlb);
    deleteList(virtualMemory);
    // Clean up any remaining tables
    for(i = 0; i < numTraceFiles; i++){
        if(pageTables[i] != NULL){
            deleteList(pageTables[i]);
        }
    }
}

int isPowerOfTwo (int x){
  return ((x != 0) && !(x & (x - 1)));
}

int getPowerOfTwo(int number){
    // Assumed we allready checked that number is a power of two
    int count = 0;
    while((number = number >> 1) > 0){
        count++;
    }
    return count;
}
