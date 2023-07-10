pub mod native;

use std::ffi::{CString};
use std::ptr;
use std::option::{Option};

use libc::c_void;

// TODO: Make this not SUPER unsafe!
pub mod internal_callbacks {
    use super::Window;
    use super::native;

    pub unsafe extern "C" fn glfw_framebuffer_resize_callback(window : *mut native::GLFWwindow, width : i32, height : i32) {
        let window_ptr = native::glfwGetWindowUserPointer(window) as *mut Window;
        let target_window = &mut *window_ptr;

        target_window.width = width;
        target_window.height = height;

        target_window.was_resized = true
    }
}

pub struct Window {
    pub native_ptr : *mut native::GLFWwindow,
    pub width : i32,
    pub height : i32,

    pub was_resized : bool
}

impl Window {
    pub fn setup_internal_callbacks(&mut self) {
        unsafe {
            native::glfwSetWindowUserPointer(self.native_ptr, self as *mut Self as *mut c_void);
            native::glfwSetFramebufferSizeCallback(self.native_ptr, Some(internal_callbacks::glfw_framebuffer_resize_callback));
        }
    }

    pub fn should_close(&self) -> bool {
        unsafe { 
            if !self.native_ptr.is_null() { 
                return native::glfwWindowShouldClose(self.native_ptr) != 0; 
            } 
        }

        return false;
    }

    pub fn make_context_current(&self) {
        unsafe { 
            if !self.native_ptr.is_null() { 
                native::glfwMakeContextCurrent(self.native_ptr); 
            } 
        }
    }

    pub fn swap_buffers(&self) {
        unsafe { 
            if !self.native_ptr.is_null() { 
                native::glfwSwapBuffers(self.native_ptr); 
            } 
        }
    }

    pub fn new(width : i32, height : i32, title : &str) -> Option<Window> {
        unsafe {
            // TODO Monitors for fullscreen
            let ctitle = CString::new(title).unwrap();
            let mut_null = ptr::null_mut();
            
            let window_ptr = native::glfwCreateWindow(
                width, 
                height, 
                ctitle.as_bytes_with_nul().as_ptr() as *const i8, 
                mut_null as *mut native::GLFWmonitor, 
                mut_null as *mut native::GLFWwindow
            );
    
            if window_ptr != mut_null as *mut native::GLFWwindow {
                let win = Window { 
                    native_ptr: window_ptr,
                    
                    width: width,
                    height: height,

                    was_resized: true // Force aspect to be calculated
                };

                return Some(win);
            } else {
                return None;
            }
        }
    }
}


const GLFW_OK : i32 = 0;

#[repr(u32)]
pub enum WindowHint {
    ContextVersionMajor = native::GLFW_CONTEXT_VERSION_MAJOR,
    ContextVersionMinor = native::GLFW_CONTEXT_VERSION_MINOR,
    OpenGLProfile = native::GLFW_OPENGL_PROFILE,
    TransparentFramebuffer = native::GLFW_TRANSPARENT_FRAMEBUFFER
}

pub const GLFW_OPENGL_CORE_PROFILE : u32 = native::GLFW_OPENGL_CORE_PROFILE;
pub const GLFW_OPENGL_COMPAT_PROFILE : u32 = native::GLFW_OPENGL_COMPAT_PROFILE;

pub fn glfw_init() -> bool {
    unsafe {
        let state = native::glfwInit() != GLFW_OK;
        return state;
    }
}

pub fn glfw_poll_events() {
    unsafe { native::glfwPollEvents(); }
}

pub fn glfw_window_hint(hint : WindowHint, value : i32) {
    unsafe { native::glfwWindowHint(hint as i32, value); }
}