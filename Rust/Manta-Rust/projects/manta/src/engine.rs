pub mod camera;
pub mod data;
pub mod timing;
pub mod drawing;

use glfw3::*;
use glad::glad_init;

use rsnumerics::color::Color;

pub struct Engine {
    pub active_camera: camera::Camera,
    pub timing: timing::Timing,
    pub window: Window,

    pub clear_color: Option<Color>
}

impl Engine {
    // Please only ever create 1 instance of the engine, multiple instances will break!
    pub fn new(width : i32, height : i32, title : &str) -> Engine {
        if glfw_init() != true {
            panic!("GLFW failed to initialize!");
        }
    
        glfw_window_hint(WindowHint::ContextVersionMajor, 4);
        glfw_window_hint(WindowHint::ContextVersionMinor, 6);
        glfw_window_hint(WindowHint::OpenGLProfile, GLFW_OPENGL_COMPAT_PROFILE as i32);
        glfw_window_hint(WindowHint::TransparentFramebuffer, GLFW_OPENGL_COMPAT_PROFILE as i32);
    
        let mut engine = Engine {
            active_camera: camera::Camera::default(),
            timing: timing::Timing::new(),
            window: Window::new(width, height, title).expect("Failed to create window!"),
            clear_color: None
        };

        engine.window.setup_internal_callbacks();
        engine.window.make_context_current();

        if !glad_init() {
            panic!("GLAD failed to initialize!");
        }

        drawing::set_viewport(0, 0, width, height);
        drawing::set_culling(drawing::CullingMode::Back);
        drawing::set_depth_test(drawing::DepthTestMode::Less);

        return engine;
    }

    pub fn should_shutdown(&self) -> bool {
        return self.window.should_close();
    }

    // Call this before you issue draw calls to the engine
    pub fn begin_render(&mut self) {
        glfw_poll_events();

        if self.window.was_resized {
            drawing::set_viewport(0, 0, self.window.width, self.window.height);
            self.active_camera.aspect = self.window.width as f32 / self.window.height as f32;
        }

        drawing::clear(self.clear_color);
    
        self.active_camera.update_matrices();
    }

    // Call this once you're done submitting drawcalls to flip the swapchain image
    pub fn end_render(&self) {
        self.window.swap_buffers();
    }

    pub fn update(&mut self) {
        self.timing.update();
    }
}