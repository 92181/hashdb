# Tiny Key-Value Database
A tiny key-value store database written in Rust. I decided to rewrite this to get a better understanding of Rust & it's syntax. Originally it was originally written in C, to get more insight into databases and familiarize myself with different hashing algoritms.

The library is consists of a single library file which can be easily included in any Rust project.

Various use cases are shown such as:
* Basic usage example, insert, retrieve and delete entries by key or index.
* Disk write mode (flush, restore), which allows you to store memory pages on the disk.
* Database context switching, between multiple active DB's.

## Usage
This is a basic example on how to use the small library correctly.

```rs
use dir_db::*;

fn main() {
	let mut db = DB::new();

	add_entry(&mut db, "Rust", b"Value");
	add_entry(&mut db, "Banana", b"Tasty?");

	if let Ok(d) = get_entry(&db, "Banana") {
		let s = match str::from_utf8(&d) { Ok(v) => v, Err(e) => panic!("Invalid UTF-8: {}", e) };

		println!("Result: {}", s);
	}

	drop(db);
}
```

Run examples with: `cargo run --example basic` and `cargo run --example disk`

```sh
cargo build (--release)
```

## License & Attribution
It uses the xxHash hashing algorithm created by Yann Collet (BSD 2-Clause License).

If you like this project consider starring it. Made By Wolf Pieter Schulz.
