use std::fs::{self, OpenOptions};
use std::io::{Seek, SeekFrom, Read, Write};
use std::path::PathBuf;

// Default Limits;
const DEF_TABLE_SIZE: u32 = 5120;
const DEF_PAGE_SIZE: usize = 1024*1024;
const DEF_LIST_SIZE: u32 = 512;

const D_ENTRY_HEADER: usize = size_of::<u64>() + size_of::<u16>() * 2;

// Define DB Structs;
#[derive(Default)]
pub struct DB {
	table: Vec<usize>,
	entries: Vec<Entry>,
	pages: Vec<(Vec<u8>, usize, u16)>,
	current_page: usize
}

struct Entry {
	page_index: usize,
	//key_pos: usize,
  //data_pos: usize, 
	pos: (usize, usize),
	size: u64,
	table_next: usize,
	entry_status: u8
}

// The xxHash Hashing Algorithm (32-Bit);
fn xxh(data: &[u8]) -> u32 {
	const X0: u32 = 2654435761;
	const X1: u32 = 2246822519;
	const X3: u32 = 2654435761;
	const X4: u32 = 374761393;
	
	let mut h = X3;
	let mut p = 0;
	let l = data.len();
	let z = l as u32;
	
	// Hashing 4 Bytes;
	while p + 4 <= l {
		let k1 = u32::from_le_bytes([
			data[p],
			data[p + 1],
			data[p + 2],
			data[p + 3],
		]).wrapping_mul(X1);
		
		h ^= (k1.wrapping_shl(13) | k1.wrapping_shr(19)).wrapping_mul(X0);
		h = (h.wrapping_shl(17) | h.wrapping_shr(15))
			.wrapping_mul(X0)
			.wrapping_add(668265263);
		p += 4;
	}
	
	// Handle Remainder (1-3 Bytes);
	if p < l {
		h ^= (data[p] as u32).wrapping_mul(X4);
		h = (h.wrapping_shl(11) | h.wrapping_shr(21)).wrapping_add(X0);
		p += 1;
		
		if p < l {
			h ^= (data[p] as u32).wrapping_mul(X4);
			h = (h.wrapping_shl(11) | h.wrapping_shr(21)).wrapping_add(X0);
			p += 1;
			
			if p < l {
				h ^= (data[p] as u32).wrapping_mul(X4);
				h = (h.wrapping_shl(11) | h.wrapping_shr(21)).wrapping_add(X0);
			}
		}
	}
	
	// Perform Final Bitwise Manipulations;
	h ^= z;
	h = (h ^ h.wrapping_shr(15)).wrapping_mul(X1);
	h = (h ^ h.wrapping_shr(13)).wrapping_mul(3266489917);
	h ^= h.wrapping_shr(16);
	
	return h % DEF_TABLE_SIZE;
}

// Initialize Database;
impl DB {
	pub fn new() -> Self {
		let mut pages = Vec::with_capacity(DEF_LIST_SIZE as usize);
		let t = vec![0; DEF_TABLE_SIZE as usize];
		let e = Vec::with_capacity((DEF_TABLE_SIZE / 2) as usize);

		// Allocate Initial Page;
    //let p = vec![0u8; DEF_PAGE_SIZE as usize]; pages.push((p, 0, 0));
		let p = vec![0u8; DEF_PAGE_SIZE as usize]; pages.push((p, 0, Vec::with_capacity((DEF_TABLE_SIZE / 3) as usize)));

		return Self {
			table: t,
			entries: e,
			pages,
			..Default::default()
		}
	}

	fn get_id_path(i: u64, s: &mut [u8]) {
		let b = ('z' as u8 - 'a' as u8 + 1) as u64;
		let mut c = i;

		for byte in s.iter_mut().take(8) {
			*byte = 'a' as u8 + (c % b) as u8;
			c /= b;
		}
	}

	pub fn disk_restore(mut self, dir_path: &str) -> Self {
		let (mut path, mut id_buf, mut c) = (PathBuf::from(dir_path), [0u8; 8], 0);

		if !path.exists() {
			return self;
		}

		// Serialize DB Pages;
		loop {
			Self::get_id_path(c, &mut id_buf);

			path.push( unsafe { std::str::from_utf8_unchecked(&id_buf) } );

			if path.exists() {
				let mut data_pos = 0;

				if let Ok(disk_data) = fs::read(&path) {
					if !disk_data.is_empty() && disk_data.len() > 8 {
						let raw = &disk_data[data_pos..data_pos + size_of::<u16>()];
						let (entry_amount, mut u) = (u16::from_le_bytes(raw.try_into().unwrap()), 0);
						
						println!("restore: entry amount: {}",entry_amount); // OK.

            data_pos += size_of::<u16>();

						while u < entry_amount && data_pos + D_ENTRY_HEADER < disk_data.len() {
							let total_size = u64::from_le_bytes(disk_data[data_pos..data_pos+8].try_into().unwrap()) as usize; 
              let key_size = u16::from_le_bytes(disk_data[data_pos+8..data_pos+10].try_into().unwrap()) as usize;

							data_pos += D_ENTRY_HEADER;

							// Skip (Mark Removed);
							if key_size == 0 {
								u += 1; continue;
							}

							println!("restore pages SIZES: {} {}",total_size,key_size);


							if data_pos + total_size <= disk_data.len() {
								let (key, value) = (&disk_data[data_pos..data_pos + key_size], &disk_data[data_pos + key_size..data_pos + total_size]);

								add_entry(&mut self, unsafe { std::str::from_utf8_unchecked(key) }, value);
								let l = self.entries.len() - 1; self.entries[l].entry_status = 0;

								println!("restore pages 4x3");

								data_pos += total_size - D_ENTRY_HEADER;
							}

							println!("restore pages 5");

							u += 1;
						}
					}
				}
			} else {
				return self;
			}

			println!("restore pages 5");

			path.pop(); c += 1;
		}
	}
}

// Insert Entry (Key & Value);
pub fn add_entry(db: &mut DB, key: &str, value: &[u8]) {
	let (k, v, s) = (key.as_bytes(), value, key.len() + value.len());

	println!("{}", key); println!("{}", xxh(k));

	// Get Memory Page;
	let data_len = db.pages[db.current_page].0.len();

	if db.pages[db.current_page].1 + s > data_len || db.pages[db.current_page].2 == u16::MAX {
		let mut p_size: usize = DEF_PAGE_SIZE;

		if s > DEF_PAGE_SIZE {
			p_size = value.len() * 2;
		}
		
		let p = vec![0u8; p_size]; db.pages.push((p, 0, 0));

		db.current_page += 1;
	}

	let page = &mut db.pages[db.current_page];
	let (page_data, page_pos) = (&mut page.0, page.1);

	// Create Entry;
	let (key_pos, data_pos) = (page_pos, page_pos + k.len());

	let mut h = Entry {
		page_index: db.current_page,
    pos: (key_pos, data_pos),
		size: s as u64,
    table_next: 0,
		entry_status: 1
	};

	// Store Key & Data;
	page_data[key_pos..key_pos + k.len()].copy_from_slice(k);
	page_data[data_pos..data_pos + v.len()].copy_from_slice(v);

	page.1 += s; page.2 += 1;

	// Entry & Table Insert;
	let i: usize = xxh(k) as usize;

	h.table_next = db.table[i]; db.table[i] = db.entries.len();

	db.entries.push(h);
}

pub fn add_entry_by_index(db: &mut DB, index: u64, value: &[u8]) {
	return add_entry(db, &index.to_string(), value);
}

// Retrieve Entry;
pub fn get_entry(db: &DB, key: &str) -> Result<Vec<u8>, u32> {
	let k: &[u8] = key.as_bytes();
	let mut i = db.table[xxh(k) as usize];

	while i != 0 {
		let e = &db.entries[i]; let s = e.pos.1 - e.pos.0;
		
		// Check Match;
		if k.len() == s {
			let p = &db.pages[e.page_index].0;

			if k[..k.len()] == p[e.pos.0..e.pos.0 + k.len()] {
				let n = e.pos.1;

				return Ok::<Vec<u8>, u32>(p[n..n + (e.size as usize - s)].to_vec());
			}
		}

		i = e.table_next;
	}

	return Err(0);
}

pub fn get_entry_by_index(db: &DB, index: u64) -> Result<Vec<u8>, u32> {
	return get_entry(db, &index.to_string());
}

// Remove Entry Functions;
fn remove_reference(db: &mut DB, key: &[u8], hash: usize) -> Result<Entry, u32> {
	let mut i = db.table[hash]; let mut o = i;

	while i != 0 {
		let e = &db.entries[i]; let s = e.pos.1 - e.pos.0;
		
		// Check Match;
		if k.len() == s {
			let p = &db.pages[e.page_index].0;

			// Remove Reference (Table / Next);
			if k[..k.len()] == p[e.pos.0..e.pos.0 + k.len()] {
				if o == i {
					db.table[o] = 0;
				} else {
					let prev = &db.entries[o]; prev.table_next = e.table_next;
				}

				return Ok::<Entry, u32>(e);
			}
		}
	}

	return Err(0);
}

pub fn remove_entry(db: &mut DB, key: &str) {
	let k: &[u8] = key.as_bytes();

	if let Ok(e) = remove_reference(&db, k, xxh(k) as usize) {
		e.entry_status = 240; // Mark Deleted;
	}
}

pub fn remove_entry_by_index(db: &mut DB, index: u64) {
	let k: &[u8] = index.to_le_bytes();

	if let Ok(e) = remove_reference(&db, k, xxh(k) as usize) {
		e.entry_status = 240; // Mark Deleted;
	}
}

// Defragment Database (In-Memory);
pub fn defragment_pages(db: &mut DB, sensitivity: u32) {
	let (mut i: usize, mut active_entry) = (0, 0); let page_entry_list = vec![0u8; db.pages.len() as usize];

	// Compact Entry List;
	while i < db.entries.len() {
		let e = &db.entries[i];

		if e.entry_status == 240 {
			// Get Next Active;
			while active_entry < db.entries.len() {
				if db.entries[active_entry].entry_status != 240 {
					break;
				}

				active_entry += 1;
			}

			if active_entry != i {
				let x = &mut db.entries[active_entry]; let page_active =  db.pages[x.page_index];

				// Remove Active Entry Reference (Table);
				let key = &page_active.0[x.pos.0..x.pos.1]; let hash = xxh(key) as usize;  remove_reference(&db, key, hash);

				// Build Page Entry List;
				page_entry_list[x.page_index].push(active_entry); 

				// Insert (Table, Next);
				x.table_next = db.table[hash]; db.table[hash] = active_entry; 

				db.entries[i] = x.clone();
			} else {
				db.entries.truncate(i - 1); break;
			}
		}

		i += 1; active_entry += 1;
	}

	// Compact Data Pages;
	i = 0;

	while i < db.pages.len() {
		let (page_entries, mut u, mut data_shift_pos) = (page_entry_list[i], 0, 0); // page_data = db.pages[i].0;

		while u < page_entries.len() - 1 {
			let (e, n) = (page_entries[u], page_entries[u + 1]);

			// Detect Page Gap;
			if e.pos.0 + e.size != n.pos.0 {
				let gap = n.pos.0 - (e.pos.0 + e.size);


				//data_shift_pos += ;
				// shift ()
			}

			// COPY
			page_data.copy_within(e.pos.0..e.pos.0 + e.size, data_shift_pos); // ok?

			data_shift_pos += e.size;

			// ...
			// ...
			
			u += 1;
		}

		// FIRST TODO!
		// TODO!


		//e.entry_status = 2; // Change status?

		db.pages[i].2 = page_entries.len();

		i += 1;
	}

	// Combine Pages (Size < DEF_PAGE_SIZE / 2)
	/*i = 0;

	while i < db.pages.len()
 	{
		i += 1;
	}*/




	// TODO
	// FIRST shift-back all db.entries (update page_entries).
	// (remove every inactive, replace every moved active entry their reference with NEW);
	// so db.entries[page_entries[u]] and page_entries[u], but also table ref + linked list ref (use remove_entry() ????? without set 240)?

	// then move over active references and shift data (change key_pos + data_pos).

	// if e.entry_status == 240 {


					// problem: lots of dead entries not enough to fill.. will cause gaps in db.entries. (as cannot fill over pages..)
					// fix: do not use db.page_entries FOR SHIFT but only for DATA SHIFT (but do update db.page_entries while shifting)







					/*let mut (first_activ, last_activ, total_size) = (u, u, 0);
					
					// Get Next Continuous Active Entries;
					while first_activ < page_entries.len() {
						if db.entries[page_entries[first_activ]].entry_status != 240 {
							last_activ = first_activ;

							while last_activ < page_entries.len() {
								let e = &db.entries[page_entries[last_activ]];

								if e.entry_status == 240 {
									break;
								}

								// e.entry_status = 2;
								total_size += e.size; last_activ += 1;
							}

							break;
						}

						first_activ += 1;
					}*/




					/*if first_activ < page_entries.len() {
						let n_active = &mut db.entries[page_entries[first_activ]];

						// Shift Back Data;
						page_data.copy_within(n_active.key_pos..n_active.key_pos + total_size, shift_pos); // ok?

						shift_pos += total_size;

						// Remove From Entry Lists (Entries, Page Entries);

						// patch with last_active
						db.entries = 
						page_entries[u] = ;

						// FOR EVERY ACTIVE PAGE AFFECTED
						n_active.key_pos = ;
						n_active.data_pos = ;
						// e.entry_status = 2;

						// keep shift amount saved.
						
						// patch hole

*/


	// defragment in memory only
	// not on disk.
	// SET every NON_DELETED ENTRY on PAGE TO e.entry_status = 2; (so disk_flush will update at position!);
}

// DB Disk Flush;
pub fn disk_flush(db: &mut DB, dir_path: &str) -> Result<(), Box<dyn std::error::Error>> { 
	let mut i: usize = 0;

	while i < db.entries.len() {
		let e = &mut db.entries[i];

		// Add || Update Entry;
		if e.entry_status == 1 || e.entry_status == 2 {
			let (mut path, mut id_buf) = (PathBuf::from(dir_path), [0u8; 8]); DB::get_id_path(p as u64, &mut id_buf);

			path.push(unsafe { std::str::from_utf8_unchecked(&id_buf) });

			// Open File;
			let mut file = OpenOptions::new().read(true).write(true).create(true).open(&path)?;

			if file.metadata()?.len() == 0 {
				let p = e.page_index;

				let page_size = db.pages[p].0.len(); file.set_len(page_size as u64)?;
			}
			println!("fileY");

			// Update Entry Count (First: 2B);
			let mut buf = [0u8; size_of::<u16>()]; file.read_exact(&mut buf)?; let p_entry_index = u16::from_le_bytes(buf);

			// add if higher = entry_count_memory;
			if e.entry_status == 1 {
				file.rewind()?; file.write_all(&(p_entry_index + 1).to_le_bytes())?;
			}

			file.seek(SeekFrom::Start((size_of::<u16>() + D_ENTRY_HEADER * p_entry_index as usize + e.pos.0) as u64))?;

			// Write Entry Sizes;
			let key_size: u16 = (e.pos.1 - e.pos.0).try_into().unwrap();

			file.write_all(&(e.size + D_ENTRY_HEADER as u64).to_le_bytes())?; file.write_all(&key_size.to_le_bytes())?; 
			
			file.write_all(&p_entry_index.to_le_bytes())?;

			println!("disk flush sizes: {} {} keypos: {}",e.size + D_ENTRY_HEADER as u64,key_size, e.pos.0); 			

			// Write Entry Data;
			let page_data = &db.pages[db.current_page].0; let entry_data = &page_data[e.pos.0..e.pos.0 + e.size as usize];

			file.write_all(entry_data)?;

			e.entry_status = 0;
		}

		i += 1;
	}

	Ok(())
}