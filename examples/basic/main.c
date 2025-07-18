#include <stdio.h>
#include <unistd.h>
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
    hdb j,h;dbc(&j,12,32);//dbc(j,1024,512*512);

    // use h too here!

    // Insert Key And Corresponding Value;
    dbi(&j,"Apple","The hashmap.",5,12);dbi(&j,"Pear","Data structures.",4,16);dbi(&j,"Orange","Key-value pairs!",6,16);
    
    // Remove The Entry By The Supplied Key;
    dbd(&j,"Apple",5);

    // Retrieve Values Stored In Array By Key;
    char *y,*r;
    
    y=dbg(&j,"Apple",5,&r);ptr(y,r);y=dbg(&j,"Pear",4,&r);ptr(y,r);y=dbg(&j,"Grape",5,&r);ptr(y,r);

    // Destroy Datamap Structure;
    //free(*(unsigned char**)j);
};
