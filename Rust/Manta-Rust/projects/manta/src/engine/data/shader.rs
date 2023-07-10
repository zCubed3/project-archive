use glad::opengl::shaders::{vertfrag::VertFragShaderPrototype};
use glad::opengl;

use std::fmt;

use super::super::Engine;

#[derive(Debug, Default)]
pub struct VertFragShader {
    pub program : u32,

    pub uniform_matrix_p : i32,
    pub uniform_matrix_v : i32,
    pub uniform_matrix_m : i32,

    pub uniform_camera_pos : i32,
}

use std::{fs, io::Read};

#[derive(Debug, Clone)]
pub struct VertFragShaderCompileError {
    pub reason : String
}

impl VertFragShaderCompileError {
    pub fn new(reason : &str) -> VertFragShaderCompileError {
        return VertFragShaderCompileError { reason: reason.to_string() };
    }
}

impl fmt::Display for VertFragShaderCompileError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        return write!(f, "Shader failed to compile: {}", self.reason);
    }
}

impl VertFragShader {
    pub fn compile_from_file(path : &str) -> Result<VertFragShader, VertFragShaderCompileError> {
        let vert_header = "#version 330\n#define VERT\n";
        let frag_header = "#version 330\n#define FRAG\n";

        let mut file = fs::File::open(path).unwrap();

        let mut source = String::new();
        file.read_to_string(&mut source).expect("Failed reading source file!");
        
        let mut prototype = VertFragShaderPrototype::new();

        prototype.source(vec![vert_header, source.as_str()], vec![frag_header, source.as_str()]);
        let compile_result = prototype.compile();

        if compile_result.is_err() {
            return Err(VertFragShaderCompileError::new(compile_result.unwrap_err().reason.as_str()));
        }

        let link_result = prototype.link();

        if link_result.is_ok() {
            let mut shader = VertFragShader::default();
            shader.program = link_result.unwrap();

            shader.populate_uniforms();

            return Ok(shader);
        } else {
            return Err(VertFragShaderCompileError::new(link_result.unwrap_err().reason.as_str())); // Forward the error
        }
    }

    pub fn populate_uniforms(&mut self) {
        self.uniform_matrix_p = opengl::gl_get_uniform_location(self.program, "MANTA_MAT_P").expect("Failed to find perspective uniform!");
        self.uniform_matrix_v = opengl::gl_get_uniform_location(self.program, "MANTA_MAT_V").expect("Failed to find view uniform!");
        self.uniform_matrix_m = opengl::gl_get_uniform_location(self.program, "MANTA_MAT_M").expect("Failed to find model uniform!");
    
        self.uniform_camera_pos = opengl::gl_get_uniform_location(self.program, "MANTA_CAMERA_POS").expect("Failed to find camera position!");
    }

    pub fn draw_with(&self, model_mat : [f32; 16], engine : &Engine) {
        opengl::gl_use_program(self.program);

        opengl::gl_uniform_matrix_4fv(self.uniform_matrix_p, engine.active_camera.perspective_matrix_values.as_ptr());
        opengl::gl_uniform_matrix_4fv(self.uniform_matrix_v, engine.active_camera.view_matrix_values.as_ptr());
        opengl::gl_uniform_matrix_4fv(self.uniform_matrix_m, model_mat.as_ptr());

        //opengl::gl_uniform_matrix_3fv(self.uniform_matrix_m, model_mat.as_ptr());
    }
}