use rsnumerics::{ mat4::Mat4, vector3::Vector3 };

#[derive(Default, Clone, Copy)]
pub struct Camera {
    pub perspective_matrix : Mat4,
    pub perspective_matrix_values : [f32; 16],

    pub view_matrix : Mat4,
    pub view_matrix_values : [f32; 16],

    pub position : Vector3,

    pub field_of_view : f32,
    pub cull_near : f32,
    pub cull_far : f32,
    pub aspect : f32,
}

impl Camera {
    pub fn new(position : Vector3, fov : f32) -> Camera {
        let mut cam = Camera::default();

        cam.position = position;
        cam.field_of_view = fov;
        cam.cull_near = 0.001f32;
        cam.cull_far  = 100.0f32;
        cam.aspect = 1.0f32;

        return cam;
    } 

    pub fn update_matrices(&mut self) {
        self.perspective_matrix = Mat4::from_perspective(self.field_of_view, self.aspect, self.cull_near, self.cull_far);
        self.view_matrix = Mat4::from_eye(self.position);

        self.perspective_matrix_values = self.perspective_matrix.as_arr();
        self.view_matrix_values = self.view_matrix.as_arr();
    }
}

