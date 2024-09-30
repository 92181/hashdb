# Tiny C Based HashDB
A tiny database written in plain C, I decided to write this to understand the inner-workings of databases further and familiarize myself with different hashing algoritms such as SHA256 and xxHash.
The library is consists of a single C header file which can be easily included in any C or C++ project.

This hash based 'database' is thus fully embeddable and modular. Three examples are provided such as...
* Basic usage example, insert, retrieve and delete entries with their respondings keys.
* A data map switch example, switches between data maps and retrieves values.
* A disk read and write example, which allows you to store a map on the disk.

# Usage
This is a basic example on how to use the small library correctly. More examples can be found in the example directory.

```c
// Create A Table And Data Array;
unsigned char *j[32];dbc(j,10,200);

// Insert Key And Corresponding Value;
dbi("apple","You",5,3);dbi("appena","Love",6,4);dbi("orange","Data",6,4);

// Remove The Entry By The Supplied Key;
dbd("apple",5);

// Retrieve Values Stored In Array By Key;
unsigned char *y,*r;

dbg("apple",5,&y,&r);if(y>0){while(y<r){printf("%c",*y);y+=1;}printf("\n");}else{printf("Not Found!\n");}
dbg("appena",6,&y,&r);if(y>0){while(y<r){printf("%c",*y);y+=1;}printf("\n");}else{printf("Not Found!\n");}

// Free The Table And Data Array;
free(*(unsigned char**)j);
```

# License & Attribution
It uses an custom implemention the xxHash hashing algorithm created by Yann Collet (BSD 2-Clause License).
This project uses the MIT Attribution license. If you like this project consider starring it.
