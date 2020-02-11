#[cfg(test)]
mod tests {
    #[test]
    fn it_works() {
        assert_eq!(2 + 2, 4);
    }
}


#[no_mangle]
#[allow(non_snake_case)]
pub extern "C" fn rust_foo() -> i32
{
    let a: i32 = 5;
    a
}
