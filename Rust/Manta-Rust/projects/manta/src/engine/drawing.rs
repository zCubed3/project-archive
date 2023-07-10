use glad::opengl;

use rsnumerics::color::Color;

// Clears the frame with an optionally given color
pub fn clear(color : Option<Color>) {
    if color.is_some() { 
        let real_color = color.unwrap();
        opengl::gl_clear_color(real_color.r, real_color.g, real_color.b, real_color.a); 
    }

    opengl::gl_clear(opengl::GL_COLOR_BUFFER_BIT | opengl::GL_DEPTH_BUFFER_BIT);
}

pub fn set_viewport(x : i32, y : i32, width : i32, height : i32) {
    opengl::gl_viewport(x, y, width, height);
}

pub enum RenderFeature {
    FaceCulling,
    DepthTesting
}

pub fn set_feature_enabled(feature : RenderFeature, state : bool) {
    let feature_id = match feature {
        RenderFeature::FaceCulling => opengl::GL_CULL_FACE,
        RenderFeature::DepthTesting => opengl::GL_DEPTH_TEST
    };

    if state {
        opengl::gl_enable(feature_id);
    } else {
        opengl::gl_disable(feature_id);
    }
}

#[repr(u32)]
#[derive(Debug, Clone, Copy)]
pub enum CullingMode {
    Off = 0,
    Back = opengl::GL_BACK,
    Front = opengl::GL_FRONT
}

pub fn set_culling(mode : CullingMode) {
    set_feature_enabled(RenderFeature::FaceCulling, mode as u32 != 0);
    opengl::gl_cull_face(mode as u32);
}

#[repr(u32)]
#[derive(Debug, Clone, Copy)]
pub enum DepthTestMode {
    Off = 0,
    Less = opengl::GL_LESS,
    Greater = opengl::GL_GREATER
}

pub fn set_depth_test(mode : DepthTestMode) {
    set_feature_enabled(RenderFeature::DepthTesting, mode as u32 != 0);
    opengl::gl_depth_func(mode as u32);
}