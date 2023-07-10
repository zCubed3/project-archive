extern crate cc;

fn main() {
    cc::Build::new()
        .file("src/glad/glad.c")
        .include("src/")
        .compile("glad");
}