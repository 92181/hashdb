#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>

#define MMAP 1 // (Temporary);

// Main Structure (Hash Table, Size, Page Array, Size, Page Index, Data Index);
struct hdb
{
    unsigned char **table,**s,**pages,**k,**p,*y;

    #if MMAP
    char *n,*e;
    #endif
}
typedef hdb;

// The xxHash Hashing Algorithm (32-Bit);
const unsigned int x0=2654435761,x1=2246822519,x2=x0+x1,x3=319162894+x2;

unsigned int xxh(const char *p,const char *l)
{
    // Define Result & End Ptr;
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

// Introduce page reallocate (To-do!)
// Add DBC Init Pages (To-do!)

#if MMAP
void *walloc(hdb *j,unsigned long int s)
{
    // Fill Path Name;
    char *b=j->e;unsigned int r=j->p-j->pages;

    if(r==0)
    {
        *b='a';*(b+1)=0;
    }
    else
    {
        while(r>0) 
        {
            *b=('a'+(r%26));r/=26;b+=1;
        };

        *b=0;
    };

    // Open File;
    int fd=open(j->n,O_RDWR|O_CREAT|O_TRUNC,511);

    if(ftruncate(fd,s)==-1) 
    {
        exit(1);
    };

    void *p=mmap(NULL,s,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);

    if((long)p==-1)
    {
        exit(1);
    };

    return p;
};

void *wfree()
{
    //munmap
    return 0;
};
#else
void *walloc(unsigned long int s)
{
    return malloc(s);
};

void *wfree()
{
    return 0;
};
#endif

// Init Storage Structure;
static inline void dbc(hdb *j,unsigned int k,unsigned int l)
{
    // Assign Memory For Table & Data Array;
    unsigned int f=k*sizeof(unsigned char*);//z=k;

    j->table=malloc(f);j->s=j->table+k; // where is j->s used? replace with z?
    // check malloc status!

    j->pages=j->p=malloc(1024*sizeof(unsigned char*));j->k=j->pages+1024;
    // check malloc status!

    #if MMAP
    // Get Path Ending;
    char *u=j->n;while(*u!=0){u+=1;};j->e=u;

    // Allocate First Page;
    *j->p=j->y=walloc(j,l);
    #else
    *j->p=j->y=malloc(l);
    #endif

    *(unsigned int*)j->y=l;j->y+=sizeof(unsigned int);

    // Init Table Array To 0;
    unsigned char **r=j->table;do{*r=0;r+=1;}while(r<j->s);printf("Done\n");
};

// Destroy Storage Structure;
static inline void dis()
{
    // diff is init with disk path.
};







// Insert Key & Value;
static inline void dbi(hdb *j,char *m,char *n,unsigned int z,unsigned int s)
{
    unsigned char **a=j->table+xxh(m,m+z);

    // Prevent Memory Overflow;
    unsigned char *h=*j->p+24+z,*u=h+s;

    printf("Insert!\n");

    if(u>=*j->p)
    {
        // Reallocate Page Array (On Page Overflow);
        if(j->p+1>=j->k)
        {
            
        };

        // New Page;
        unsigned int l=(unsigned int)**j->p;
        
        if(z+s+24>l){l=z+s*2+24;};

        j->p+=1;

        printf("DataSizeL: %u\n",l);

        #if MMAP
        *j->p=j->y=walloc(j,l); // fix name
        #else
        *j->p=j->y=malloc(l);
        #endif

        *(unsigned int*)j->y=l;j->y+=sizeof(unsigned int);
        

        h=j->y+24+z;u=h+s;

        // Set datasize here!

        printf("Page Created:\n");
    };

    // Set Next Ptr & Assign Data Ptr;
    *(unsigned char**)j->y=(unsigned char*)*a;*a=j->y;

    // h never gets updated (ON NEW PAGE!) THAT IS THE PROBLEM

    // Set Sizes, Write Key & Value;
    printf("h: %lu\n",(long)(h-*(j->y+24)));
    *(unsigned char**)(j->y+8)=h;
    printf("Fill: %lu\n\n",(long)(*(unsigned char**)(j->y+8))); // matches with h;
    *(unsigned char**)(j->y+16)=u;
    
    j->y+=24;

    do{*j->y=*m;j->y+=1;m+=1;}while(j->y<h);while(j->y<u){*j->y=*n;j->y+=1;n+=1;};
};

// Retrieve Value By Key From Table;
static inline char *dbg(hdb *j,char *k,unsigned int s,char **u)
{
    // Return Hash Table Memory Pointer;
    char *p=k+s,*n=k;unsigned char *m,*g=*(j->table+xxh(k,p));

    while(g>0)
    {
        // Compare Key (Collision Check);
        m=g+24;
        
        if(*(unsigned char**)(g+8)-m==s)
        {
            // Compare Keys;
            while(k<p&&*m==*k){k+=1;m+=1;};

            if(k==p)
            {
                *u=*(char**)(g+16);return (char*)m;
            };
        };

        // Set Stored Pointer To Next Adress;
        g=*(unsigned char**)g;k=n;
    };

    return 0;
};

// Remove Entry From Table And Data Array;
static inline void dbd(hdb *j,char *k,unsigned int s)
{
    unsigned char **a=j->table+xxh(k,k+s);char *p=k+s,*m,*f=(char*)*a,*n=k,*g=(char*)*a;

    while(g>0)
    {
        // Compare Key To Make Sure There Is No Collision;
        m=g+24;while(k<p&&*m==*k){k+=1;m+=1;}

        if(k==p)
        {
            // Change Previous Next To Current Next Ptr;
            *(unsigned long int*)f=(unsigned long int)*g;*(unsigned long int*)(g+8)=0;
            
            return;
        };

        // Convert Stored Pointer To Next Adress;
        f=g;g=*(char**)g;k=n;
    };
};

/*
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
};*/