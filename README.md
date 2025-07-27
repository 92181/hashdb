# Tiny Key-Value Database
A tiny key-value store database written in plain C, I decided to write this to understand the inner-workings of databases further and familiarize myself with different hashing algoritms such as SHA256 and xxHash.
The library is consists of a single C header file which can be easily included in any C or C++ project.

This hash based 'database' is thus fully embeddable and modular. Various use cases are shown such as...
* Basic usage example, insert, retrieve and delete entries with their respondings keys.
* A disk write mode (MMAP), which allows you to store memory pages on the disk.
* Database context switching, between multiple active DB's.

# Usage
This is a basic example on how to use the small library correctly.

```c
// Create Context;
hdb j;dbc(&j,128,1024);

// Insert Key And Corresponding Value;
dbi(&j,"Math","The hashmap.",5,12);

// Remove The Entry By The Supplied Key;
dbd("Math",5);

// Retrieve Values Stored In Array By Key;
char *y,*r;y=dbg(&j,"Math",5,&r);ptr(y,r);

// Destroy Context;
dis(&j);
```

# License & Attribution
It uses an custom implemention of the xxHash hashing algorithm created by Yann Collet (BSD 2-Clause License).

This project uses the MIT Attribution license. If you like this project consider starring it.

Made By Wolf Pieter Schulz.
