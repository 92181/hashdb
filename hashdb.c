#include <stdlib.h>
#include <sys/mman.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>

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

// Handle Page Array Overflow;
static inline void dpo(hdb *j)
{
    if(j->p+1>=j->k)
    {
        unsigned int l=(j->k-j->pages)*2,o=j->p-j->pages;

        j->pages=realloc(j->pages,l*sizeof(unsigned char**));j->p=j->pages+o;j->k=j->pages+l;
    };
};

#if MMAP
// Get Page Path Name;
static inline void phn(hdb *j)
{
    // Fill Path Name;
    char *b=j->e;unsigned int r=j->p-j->pages;printf("r: %u\n",r);

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
};

// Allocate Page;
void *walloc(hdb *j,unsigned long s)
{
    phn(j);

    // Open File (New File, All Bytes: 0);
    int f=open(j->n,O_RDWR|O_CREAT|O_TRUNC,511);

    if(ftruncate(f,s)==-1) 
    {
        exit(1);
    };

    void *p=mmap(NULL,s,PROT_READ|PROT_WRITE,MAP_SHARED,f,0);if((long)p==-1){exit(1);};

    // Save Size & File Descriptor;
    *(unsigned long*)p=s;*(int*)(p+sizeof(long))=f;printf("Page Created:\n");

    return p;
};

// Defragment Data Pages (Heavy);
static inline void dbf(hdb *j)
{
    // Reset Table;
    unsigned char **r=j->table;do{*r=0;r+=1;}while(r<j->s);

    // Loop Through Pages;
    unsigned char **p=j->pages;
    
    while(p<j->p)
    {
        // Get Page Length & File Descriptor;
        char *u=(char*)*p,*e,*i,*c,*m;unsigned long l=*(unsigned long*)u;int f=*(int*)(u+sizeof(long));
        
        e=u+l;c=u+sizeof(l)+sizeof(f);m=c+20;

        // Shift Page Content (Compress);
        while(m<e)
        {
            i=m-20;unsigned int s=*(unsigned int*)(i+8);unsigned long z=*(unsigned long*)(i+12);

            // If Entry Is Valid;
            if(s!=0)
            {
                // Insert Into Table & Handle Next Ptr;
                unsigned char **a=j->table+xxh(m,m+s);*(unsigned char**)c=(unsigned char*)*a;*a=(unsigned char*)c;

                // Write Content (Shift);
                if(c!=i)
                {
                    while(i<m+z){*c=*i;c+=1;i+=1;};
                }
                else
                {
                    c+=z;
                };

                m+=z+20;
            }
            else if(z>0)
            {
                m+=z+20;c+=z;
            }
            else
            {            
                break;
            };
        };

        // Downsize Page (Failure Irrelevant);
        unsigned long n=m-20-u;

        if(l!=n)
        {
            ftruncate(f,n);
        };

        p+=1;
    };
};
#endif

// Init Storage Structure;
static inline unsigned int dbc(hdb *j,unsigned int k,unsigned long l)
{
    // Assign Memory For Table & Set All Entries To 0;
    unsigned int f=k*sizeof(unsigned char*);j->table=malloc(f);j->s=j->table+k;

    unsigned char **r=j->table;do{*r=0;r+=1;}while(r<j->s);

    // Handle Memory Pages;
    j->pages=j->p=malloc(1024*sizeof(unsigned char*));j->k=j->p+1024;

    if(j->p==0||j->table==0)
    {
        return 1;
    };

    #if MMAP
    // Get Path End;
    char *u=j->n;while(*u!=0){u+=1;};j->e=u;

    // Look For Uninitialized Pages At Path;
    struct dirent *e;DIR *d=opendir(j->n);

    printf("j->n: %s\n",j->n); /// ahhhhh init on pages fails second time because no ZERRO!

    if(d!=0)
    {
        while((e=readdir(d))!=0)
        {
            // Skip Hidden;
            if(*e->d_name=='.'){continue;};
 
            // Get File Path & Open It;
            char *b=j->e;phn(j);

            int f=open(j->n,O_RDWR,511);unsigned long l;read(f,&l,sizeof(long));

            char *p=mmap(NULL,l,PROT_READ|PROT_WRITE,MAP_SHARED,f,0),*e=p+l;if((long)p==-1){return 1;};

            // Save Pointer & File Descriptor;
            dpo(j);*j->p=(unsigned char*)p;j->p+=1;p+=sizeof(unsigned long);*(int*)p=f;p+=sizeof(int);
            
            printf("File DesX: %u\n",f);

            char *i=p;

 
            printf("Name: %s, Size: %lu\n",j->n,l);
            char *m=i+20;

            // read page content! (Reinsert Entries Into Table);
            while(m<e)
            {
                unsigned int z=*(unsigned int*)(i+8);unsigned long y=*(unsigned long*)(i+12);

                printf("Sizes: %u, %lu\n",z,y);

                // if keysize == 0  (Always 0 When Not Overwritten!)
                if(z!=0)
                {
                    // Insert Into Table & Handle Next Ptr;
                    unsigned char **a=j->table+xxh(m,m+z);*(unsigned char**)i=(unsigned char*)*a;*a=(unsigned char*)i;

                    i=m+y;m=i+20;
                }
                else if(y>0)
                {
                    // skip over using total size
                    i=m+y;m=i+20;
                }
                else
                {
                    printf("br\n");
                    // if datasize is zero skip page!
                    break;
                };            
            };

            j->y=(unsigned char*)i;
        };

        closedir(d);
    };
    
    printf("j\n");
    if(j->pages==j->p)
    {
        printf("Empty! thus init!\n"); // should not happen second time!
        l+=sizeof(long)+sizeof(int);*j->p=walloc(j,l);j->y=*j->p+sizeof(long)+sizeof(int);
    }
    else
    {
        printf("d\n");
        j->p-=1;
    };
    printf("x\n");
    #else
    *j->p=j->y=malloc(l);
    
    if(*j->p==0)
    {
        return 1;
    };
    #endif

    printf("Done\n");
    return 0;
};

// Destroy Database Structure;
static inline void dis(hdb *j)
{
    // Free Pages;
    #if MMAP
    unsigned char **u=j->pages;
    
    while(u<j->p+1)
    {
        unsigned char *o=*u;int f=*(int*)(o+sizeof(long));

        munmap(o,*(unsigned long*)o);close(f);

        u+=1;
    };

    *j->e=0;
    #else
    unsigned char **u=j->pages;

    while(u<j->p)
    {
        free(*u);u+=1;
    };
    #endif

    // Free Table & Page Array;
    free(j->table);free(j->pages);
};

// Retrieve Value By Key From Table;
static inline char *dbg(hdb *j,char *k,unsigned int s,char **u)
{
    // Return Hash Table Memory Pointer;
    char *p=k+s,*n=k,*m;unsigned char *g=*(j->table+xxh(k,p));

    while(g>0)
    {
        // Compare Key (Collision Check);
        m=(char*)g+20;
        
        if(*(unsigned int*)(g+8)==s)
        {
            // Compare Keys;
            while(k<p&&*m==*k){k+=1;m+=1;};

            if(k==p)
            {
                *u=m+*(unsigned long*)(g+12);return m;
            };
        };

        // Set Stored Pointer To Next Adress;
        g=*(unsigned char**)g;k=n;
    };

    return 0;
};

// Insert Key & Value;
static inline void dbi(hdb *j,char *k,char *n,unsigned int s,unsigned int z)
{
    unsigned char **a=j->table+xxh(k,k+s);

    // Prevent Duplicate Inserts;
    char *r;if(dbg(j,k,s,&r)!=0){return;};

    // Prevent Memory Overflow;
    unsigned char *h=*j->p+20+s,*u=h+z;

    if(u>=*j->p)
    {
        // New Page;
        unsigned long l=**(unsigned long**)j->p;dpo(j);j->p+=1;
        
        if(z+s+32>l){l=z+s*2+32;};

        #if MMAP
        *j->p=walloc(j,l);j->y=*j->p+sizeof(long)+sizeof(int);
        #else
        *j->p=j->y=malloc(l);

        if(*j->p==0)
        {
            return;
        };
        #endif
        
        h=j->y+20+s;u=h+z;
    };

    // Set Next Ptr & Assign Data Ptr;
    *(unsigned char**)j->y=(unsigned char*)*a;*a=j->y;

    // Set Sizes, Write Key & Value;
    *(unsigned int*)(j->y+8)=s;*(unsigned long*)(j->y+12)=s+z;j->y+=20;

    do{*j->y=*k;j->y+=1;k+=1;}while(j->y<h);while(j->y<u){*j->y=*n;j->y+=1;n+=1;};
};

// Remove Entry From Table And Data Array;
static inline void dbd(hdb *j,char *k,unsigned int s)
{
    unsigned char **a=j->table+xxh(k,k+s);char *p=k+s,*m,*f=(char*)*a,*n=k,*g=f;

    while(g>0)
    {
        // Compare Key To Make Sure There Is No Collision;
        m=g+20;while(k<p&&*m==*k){k+=1;m+=1;};

        if(k==p)
        {
            // Change Previous Next To Current Next Ptr;
            *(unsigned long int*)f=(unsigned long int)*g;*(unsigned int*)(g+8)=0;
            
            return;
        };

        // Convert Stored Pointer To Next Adress;
        f=g;g=*(char**)g;k=n;
    };
};