#include <stdio.h>
#include <stdlib.h>

// Global Variables (Thread-Safe);
const unsigned int x0=2654435761,x1=2246822519,x2=x0+x1,x3=319162894+x2;unsigned int z;unsigned char **r,**e,**c,**y;

// The xxHash Hashing Algorithm (32-Bit);
unsigned int xxh(const unsigned char *p,const unsigned char *l)
{
    // Define Result And End Pointer;
    unsigned int h=x3,z=l-p;

    // Main Hashing Logic;
    while(p+4<=l)
    {
        unsigned int k1=*(unsigned int*)p*x1;

        h^=((k1<<13)|(k1>>19))*x0;h=((h<<17)|(h>>15))*x0+668265263;p+=4;
    };

    // Handle Remainder Of Division By 4;
    if(p<l)
    {
        h^=*p*374761393;h=((h<<11)|(h>>21))+x0;p+=1;

        if(p<l)
        {
            h^=*p*374761393;h=((h<<11)|(h>>21))+x0;p+=1;

            if(p<l)
            {
                h^=*p*374761393;h=((h<<11)|(h>>21))+x0;p+=1;
            };
        };
    };

    // Perform Final Bitwise Manipulations;
    h^=z;h=(h^h>>15)*x1;h=((h^h>>13)*3266489917);h^=h>>16;

    return h%z;
};

// Allocate Memory For A New Map;
static inline void dbc(unsigned char **j,unsigned int k,unsigned int l)
{
    // Assign Memory To The Table And Data Array;
    z=k;r=j;unsigned int f=k*sizeof(unsigned char*);*(unsigned char***)r=malloc(f+l);y=j+1;e=j+2;c=j+3;*e=(unsigned char*)*r+f;*c=*e+l;
    
    // Initialize The Table Array To Zero;
    *y=(unsigned char*)*r;do{*(unsigned char**)*y=0;*y+=8;}while(*y<*e);
};

// Set Supplied Map To Active;
static inline void dbs(unsigned char **j)
{
    // Update Cached Pointers;
    r=j;y=j+1;e=j+2;c=j+3;z=(*e-*r)/8;
};

// Insert Key And Value Into Hashmap;
static inline void dbi(unsigned char *m,unsigned char *n,unsigned int z,unsigned int s)
{
    // Check Data Array Memory Bounds;
    unsigned char *h=*y+24+z,*u=h+s,*i;

    if(u>=*c)
    {
        // Update Cache Variables;
        unsigned long int l=u-*e+1024;i=realloc(*r,l);unsigned long int o=i-*(unsigned char**)r;*r=i;h+=o;u+=o;*y+=o;*e+=o;*c=(unsigned char*)*e+l;

        // Shift Table Pointers;
        while(i<*e)
        {
            if(*(unsigned char**)i>0){*(unsigned char**)i+=o;};i+=8;
        };

        // Shift All Data Array Pointers;
        while(i<*y)
        {
            if(*(unsigned char**)(i+8)>0){if(*(unsigned char**)i>0){*(unsigned char**)i+=o;};*(unsigned char**)(i+8)+=o;};*(unsigned char**)(i+16)+=o;

            // Set Stored End Pointer To Next Adress;
            i=*(unsigned char**)(i+16);
        };
    };

    // Return Hash Table Memory Pointer, Assign Table Index To Next Pointer And Create Link With Table;
    unsigned char **a=*(unsigned char***)r+xxh(m,m+z);*(unsigned char**)*y=(unsigned char*)*a;*a=*y;*y+=24;

    // Set Size Of Key And Total Size;
    *(unsigned char**)(*y-16)=h;*(unsigned char**)(*y-8)=u;

    // Set Key And Value At Data Array;
    do{**y=*m;*y+=1;m+=1;}while(*y<h);do{**y=*n;*y+=1;n+=1;}while(*y<u);
};

// Retrieve Value By Key From Table;
static inline void dbg(unsigned char *k,unsigned int s,unsigned char **i,unsigned char **u)
{
    // Return Hash Table Memory Pointer;
    unsigned char *p=k+s,*m,*n=k,*g=*(*(unsigned char***)r+xxh(k,p));
    
    while(g>0)
    {
        // Compare Key To Make Sure There Is No Collision;
        m=g+24;if(*(unsigned char**)(g+8)-m==s)
        {
            // Compare Keys;
            while(k<p&&*m==*k){k+=1;m+=1;};

            if(k==p)
            {
                *i=m;*u=*(unsigned char**)(g+16);
                
                return;
            };
        };

        // Set Stored Pointer To Next Adress;
        g=*(unsigned char**)g;k=n;
    };

    // Not Found;
    *i=0;
};

// Defragment Data Array (Heavy);
static inline void dbf()
{
    // Loop Through Data Array;
    unsigned long int j=0;unsigned char *p=*e,*n=0,*l=0;
    
    while(n<*y)
    {
        // Detect Empty Entry;
        if(*(unsigned long int*)(p+8)==0)
        {
            // Get Size Of Empty Entry;
            j+=(unsigned long int)(*(unsigned char**)(p+16)-p);

            // Skip Over Empty Entry;
            if(l==0){l=p;};n=*(unsigned char**)(p+16);
        };

        // Shift Back When Needed;
        if(j>0)
        {
            // Shift Memory Pointers Back;
            *(unsigned long int*)l=*(unsigned long int*)(n-j);*(unsigned long int*)(l+8)=*(unsigned long int*)(n+8)-j;*(unsigned long int*)(l+16)=*(unsigned long int*)(n+16)-j;
            
            // Assign The Adjusted Pointer To The Table If Not A Collision Entry;
            unsigned char **a=*(unsigned char***)r+xxh(n+24,*(unsigned char**)(n+8));if(*a==n){*a=l;};

            // Shift Key And Value;
            unsigned char *q=*(unsigned char**)(n+16);

            n+=24;l+=24;while(n<q)
            {
                *l=*n;n+=1;l+=1;
            };
        };

        // Assign The End Pointer To The Entry Pointer;
        p=*(unsigned char**)(p+16);
    };

    // Reset Data Array Size;
    *y-=j;
};

// Remove Entry From Table And Data Array;
static inline void dbd(unsigned char *k,unsigned int s)
{
    // Return Hash Table Memory Pointer;
    unsigned char **a=*(unsigned char***)r+xxh(k,k+s),*p=k+s,*m,*f=0,*n=k,*g=*a;

    while(g>0)
    {
        // Compare Key To Make Sure There Is No Collision;
        m=g+24;while(k<p&&*m==*k){k+=1;m+=1;}

        if(k==p)
        {
            if(f>0)
            {
                // Set Previous Next Pointer To Our Current Next Pointer;
                *(unsigned long int*)f=(unsigned long int)*g;
            }
            else
            {
                // Set Table Pointer To Our Current Next Pointer;
                *a=*(unsigned char**)g;
            };

            // Set Key Size To Zero;
            *(unsigned long int*)(g+8)=0;

            // Check If The Data Array Needs To Be Defragmented;
            if(1)
            {
                dbf();
            };
            
            return;
        };

        // Convert Stored Pointer To Next Adress;
        f=g;g=*(unsigned char**)g;k=n;
    };
};

// Write Table And Data Array To Disk;
static inline void dbw(unsigned char *n)
{
    FILE *f=fopen(n,"w");

    // Write Header Data;
    fwrite(r,1,32,f);

    // Write Table And Data Array To File;
    fwrite(*r,1,(*y-*r),f);fclose(f);
};

// Read Table And Data Array From Disk;
static inline void dbr(unsigned char **j,unsigned char *n)
{
    FILE *f=fopen(n,"r");

    // Set Headers To New Map;
    fread(j,1,32,f);r=j;y=r+1;e=r+2;c=r+3;z=(*e-*r)/8;

    // Allocate Memory Based On Stored Headers, And Calculate Memory Shift;
    unsigned long int o,k=*c-*r;n=*r;*(unsigned char***)r=malloc(k);o=(*r-n);fread(*r,1,k,f);*y+=o;*e+=o;*c+=o;
    
    // Shift Table Pointers;
    n=*r;while(n<*e)
    {
        if(*(unsigned char**)n>0){*(unsigned char**)n+=o;};n+=8;
    };

    // Shift All Data Array Pointers;
    while(n<*y)
    {
        if(*(unsigned char**)(n+8)>0){if(*(unsigned char**)n>0){*(unsigned char**)n+=o;};*(unsigned char**)(n+8)+=o;};*(unsigned char**)(n+16)+=o;

        // Set Stored End Pointer To Next Adress;
        n=*(unsigned char**)(n+16);
    };

    fclose(f);
};