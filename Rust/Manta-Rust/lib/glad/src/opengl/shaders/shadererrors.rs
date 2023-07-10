use std::fmt;

#[derive(Debug, Clone)]
pub struct ShaderCompilationError {
    pub reason : String
}

impl ShaderCompilationError {
    pub fn new(reason : &str) -> ShaderCompilationError {
        return ShaderCompilationError { reason: reason.to_string() };
    }
}

impl fmt::Display for ShaderCompilationError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        return write!(f, "Shader failed to compile: {}", self.reason);
    }
}

#[derive(Debug, Clone)]
pub struct ShaderLinkageError {
    pub reason : String
}

impl ShaderLinkageError {
    pub fn new(reason : &str) -> ShaderLinkageError {
        return ShaderLinkageError { reason: reason.to_string() };
    }
}

impl fmt::Display for ShaderLinkageError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        return write!(f, "Shader failed to link: {}", self.reason);
    }
}