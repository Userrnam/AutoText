use tokenizers::Tokenizer;
use std::ffi::CStr;
use std::os::raw::c_char;
use std::os::raw::c_void;

#[no_mangle]
pub extern "C" fn tokenizer_from_file(input: *const c_char) -> *const c_void {
    unsafe {
        if input.is_null() { return std::ptr::null(); }

        let c_str = CStr::from_ptr(input);
        let name = c_str.to_str().unwrap(); // Convert C string to Rust string

        let tokenizer = Tokenizer::from_file(name.clone());
        if tokenizer.is_err() { return std::ptr::null(); }

        let tokenizer = tokenizer.unwrap();

        let boxed = Box::new(tokenizer);
        Box::into_raw(boxed) as *const c_void
    }
}

#[no_mangle]
pub extern "C" fn free_tokenizer(ptr: *mut Tokenizer) {
    unsafe {
        if !ptr.is_null() {
            let _ = Box::from_raw(ptr);
        }
    }
}

#[no_mangle]
pub extern "C" fn tokenizer_encode(tokenizer: *mut Tokenizer,
                                   input: *const c_char,
                                   result_data: *mut u32,
                                   result_allocated: u32) -> i32 {
    unsafe {
        if tokenizer.is_null() || input.is_null() || result_data.is_null() {
            return -1;
        }

        let c_str = CStr::from_ptr(input);
        let input = c_str.to_str(); // Convert C string to Rust string
        if input.is_err() {
            return -2;
        }
        let input = input.unwrap();

        let tokenizer_ref = &mut *tokenizer;
        let encoding = tokenizer_ref.encode(input, true);
        if encoding.is_err() {
            return -3;
        }
        let encoding = encoding.unwrap();

        let ids = encoding.get_ids();
        let mut i = 0;
        for elem in ids.iter() {
            if i >= result_allocated {
                break;
            }
            *result_data.offset(i as isize) = *elem;
            i += 1;
        }

        ids.len() as i32
    }
}

#[no_mangle]
pub extern "C" fn tokenizer_decode(tokenizer: *mut Tokenizer,
                                   input_data: *const u32,
                                   input_count: u32,
                                   result_data: *mut u8,
                                   result_allocated: u32) -> i32 {
    unsafe {
        if tokenizer.is_null() || input_data.is_null() || result_data.is_null() {
            return -1;
        }

        let tokenizer_ref = &mut *tokenizer;
        let ids = std::slice::from_raw_parts(input_data, input_count as usize).to_vec();
        let rust_string = tokenizer_ref.decode(ids, false);
        if rust_string.is_err() {
            return -2;
        }
        let rust_string = rust_string.unwrap();

        let bytes = rust_string.into_bytes();
        let mut i = 0;
        for byte in &bytes {
            if i >= result_allocated {
                break;
            }
            *result_data.offset(i as isize) = *byte;
            i += 1;
        }

        bytes.len() as i32
    }
}
