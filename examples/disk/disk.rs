use dir_db::*;
use std::env;

fn main() {
	// Set CWD (Current Working Directory);
	if let Err(_e) = env::set_current_dir("examples/disk") {
		return;
	}

	// Setup;
	//let mut db = DB::new(); // add later .disk_restore("./pages")
	let mut db = DB::new().disk_restore("./pages");

	let key_name = "Banana";

	add_entry(&mut db, "Rust", b"Value");
	add_entry(&mut db, key_name, b"Tasty?");

	if let Ok(d) = get_entry(&db, key_name) {
		let s = match str::from_utf8(&d) { Ok(v) => v, Err(e) => panic!("Invalid UTF-8: {}", e) };

		println!("Result: {}", s);
	} else {
		println!("No entry with name: {}.",key_name);
	}

	let _ = disk_flush(&mut db, "./pages");

	drop(db);

	// Restore From Disk;
	let mut db = DB::new().disk_restore("./pages");

	if let Ok(d) = get_entry(&db, key_name) {
		let s = match str::from_utf8(&d) { Ok(v) => v, Err(e) => panic!("Invalid UTF-8: {}", e) };

		println!("Result: {}", s);
	} else {
		println!("No entry with name: {}.",key_name);
	}

	remove_entry(&mut db, "Rust");

	defragment_pages(&mut db, 1024);

	// Still increases disk count each time because duplicate entries! Which is ok but also causes PANIC.

	// Delete 1 Page;
	//if true { // REMOVE this section after testing.
	//	let _ = fs::remove_file("./pages/aaaaaaaa"); 
	//}
}