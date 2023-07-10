#[cfg(windows)]
fn main() {
    println!("cargo:rustc-link-lib=glfw3");
    println!("cargo:rustc-link-search=./lib");
}

#[cfg(unix)]
extern crate pkg_config;

#[cfg(unix)]
fn main() {
    pkg_config::Config::new().probe("glfw3").unwrap();
    //pkg_config::Config::new().probe("GL").unwrap();
}