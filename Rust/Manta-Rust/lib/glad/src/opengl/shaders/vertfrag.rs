use super::super::*; // This is the opengl.rs file
use super::shadererrors::ShaderCompilationError;

pub struct VertFragShaderPrototype {
    pub vert_id : u32,
    pub frag_id : u32
}

impl VertFragShaderPrototype {
    pub fn new() -> VertFragShaderPrototype {
        return VertFragShaderPrototype {
            vert_id: gl_create_shader(GL_VERTEX_SHADER),
            frag_id: gl_create_shader(GL_FRAGMENT_SHADER)
        }
    }

    pub fn source(&self, vert_sources : Vec<&str>, frag_sources : Vec<&str>) {
        gl_shader_source(self.vert_id, vert_sources);
        gl_shader_source(self.frag_id, frag_sources);
    }

    pub fn compile(&self) -> Result<(), ShaderCompilationError> {
        let mut shader_compile_state : i32 = 0;
        
        gl_compile_shader(self.vert_id);
        gl_get_shader_iv( self.vert_id, GL_COMPILE_STATUS, &mut shader_compile_state);

        if shader_compile_state != 1 {
            let reason = format!("Vertex shader failed to compile for reason: {}", gl_get_shader_error_log(self.vert_id));
            return Err(ShaderCompilationError::new(reason.as_str()));
        }

        gl_compile_shader(self.frag_id);
        gl_get_shader_iv( self.frag_id, GL_COMPILE_STATUS, &mut shader_compile_state);

        if shader_compile_state != 1 {
            let reason = format!("Fragment shader failed to compile for reason: {}", gl_get_shader_error_log(self.frag_id));
            return Err(ShaderCompilationError::new(reason.as_str()));
        }

        return Ok(());
    }

    pub fn link(&mut self) -> Result<u32, ShaderCompilationError> {
        let program = gl_create_program();

        gl_attach_shader(program, self.vert_id);
        gl_attach_shader(program, self.frag_id);

        gl_link_program(program);

        gl_detach_shader(program, self.vert_id);
        gl_detach_shader(program, self.frag_id); 

        return Ok(program);
    }
}