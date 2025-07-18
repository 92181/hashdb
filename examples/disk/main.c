#include <stdio.h>
#include <unistd.h>

#define MMAP 1
#include "../../hashdb.c"

// Print Text Response;
static inline void ptr(char *y,char *r)
{
    if(y>0)
    {
        write(1,y,r-y);write(1,"\n",2);
    }
    else
    {
        printf("Not Found!\n");
    };
};

// Main Function;
int main()
{
    // Create A Table And Data Array;
    hdb j;char n[262]="pages/";j.n=n;dbc(&j,12,32);
    
    // Insert Key And Corresponding Value;
    dbi(&j,"Math","The hashmap.",5,12);dbi(&j,"Pear","Data structures.",4,16);dbi(&j,"Orange","Key-value pairs!",6,16);
    
    // Retrieve Values Stored In Array By Key;
    char *y,*r;
    
    y=dbg(&j,"Math",5,&r);ptr(y,r);y=dbg(&j,"Pear",4,&r);ptr(y,r);y=dbg(&j,"Grape",5,&r);ptr(y,r);

    /*


    // Read From Disk By Name;
    unsigned char *h[32],*y,*r;dbr(h,"Research");

    // Retrieve Values Stored In Array By Key;
    dbg("Math",4,&y,&r);if(y>0){while(y<r){printf("%c",*y);y+=1;}printf("\n");}else{printf("Not Found!\n");}
    dbg("Turbine",7,&y,&r);if(y>0){while(y<r){printf("%c",*y);y+=1;}printf("\n");}else{printf("Not Found!\n");}

    // Free All Allocated Memory;
    free(*(unsigned char**)j);free(*(unsigned char**)h);*/
};