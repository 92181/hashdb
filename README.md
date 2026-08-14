# Tiny Key-Value Database
A tiny key-value store database written in Rust, I decided to rewrite this to get a better understanding of rust & it's syntax. Originally it was originally written in C, to get more insight into databases and familiarize myself with different hashing algoritms.

The library is consists of a single source file which can be easily included in any Rust project.

Various use cases are shown such as:
* Basic usage example, insert, retrieve and delete entries by key or index.
* A disk write mode (Flush, Restore), which allows you to store memory pages on the disk.
* Database context switching, between multiple active DB's.

## Usage
This is a basic example on how to use the small library correctly.

```rs
code here!
```

TODO

1. test disk store (disk example!).

2. add remove entry functionality (defragment function, real_size, free_list) (ask AI).



// storing a struct on disk using mmap is harder than i thought using rust, so no mmap
// just disk flush & in memory (at operation).
// https://www.geeksforgeeks.org/rust/file-i-o-in-rust/



Rust (Expirimental):

cargo run --example basic
cargo run --example disk

cargo build (--release)


## License & Attribution
It uses the xxHash hashing algorithm created by Yann Collet (BSD 2-Clause License).

If you like this project consider starring it. Made By Wolf Pieter Schulz.
