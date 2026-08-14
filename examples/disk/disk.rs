use dir_db::*;
use std::env;
use std::fs;

fn main() {
	// Set CWD (Current Working Directory);
	if let Err(_e) = env::set_current_dir("examples/disk") {
		return;
	}

	// Setup;
	let mut db = DB::new(); // add later .disk_restore("./pages")

	let key_name = "Banana";

	add_entry(&mut db, "Rust", b"Value");
	add_entry(&mut db, key_name, b"Tasty?");

	if let Ok(d) = get_entry(&db, key_name) {
		let s = match str::from_utf8(&d) {
			Ok(v) => v,
			Err(e) => panic!("Invalid UTF-8 sequence: {}", e),
    };

		println!("result: {}", s);
		
		//println!("data: {:#?}", &d[0..4]);
	}

	let _ = disk_flush(&mut db, "./pages"); // TEST!

	drop(db);

	println!("Dropped DB, Restore.");

	// Restore From Disk;
	let db = DB::new().disk_restore("./pages");

	if let Ok(d) = get_entry(&db, key_name) {
		let s = match str::from_utf8(&d) {
			Ok(v) => v,
			Err(e) => panic!("Invalid UTF-8 sequence: {}", e),
    };

		println!("result: {}", s);
		
		//println!("data: {:#?}", &d[0..4]);
	} else {
		println!("No entry with name: {}.",key_name);
	}

	defragment_pages(&db, 1024);

	// Delete 1 Page;
	if true {
		let _ = fs::remove_file("./pages/aaaaaaaa");
	}
}