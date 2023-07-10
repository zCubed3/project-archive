#[cfg(windows)]
fn main() {
    println!("cargo:rustc-link-search=./libraries/glfw3/lib");
    println!("cargo:rustc-link-lib=gdi32");
    println!("cargo:rustc-link-lib=kernel32");
    println!("cargo:rustc-link-lib=user32");
    println!("cargo:rustc-link-lib=shell32");
    println!("cargo:rustc-link-lib=glfw3");
}

#[cfg(unix)]
extern crate pkg_config;

#[cfg(unix)]
fn main() {
    pkg_config::Config::new().probe("glfw3").unwrap();
}