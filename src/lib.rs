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

#[derive(Clone, Copy)]
struct Entry {
	page_index: usize,
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
    let p = vec![0u8; DEF_PAGE_SIZE as usize]; pages.push((p, 0, 0));

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
						
            data_pos += size_of::<u16>();

						while u < entry_amount && data_pos + D_ENTRY_HEADER < disk_data.len() {
							let total_size = u64::from_le_bytes(disk_data[data_pos..data_pos+8].try_into().unwrap()) as usize; 
              let key_size = u16::from_le_bytes(disk_data[data_pos+8..data_pos+10].try_into().unwrap()) as usize;

							data_pos += D_ENTRY_HEADER;

							// Skip (Mark Removed);
							if key_size == 0 {
								u += 1; continue;
							}

							if data_pos + total_size <= disk_data.len() {
								let (key, value) = (&disk_data[data_pos..data_pos + key_size], &disk_data[data_pos + key_size..data_pos + total_size]);

								add_entry(&mut self, unsafe { std::str::from_utf8_unchecked(key) }, value);
								let l = self.entries.len() - 1; self.entries[l].entry_status = 0;

								data_pos += total_size - D_ENTRY_HEADER;
							}

							u += 1;
						}
					}
				}
			} else {
				return self;
			}

			path.pop(); c += 1;
		}
	}
}

// Retrieve Entry;
fn get_entry_internal(db: &DB, hash: usize, key: &[u8]) -> Result<Entry, u32> {
	let mut i = db.table[hash];

	while i != 0 {
		let e = &db.entries[i]; let s = e.pos.1 - e.pos.0;
		
		// Check Match;
		if key.len() == s {
			let p = &db.pages[e.page_index].0;

			if key == &p[e.pos.0..e.pos.0 + key.len()] {
				return Ok(*e);
			}
		}

		i = e.table_next;
	}

	return Err(0);
}

pub fn get_entry(db: &DB, key: &str) -> Result<Vec<u8>, u32> {
	let k: &[u8] = key.as_bytes(); let mut i = db.table[xxh(k) as usize];

	while i != 0 {
		let e = &db.entries[i]; let s = e.pos.1 - e.pos.0;
		
		// Check Match;
		if k.len() == s {
			let p = &db.pages[e.page_index].0;

			if k[..k.len()] == p[e.pos.0..e.pos.0 + k.len()] {
				let n = e.pos.1;

				return Ok(p[n..n + (e.size as usize - s)].to_vec());
			}
		}

		i = e.table_next;
	}

	return Err(0);
}

pub fn get_entry_by_index(db: &DB, index: u64) -> Result<Vec<u8>, u32> {
	let k: [u8; size_of::<u64>()] = index.to_le_bytes();

	if let Ok(e) = get_entry_internal(db, db.table[xxh(&k) as usize], &k) {
		let n = e.pos.1; let p = &db.pages[e.page_index].0;

		return Ok(p[n..n + (e.size as usize - size_of::<u64>())].to_vec());
	}

	return Err(0);
}

// Insert Entry (Key & Value);
pub fn add_entry(db: &mut DB, key: &str, value: &[u8]) {
	let (k, v, s) = (key.as_bytes(), value, key.len() + value.len()); let i = xxh(k) as usize;

	// Prevent Duplicates;
	if let Ok(_d) = get_entry_internal(db, i, k) {
		return;
	}

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
	h.table_next = db.table[i]; db.table[i] = db.entries.len();

	db.entries.push(h);
}

pub fn add_entry_by_index(db: &mut DB, index: u64, value: &[u8]) {
	return add_entry(db, &index.to_string(), value);
}

// Remove Entry Functions;
fn remove_reference<'a>(table: &mut [usize], pages: &[(Vec<u8>, usize, u16)], entries: &'a mut [Entry], key: &[u8], hash: usize) -> Result<&'a mut Entry, u32> {
	let mut i = table[hash]; let mut o = i;

	while i != 0 {
		let e = &entries[i]; let s = e.pos.1 - e.pos.0;

		if key.len() == s {
			let p = &pages[e.page_index].0;

			if key == &p[e.pos.0..e.pos.0 + key.len()] {
				if o == i {
					table[hash] = 0;
				} else {
					entries[o].table_next = e.table_next;
				}

				return Ok(&mut entries[i]);
			}
		}

		o = i; i = e.table_next;
	}

	return Err(0);
}

pub fn remove_entry(db: &mut DB, key: &str) {
	let k: &[u8] = key.as_bytes();

	if let Ok(e) = remove_reference(&mut db.table, &db.pages, &mut db.entries, k, xxh(k) as usize) {
		e.entry_status = 240; // Mark Deleted;
	}
}

pub fn remove_entry_by_index(db: &mut DB, index: u64) {
	let k: &[u8] = &index.to_le_bytes();

	if let Ok(e) = remove_reference(&mut db.table, &db.pages, &mut db.entries, k, xxh(k) as usize) {
		e.entry_status = 240; // Mark Deleted;
	}
}

// Defragment Database (In-Memory);
pub fn defragment_pages(db: &mut DB, _sensitivity: u32) {
	if db.entries.is_empty() { return; }

	let mut page_entry_list: Vec<Vec<usize>> = vec![vec![]; db.pages.len()];
	let (mut i, mut active_entry) = (0, db.entries.len() - 1);

	// Compact Entry List;
	while i < active_entry {
		if db.entries[i].entry_status == 240 {
			// Get Next Active;
			while active_entry > 0 {
				if db.entries[active_entry].entry_status != 240 {
					break;
				}

				active_entry -= 1;
			}

			if active_entry != i {
				let x = db.entries[active_entry];

				// Remove Active Entry Reference (Table);
				let key = &db.pages[x.page_index].0[x.pos.0..x.pos.1]; 

				let hash = xxh(key) as usize; let _ = remove_reference(&mut db.table, &db.pages, &mut db.entries, key, hash);

				db.entries[i] = x;

				// Insert (Table, Next);
				db.entries[i].table_next = db.table[hash]; 
				
				db.table[hash] = active_entry; 
			} else {
				break;
			}
		}

		// Build Page Entry List;
		page_entry_list[db.entries[i].page_index].push(i);

		i += 1;
	}

	db.entries.truncate(active_entry);

	// Compact Data Pages;
	i = 0;

	while i < db.pages.len() {
		let (page_entries, mut u, page_data, mut data_shift_pos) = (&page_entry_list[i], 0, &mut db.pages[i].0, 0);

		if !page_entries.is_empty() {
			while u < page_entries.len() - 1 {
				let e = &mut db.entries[page_entries[u]]; let s = e.size as usize;

				// Skip (No Gap);
				if data_shift_pos == e.pos.0 {
					u += 1; data_shift_pos += s; continue;
				}

				// Copy (Shift Data);
				page_data.copy_within(e.pos.0..e.pos.0 + s, data_shift_pos);

				data_shift_pos += s; e.entry_status = 2;

				u += 1;
			}
		}

		// Update Page Info;
		db.pages[i].0.truncate(data_shift_pos); db.pages[i].1 = data_shift_pos; db.pages[i].2 = page_entries.len() as u16;

		i += 1;
	}

	// Combine Pages;
	i = 0;

	while i < db.pages.len() - 1
 	{
		let (left, right) = db.pages.split_at_mut(i + 1); let (page, next_page) = (&mut left[i], &mut right[0]);

		// Check Max Fit (Pages Truncated);
		let new_size = page.0.len() + next_page.0.len();

		if new_size < DEF_PAGE_SIZE {
			page.0.resize(new_size, 0u8);

			// Combine Entries;
			let (n_page_entries, mut u) = (&page_entry_list[i + 1], 0);

			while u < n_page_entries.len() {
				let n_entry = &mut db.entries[n_page_entries[u]];

				// Copy Data;
				page.0.copy_from_slice(&next_page.0[n_entry.pos.0..n_entry.pos.0 + n_entry.size as usize]);

				n_entry.pos.1 = page.1 + (n_entry.pos.1 - n_entry.pos.0); n_entry.pos.0 = page.1;

				page.1 += n_entry.size as usize; page.2 += 1;

				u += 1;
			}
		
			// Remove Page;
			db.pages.swap_remove(i + 1);
		} else {
			i += 1;
		}

		// Update Page Index (All Entries);
		let (page_entries, mut u) = (&page_entry_list[i], 0);

		while u < page_entries.len() {
			db.entries[page_entries[u]].page_index = i; u += 1;
		}
	}

	// db.pages.truncate();
	db.current_page = db.pages.len() - 1;
}

// DB Disk Flush;
pub fn disk_flush(db: &mut DB, dir_path: &str) -> Result<(), Box<dyn std::error::Error>> { 
	let (mut i, mut page_max) = (0, 0); let (mut path, mut id_buf) = (PathBuf::from(dir_path), [0u8; 8]);

	while i < db.entries.len() {
		let e = &mut db.entries[i]; let p = e.page_index;

		// Count Pages;
		if page_max < p { page_max = p; }

		// Add || Update Entry;
		if e.entry_status == 1 || e.entry_status == 2 {
			DB::get_id_path(p as u64, &mut id_buf); path.push(unsafe { std::str::from_utf8_unchecked(&id_buf) });

			// Open File;
			let mut file = OpenOptions::new().read(true).write(true).create(true).open(&path)?; path.pop();

			if file.metadata()?.len() == 0 {
				let page_size = db.pages[p].0.len(); file.set_len(page_size as u64)?;
			}

			// Get Entry Count (First: U16);
			let page_data = &db.pages[db.current_page]; 

			let mut buf = [0u8; size_of::<u16>()]; file.read_exact(&mut buf)?; let mut p_entry_index = u16::from_le_bytes(buf);

			// Shrink Disk Pages;
			if p_entry_index > page_data.2 {
				p_entry_index = page_data.2; file.set_len((page_data.0.len() + D_ENTRY_HEADER * p_entry_index as usize) as u64)?; 
			}

			if e.entry_status == 1 {
				file.rewind()?; file.write_all(&(p_entry_index + 1).to_le_bytes())?;
			}

			file.seek(SeekFrom::Start((size_of::<u16>() + D_ENTRY_HEADER * p_entry_index as usize + e.pos.0) as u64))?;

			// Write Entry Sizes;
			let key_size: u16 = (e.pos.1 - e.pos.0).try_into().unwrap();

			file.write_all(&(e.size + D_ENTRY_HEADER as u64).to_le_bytes())?; 
			file.write_all(&key_size.to_le_bytes())?; file.write_all(&p_entry_index.to_le_bytes())?;

			// Write Entry Data;
			let entry_data = &page_data.0[e.pos.0..e.pos.0 + e.size as usize]; file.write_all(entry_data)?;

			e.entry_status = 0;
		}

		i += 1;
	}

	// Delete Empty Disk Pages;
	let mut c: u64 = (page_max as u64) + 1;

	loop {
		DB::get_id_path(c, &mut id_buf); path.push( unsafe { std::str::from_utf8_unchecked(&id_buf) } );

		if path.exists() {
			let _ = fs::remove_file(&path); path.pop();
		} else {
			break;
		}

		c += 1;
	}

	Ok(())
}