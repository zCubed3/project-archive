mod native;

use glfw3::native::{glfwGetProcAddress};

pub fn glad_init() -> bool {
    unsafe { 
        let addr = glfwGetProcAddress;
        return native::gladLoadGLLoader(Some(addr)) == 1;
    }
}

pub fn glad_get_version() -> [i32; 2] {
    unsafe {
        return [ native::GLVersion.major, native::GLVersion.minor ];
    }
}

pub mod opengl;