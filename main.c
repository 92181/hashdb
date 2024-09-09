#include <stdio.h>
#include <stdlib.h>

// Global Variables (Thread-Safe);
unsigned int z;unsigned char *v,*x,*d,**t;

// Hash Algorithm (Public Domain);
unsigned int hash(unsigned char *k,unsigned char *j)
{
    // Create A Bad Hash Using The Key;
    unsigned int h=0;while(k<j){h=(h<<5)+*k;k+=1;}

    // Return Hash Value Modules Table Size;
    return h%z;
};

// Allocate Memory For A New Hashmap;
static inline void dbc(unsigned char ***h,unsigned char **j,unsigned int k,unsigned int l)
{
    // Assign Memory To The Table And Data Array;
    z=k;unsigned int f=k*sizeof(h);t=*h=malloc(f+l);v=*j=(unsigned char*)t+f;x=v+l;d=v;

    // Initialize The Table Array To Zero;
    do{**h=0;*h+=1;}while(*h<(t+k));*h=t; // (t+k) hmmm would rather use t+f
};

// Insert Key And Value Into Hashmap;
static inline void dbi(unsigned char *m,unsigned int size,unsigned int s)
{
    // Check Data Array Memory Bounds;
    unsigned char *u=d+24+s;if(u>=x)
    {
        printf("Reallocation Required;\n");
        // Realloc with more space!! desync with h and j?
    };

    // Return Hash Table Memory Pointer;
    unsigned char **a=t+hash(m,m+size);

    // Assign Returned Table Index To Next Pointer And Create Link With Table;
    *(unsigned long int*)d=(unsigned long int)*a;*a=d;printf("Next Pointer: %lu\n",*(unsigned long int*)d);

    // Set Size Of Key And Total Size;
    *(unsigned long int*)(d+8)=(unsigned long int)d+24+size;*(unsigned long int*)(d+16)=(unsigned long int)u;
    
    // Set Key And Value At Data Array;
    d+=24;do{*d=*m;d+=1;m+=1;}while(d<u);
};

// Retrieve Value By Key From Table;
static inline void dbg(unsigned char *k,unsigned int s,unsigned char **i,unsigned char **u)
{
    // Return Hash Table Memory Pointer;
    unsigned char *p=k+s,*m,*n=k,*g=*(t+hash(k,p));
    
    while(g>0)
    {
        // Compare Key To Make Sure There Is No Collision;
        m=g+24;while(k<p&&*m==*k){k+=1;m+=1;}

        if(k==p) // FIRST compare size of keys! t+s==p
        {
            *i=g+24+s;*u=(unsigned char*)*(unsigned long int*)(g+16);
            
            return;
        }

        // Convert Stored Pointer To Next Adress;
        g=(unsigned char*)*(unsigned long int*)g;k=n;
    };

    // Not Found;
    *i=0;
};

// Defragment Data Array (Heavy);
static inline void dbf()
{
    // Loop Through Data Array;
    unsigned char *p=v,*n,*l;while(p<d)
    {
        // Detect Empty Entry;
        if(*(unsigned long int*)(p+8)==0)
        {
            // Get Size Of Empty Entry;
            unsigned long int i=(unsigned long int)((unsigned char*)*(unsigned long int*)(p+16)-p);printf("Empty Entry! Size: %lu\n",i);

            // Skip Over Empty Entry;
            l=p;n=(unsigned char*)*(unsigned long int*)(p+16);

            while(n<d)
            {
                // Shift Memory Pointers Back;
                *(unsigned long int*)l=*(unsigned long int*)n-i;*(unsigned long int*)(l+8)=*(unsigned long int*)(n+8)-i;*(unsigned long int*)(l+16)=*(unsigned long int*)(n+16)-i;
                
                
                //printf("Value For Total Size: %lu\n",*(unsigned long int*)(n+16)-i); // Should be!
                //printf("%lu\n",*(unsigned long int*)(l+16));printf("%lu\n",(unsigned long int)d);

                // Shift Key And Value;
                n+=24;l+=24;while(n<(unsigned char*)*(unsigned long int*)(n+16))
                {
                    n+=1;l+=1;
                }

                // Assign The End Pointer To The Entry Pointer;
                n=(unsigned char*)*(unsigned long int*)(n+16); // Alloc Segm!

                printf("Value For Total Size: %lu\n",*(unsigned long int*)(n+16)); // Segm
                printf("!!!\n");

                // 22 32 14 5
                // 22 14 5

                //break; // For Now...
            }

            // Reset Data Array Size;
            d-=i;

            continue;
        };

        // Assign The End Pointer To The Entry Pointer;
        p=(unsigned char*)*(unsigned long int*)(p+16);
    };
};

// Remove Entry From Table And Data Array;
static inline void dbd(unsigned char *k,unsigned int s)
{
    // Return Hash Table Memory Pointer;
    unsigned char **a=t+hash(k,k+s);printf("%u\n",hash(k,k+s));

    // Return Hash Table Memory Pointer;
    unsigned char *p=k+s,*m,*f=0,*n=k,*g=*(t+hash(k,p));

    while(g>0)
    {
        // Compare Key To Make Sure There Is No Collision;
        m=g+24;while(k<p&&*m==*k){k+=1;m+=1;}

        if(k==p) // FIRST compare size of keys! t+s==p
        {
            if(f>0)
            {
                // Set Previous Next Pointer To Our Current Next Pointer;
                *(unsigned long int*)f=(unsigned long int)*g;
            }
            else
            {
                // Set Table Pointer To Our Current Next Pointer;
                *a=(unsigned char*)*(unsigned long int*)g;
            };

            // Set Key Size To Zero;
            printf("%lu\n",*(unsigned long int*)(g+8));
            *(unsigned long int*)(g+8)=0;
            //*(unsigned long int*)(g+8)=0;
            printf("%lu\n",*(unsigned long int*)(g+8));

            // Check If The Data Array Needs To Be Defragmented;
            if(0)
            {
                dbf();
            };
            
            return;
        }

        // Convert Stored Pointer To Next Adress;
        f=g;g=(unsigned char*)*(unsigned long int*)g;k=n; 
    };

    // Move Next Pointer To Table Array; hmm flawed?
    // Clean up! Just memory perfect clean up with tolarance do not do it every free!
};

// Write Table And Data Array To Disk;
static inline void dbw()
{
    FILE *file=fopen("tdb","w");

    // Write the data to the file
    fwrite("\n.",1,43,file);

    while(0) // write not complete
    {
        //unsigned int l=fwrite(t,1,t-x,file);
        // fwrite t using x size, t<x
    }

    fclose(file);
};

// Read Table And Data Array From Disk;
static inline void dbr()
{
    // WIP;
};

// FIRST dbd, free!! High Priority!
// THEN write to disk

// row database, colums with direct memory access array?

// Main Function;
int main()
{
    // Create A Table And Data Array;
    unsigned char **h,*j;dbc(&h,&j,10,200);

    // Insert Key And Corresponding Value;
    dbi("appleHello",5,10);dbi("appenaData",6,10);dbi("orangeHey",6,9);

    // Remove The Entry By The Supplied Key;
    dbd("apple",5);
    //dbd("appena",6);

    // Retrieve Values Stored In Array By Key;
    unsigned char *y,*r;

    dbg("apple",5,&y,&r);if(y>0){while(y<r){printf("%c",*y);y+=1;}printf("\n");}else{printf("Not Found!\n");}
    dbg("appena",6,&y,&r);if(y>0){while(y<r){printf("%c",*y);y+=1;}printf("\n");}else{printf("Not Found!\n");}
    dbg("grape",5,&y,&r);if(y>0){while(y<r){printf("%c",*y);y+=1;}printf("\n");}else{printf("Not Found!\n");}

    // Free The Table And Data Array;
    free(h);
};