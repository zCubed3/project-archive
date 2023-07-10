use super::native;
use libc::{c_void, c_char};
use std::ffi::{CString};

pub mod shaders;

//
// Macros
//

// Used to help with bringing constants from native into this file
macro_rules! const_from_native {
    ($x:ident, $t:ty) => {
        pub const $x : $t = native::$x;
    };
}

//
// Buffer operations
//
const_from_native!(GL_ELEMENT_ARRAY_BUFFER, u32);
const_from_native!(GL_ARRAY_BUFFER, u32);

pub fn gl_gen_buffer() -> u32 {
    unsafe {
        let gl_gen_buffers_ptr = native::glad_glGenBuffers.unwrap();

        let mut id = 0;
        let id_ptr : *mut u32 = &mut id;

        gl_gen_buffers_ptr(1, id_ptr);
        return id;
    }
}


//
// Index Buffers
//
pub struct IndexBuffer {
    pub id : u32
}

impl IndexBuffer {
    pub fn new() -> IndexBuffer {
        return IndexBuffer { id: gl_gen_buffer() };
    }

    pub fn bind(&self) {
        unsafe {
            let gl_bind_buffer_ptr = native::glad_glBindBuffer.unwrap();
            gl_bind_buffer_ptr(GL_ELEMENT_ARRAY_BUFFER, self.id);
        }
    }

    pub fn set_indices(&self, indices : &Vec<u32>) {
        self.bind();
            
        unsafe {
            let gl_buffer_data_ptr = native::glad_glBufferData.unwrap();
            gl_buffer_data_ptr(GL_ELEMENT_ARRAY_BUFFER, (4 * indices.len()) as i64, indices.as_ptr() as *const c_void, GL_STATIC_DRAW);
        }
    }
}

//
// Vertex Buffers
//
pub const GL_STATIC_DRAW : u32 = native::GL_STATIC_DRAW;

#[derive(Clone, Copy)]
pub struct VertexBuffer {
    pub id : u32
}

impl VertexBuffer {
    pub fn new() -> VertexBuffer {
        unsafe {
            let gl_gen_buffers_ptr = native::glad_glGenBuffers.unwrap();

            let mut id = 0;
            let id_ptr : *mut u32 = &mut id;

            gl_gen_buffers_ptr(1, id_ptr);

            let vbuf = VertexBuffer { id: id };

            return vbuf;
        }
    }

    pub fn bind(&self) {
        unsafe {
            let gl_bind_buffer_ptr = native::glad_glBindBuffer.unwrap();
            gl_bind_buffer_ptr(GL_ARRAY_BUFFER, self.id);
        }
    }

    pub fn set_vertex_data(&self, length : i64, data : *const c_void) {
        unsafe {
            self.bind();
            
            let gl_buffer_data_ptr = native::glad_glBufferData.unwrap();

            #[cfg(windows)]
            gl_buffer_data_ptr(GL_ARRAY_BUFFER, length as i32, data, GL_STATIC_DRAW);

            #[cfg(unix)]
            gl_buffer_data_ptr(GL_ARRAY_BUFFER, length, data, GL_STATIC_DRAW);
        }
    }
}

pub struct VertexArray {
    pub id : u32,
    pub buffers : Vec<VertexBuffer>
}

//
// Vertex Arrays
//

impl VertexArray {
    pub fn new() -> VertexArray {
        unsafe {
            let gl_gen_vertex_arrays_ptr = native::glad_glGenVertexArrays.unwrap();
            
            let mut id = 0;
            let id_ptr : *mut u32 = &mut id;

            gl_gen_vertex_arrays_ptr(1, id_ptr);

            return VertexArray { id: id, buffers: Vec::new() };
        }
    }

    pub fn bind(&self) {
        unsafe {
            let gl_bind_vertex_arrays_ptr = native::glad_glBindVertexArray.unwrap();
            gl_bind_vertex_arrays_ptr(self.id);
        }
    }

    pub fn create_vertex_buffer(&mut self) -> VertexBuffer {
        let vbuf = VertexBuffer::new();
        self.buffers.push(vbuf);

        return vbuf;
    }
}
//
// GL Operations
//
pub const GL_DEPTH_TEST : u32 = native::GL_DEPTH_TEST;
pub const GL_CULL_FACE : u32 = native::GL_CULL_FACE;

pub fn gl_enable(value : u32) {
    unsafe {
        let gl_enable_ptr = native::glad_glEnable.unwrap();
        gl_enable_ptr(value);
    }
}

pub fn gl_disable(value : u32) {
    unsafe {
        let gl_disable_ptr = native::glad_glDisable.unwrap();
        gl_disable_ptr(value);
    }
}

// Depth testing
const_from_native!(GL_LESS, u32);
const_from_native!(GL_GREATER, u32);

pub fn gl_depth_func(value : u32) {
    unsafe {
        let gl_depth_func_ptr = native::glad_glDepthFunc.unwrap();
        gl_depth_func_ptr(value);
    }
}

// Backface culling
const_from_native!(GL_BACK, u32);
const_from_native!(GL_FRONT, u32);

pub fn gl_cull_face(value : u32) {
    unsafe {
        let gl_cull_face_ptr = native::glad_glCullFace.unwrap();
        gl_cull_face_ptr(value);
    }
}

// Clearing
pub const GL_COLOR_BUFFER_BIT : u32 = native::GL_COLOR_BUFFER_BIT;
pub const GL_DEPTH_BUFFER_BIT : u32 = native::GL_DEPTH_BUFFER_BIT;

pub fn gl_clear(value : u32) {
    unsafe {
        let gl_clear_ptr = native::glad_glClear.unwrap();
        gl_clear_ptr(value);
    }
}

pub fn gl_clear_color(r : f32, g : f32, b : f32, a : f32) {
    unsafe {
        let gl_clear_color_ptr = native::glad_glClearColor.unwrap();
        gl_clear_color_ptr(r, g, b, a);
    }
}

pub fn gl_viewport(x : i32, y : i32, width : i32, height : i32) {
    unsafe {
        let gl_viewport_ptr = native::glad_glViewport.unwrap();
        gl_viewport_ptr(x, y, width, height);
    }
}

pub const GL_VERSION : u32 = native::GL_VERSION; 

pub fn gl_get_string(value : u32) -> String {
    unsafe {
        let gl_get_string_ptr = native::glad_glGetString.unwrap();
        let cstring_ptr : *const c_char = gl_get_string_ptr(value) as *const c_char;
        let cstr = std::ffi::CStr::from_ptr(cstring_ptr);

        return String::from(cstr.to_str().unwrap());
    }
}

//
// Draw calls
//
pub const GL_FLOAT : u32 = native::GL_FLOAT;
pub const GL_UNSIGNED_INT : u32 = native::GL_UNSIGNED_INT;

pub fn gl_enable_vertex_attrib_array(index : u32) {
    unsafe {
        let gl_enable_vertex_attrib_array_ptr = native::glad_glEnableVertexAttribArray.unwrap();
        gl_enable_vertex_attrib_array_ptr(index);
    }
}

pub fn gl_vertex_attrib(index : u32, size : i32, data_type : u32, normal : bool, stride : i32, offset : u32) {
    unsafe {
        let gl_vertex_attrib_ptr = native::glad_glVertexAttribPointer.unwrap();

        let offset_void  = offset as *const c_void; 

        gl_vertex_attrib_ptr(index, size, data_type, normal as u8, stride, offset_void);
    }
}

// Drawing modes
pub const GL_TRIANGLES : u32 = native::GL_TRIANGLES;

pub fn gl_draw_arrays(mode : u32, start : i32, count : i32) {
    unsafe {
        let gl_draw_arrays_ptr = native::glad_glDrawArrays.unwrap();
        gl_draw_arrays_ptr(mode, start, count);
    }
}

pub fn gl_draw_elements(mode : u32, indice_count : i32) {
    unsafe {
        let gl_draw_elements_ptr = native::glad_glDrawElements.unwrap();
        gl_draw_elements_ptr(mode, indice_count, GL_UNSIGNED_INT, std::ptr::null());
    }
}

pub fn gl_uniform_matrix_4fv(location : i32, mat4 : *const f32) {
    unsafe {
        let gl_uniform_matrix_4fv_ptr = native::glad_glUniformMatrix4fv.unwrap();
        gl_uniform_matrix_4fv_ptr(location, 1, 0, mat4);
    }
}

// 
// Shader Programs
//
pub const GL_VERTEX_SHADER : u32 = native::GL_VERTEX_SHADER; 
pub const GL_FRAGMENT_SHADER : u32 = native::GL_FRAGMENT_SHADER; 

pub fn gl_create_shader(shader_type : u32) -> u32 {
    unsafe {
        let gl_create_shader_ptr = native::glad_glCreateShader.unwrap();
        return gl_create_shader_ptr(shader_type);
    }
}

// Allows for multiple sources so we can prepend data to the top of the shader :)
pub fn gl_shader_source(shader_id : u32, sources : Vec<&str>) {
    unsafe {
        let gl_shader_source_ptr = native::glad_glShaderSource.unwrap();

        let mut sources_ptrs : Vec<*const i8> = Vec::new();
        let mut sources_cstrs : Vec<CString> = Vec::new();

        for index in 0 .. sources.len() {
            sources_cstrs.push(CString::new(sources[index]).unwrap());
        }

        // Maybe a less tired me can figure out something better...
        for index in 0 .. sources.len() {
            sources_ptrs.push(sources_cstrs[index].as_ptr());
        }

        gl_shader_source_ptr(shader_id, sources.len() as i32, sources_ptrs.as_ptr(), std::ptr::null());
    }
}

pub fn gl_compile_shader(shader_id : u32) {
    unsafe {
        let gl_shader_compile_ptr = native::glad_glCompileShader.unwrap();
        gl_shader_compile_ptr(shader_id);
    }
}

pub fn gl_create_program() -> u32 {
    unsafe {
        let gl_create_program_ptr = native::glad_glCreateProgram.unwrap();
        return gl_create_program_ptr();
    }
}

pub fn gl_attach_shader(program : u32, shader : u32) {
    unsafe {
        let gl_attach_shader_ptr = native::glad_glAttachShader.unwrap();
        gl_attach_shader_ptr(program, shader);
    }
}

pub fn gl_link_program(program : u32) {
    unsafe {
        let gl_link_program_ptr = native::glad_glLinkProgram.unwrap();
        gl_link_program_ptr(program);
    }
}


pub fn gl_detach_shader(program : u32, shader : u32) {
    unsafe {
        let gl_detach_shader_ptr = native::glad_glDetachShader.unwrap();
        gl_detach_shader_ptr(program, shader);
    }
}

pub fn gl_use_program(program : u32) {
    unsafe {
        let gl_use_program_ptr = native::glad_glUseProgram.unwrap();
        gl_use_program_ptr(program);
    }
}

const_from_native!(GL_COMPILE_STATUS, u32);
const_from_native!(GL_INFO_LOG_LENGTH, u32);

pub fn gl_get_shader_iv(shader : u32, value : u32, result : &mut i32) {
    unsafe {
        let gl_get_shader_iv_ptr = native::glad_glGetShaderiv.unwrap();
        gl_get_shader_iv_ptr(shader, value, result as *mut i32);
    }
}

pub fn gl_get_shader_error_log(shader : u32) -> String {
    unsafe {
        let mut log_length = 0;
        gl_get_shader_iv(shader, GL_INFO_LOG_LENGTH, &mut log_length);

        let gl_get_shader_info_log_ptr = native::glad_glGetShaderInfoLog.unwrap();
        let mut raw_log : Vec<u8> = vec![0; log_length as usize];

        gl_get_shader_info_log_ptr(shader, log_length, std::ptr::null_mut::<i32>(), raw_log.as_mut_ptr() as *mut i8);

        let log = String::from_utf8(raw_log).unwrap().replace("\0", "");
        return log;
    }
}

const_from_native!(GL_INVALID_VALUE, u32);

pub fn gl_get_uniform_location(program : u32, name : &str) -> Option<i32> {
    unsafe {
        let gl_get_uniform_location_ptr = native::glad_glGetUniformLocation.unwrap();

        let cstr = CString::new(name).unwrap();
        let uniform = gl_get_uniform_location_ptr(program, cstr.as_ptr());

        if uniform == GL_INVALID_VALUE as i32 {
            return None;
        } else {
            return Some(uniform);
        }
    }
}