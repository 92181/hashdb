# Tiny C Based HashDB
A tiny database written in plain C, I decided to write this to understand the inner-workings of databases further and familiarize myself with different hashing algoritms such as SHA256 and xxHash.

WIP;

# Usage

```c
// Create A Table And Data Array;
unsigned char **h,*j;dbc(&h,&j,10,200);

// Insert Key And Corresponding Value;
dbi("appleHello",5,10);dbi("appenaData",6,10);dbi("orangeHey",6,9);

// Remove The Entry By The Supplied Key;
dbd("apple",5);

// Retrieve Values Stored In Array By Key;
unsigned char *y,*r;

dbg("apple",5,&y,&r);if(y>0){while(y<r){printf("%c",*y);y+=1;}printf("\n");}else{printf("Not Found!\n");}

// Free The Table And Data Array;
free(h);
```

# License
This project uses the MIT Attribution license. If you like this project consider starring it.
