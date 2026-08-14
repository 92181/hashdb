use dir_db::*;

fn main() {
	let mut db = DB::new();

	add_entry(&mut db, "Rust", b"Value");
	add_entry(&mut db, "Banana", b"Tasty?");

	if let Ok(d) = get_entry(&db, "Banana") {
		let s = match str::from_utf8(&d) {
			Ok(v) => v,
			Err(e) => panic!("Invalid UTF-8 sequence: {}", e),
    };

		println!("result: {}", s);

		//println!("data: {:#?}", &d[0..4]);
	}

	// Free & Exit;
	drop(db);
}