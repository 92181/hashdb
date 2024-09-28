#include <stdio.h>
#include "../../hashdb.h"

// Main Function;
int main()
{
    // Create The First Map And Insert Data;
    unsigned char *j[32];dbc(j,10,200);dbi("Math","You",4,3);dbi("Science","Love",7,4);
    
    // Create The Second Map And Insert Data;
    unsigned char *h[32];dbc(h,10,200);dbi("Clarity","Data",7,4);dbi("Passion","Nature",7,6);

    // Retrieve Values Stored In H By Keys;
    unsigned char *y,*r;

    dbg("Turbine",7,&y,&r);if(y>0){while(y<r){printf("%c",*y);y+=1;}printf("\n");}else{printf("Not Found!\n");}
    dbg("Passion",7,&y,&r);if(y>0){while(y<r){printf("%c",*y);y+=1;}printf("\n");}else{printf("Not Found!\n");}

    // Set J To Active Map And Retrieve Data By Keys;
    dbs(j);

    dbg("Clarity",7,&y,&r);if(y>0){while(y<r){printf("%c",*y);y+=1;}printf("\n");}else{printf("Not Found!\n");}
    dbg("Science",7,&y,&r);if(y>0){while(y<r){printf("%c",*y);y+=1;}printf("\n");}else{printf("Not Found!\n");}
    
    // Free The Table And Data Array;
    free(*(unsigned char**)j);free(*(unsigned char**)h);
};