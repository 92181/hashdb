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
    // Create Context;
    hdb j,h;char n[262]="pages/";j.n=n;dbc(&j,128,1024);
    
    // Insert Key And Corresponding Value;
    dbi(&j,"Math","The hashmap.",5,12);dbi(&j,"Turbine","Data structures.",7,16);dbi(&j,"Surface","Key-value pairs!",6,16);

    // Create New Context;
    h.n=n;dbc(&h,64,1024);

    // Retrieve Values Stored In Array By Key;
    char *y,*r;y=dbg(&h,"Math",5,&r);ptr(y,r);y=dbg(&h,"Turbine",7,&r);ptr(y,r);y=dbg(&h,"Smooth",5,&r);ptr(y,r);
    
    // Free;
    dis(&j);dis(&h);
};