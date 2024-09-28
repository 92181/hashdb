#include <stdio.h>
#include "../../hashdb.h"

// Main Function;
int main()
{
    // Create A Table And Data Array;
    unsigned char *j[32];dbc(j,10,200);

    // Insert Key And Corresponding Value;
    dbi("Math","You",4,3);dbi("Science","Love",7,4);dbi("Clarity","Data",7,4);

    // Write Specified Map To Disk;
    dbw("Research");

    // Read From Disk By Name;
    unsigned char *h[32],*y,*r;dbr(h,"Research");

    // Retrieve Values Stored In Array By Key;
    dbg("Math",4,&y,&r);if(y>0){while(y<r){printf("%c",*y);y+=1;}printf("\n");}else{printf("Not Found!\n");}
    dbg("Turbine",7,&y,&r);if(y>0){while(y<r){printf("%c",*y);y+=1;}printf("\n");}else{printf("Not Found!\n");}

    // Free All Allocated Memory;
    free(*(unsigned char**)j);free(*(unsigned char**)h);
};