#include <stdio.h>
#include "../../hashdb.h"

// Main Function;
int main()
{
    // Create A Table And Data Array;
    unsigned char *j[32];dbc(j,10,200);

    // Insert Key And Corresponding Value;
    dbi("apple","You",5,3);dbi("appena","Love",6,4);dbi("orange","Data",6,4);dbi("grape","A hashmap is a data structure that stores key-value pairs for efficient data retrieval using a hash function!!",5,110);
    
    //printf("t: %lu\n",(unsigned long int)t);printf("j: %lu\n",(unsigned long int)j);

    // Remove The Entry By The Supplied Key;
    dbd("apple",5);

    // Retrieve Values Stored In Array By Key;
    unsigned char *y,*r;

    dbg("apple",5,&y,&r);if(y>0){while(y<r){printf("%c",*y);y+=1;}printf("\n");}else{printf("Not Found!\n");}
    dbg("appena",6,&y,&r);if(y>0){while(y<r){printf("%c",*y);y+=1;}printf("\n");}else{printf("Not Found!\n");}
    dbg("grape",5,&y,&r);if(y>0){while(y<r){printf("%c",*y);y+=1;}printf("\n");}else{printf("Not Found!\n");}

    // Free The Table And Data Array;
    free(*(unsigned char**)j);
};