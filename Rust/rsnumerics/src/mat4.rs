use super::{vector3::Vector3, vector4::Vector4};

use super::conversions::{DEG_TO_RAD};

#[repr(C, packed)]
#[derive(Debug, Default, Clone, Copy)]
pub struct Mat4 {
    pub row1 : Vector4,
    pub row2 : Vector4,
    pub row3 : Vector4,
    pub row4 : Vector4
}

impl std::ops::Mul for Mat4 {
    type Output = Self;

    fn mul(self, rhs: Self) -> Self::Output {
        let mut final_mat = Mat4::default();
        
        final_mat.row1.x = (self.row1 * rhs.column1()).sum();
        final_mat.row1.y = (self.row1 * rhs.column2()).sum();
        final_mat.row1.z = (self.row1 * rhs.column3()).sum();
        final_mat.row1.w = (self.row1 * rhs.column4()).sum();

        final_mat.row2.x = (self.row2 * rhs.column1()).sum();
        final_mat.row2.y = (self.row2 * rhs.column2()).sum();
        final_mat.row2.z = (self.row2 * rhs.column3()).sum();
        final_mat.row2.w = (self.row2 * rhs.column4()).sum();

        final_mat.row3.x = (self.row3 * rhs.column1()).sum();
        final_mat.row3.y = (self.row3 * rhs.column2()).sum();
        final_mat.row3.z = (self.row3 * rhs.column3()).sum();
        final_mat.row3.w = (self.row3 * rhs.column4()).sum();

        final_mat.row4.x = (self.row4 * rhs.column1()).sum();
        final_mat.row4.y = (self.row4 * rhs.column2()).sum();
        final_mat.row4.z = (self.row4 * rhs.column3()).sum();
        final_mat.row4.w = (self.row4 * rhs.column4()).sum();
        
        return final_mat;
    }
}

impl Mat4 {
    pub fn new(row1 : Vector4, row2 : Vector4, row3 : Vector4, row4 : Vector4) -> Mat4 {
        return Mat4 {
            row1: row1,
            row2: row2,
            row3: row3,
            row4: row4
        }
    }

    pub fn identity() -> Mat4 {
        return Mat4 {
            row1: Vector4::new(1.0, 0.0, 0.0, 0.0),
            row2: Vector4::new(0.0, 1.0, 0.0, 0.0),
            row3: Vector4::new(0.0, 0.0, 1.0, 0.0),
            row4: Vector4::new(0.0, 0.0, 0.0, 1.0)
        }
    }

    // Port of https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/building-basic-perspective-projection-matrix
    pub fn from_perspective(fov_y : f32, aspect : f32, near_cull : f32, far_cull : f32) -> Mat4 {
        let rad = (fov_y / 2.0) * DEG_TO_RAD;
        
        let y_scale = 1f32 / rad.tan();
        let x_scale = y_scale / aspect; 
        let comp = near_cull - far_cull;

        let row1 = Vector4::new(x_scale, 0.0, 0.0, 0.0);
        let row2 = Vector4::new(0.0, y_scale, 0.0, 0.0);
        let row3 = Vector4::new(0.0, 0.0, (far_cull + near_cull) / comp, -1.0);
        let row4 = Vector4::new(0.0, 0.0, 2.0 * far_cull * near_cull / comp, 0.0);

        return Mat4::new(row1, row2, row3, row4);
    }

    pub fn from_eye(eye : Vector3) -> Mat4 {
        let mut final_mat = Mat4::identity();

        final_mat.row4 = Vector4::new(-eye.x, -eye.y, -eye.z, 1.0);

        return final_mat;
    }

    pub fn mul_vec3(&self, point : Vector3) -> Vector3 {
        let mut reference = Vector3::default();
        
        reference.x = point.x * self.row1.x + point.y * self.row2.x + point.z * self.row3.x + self.row4.x;
        reference.y = point.x * self.row1.y + point.y * self.row2.y + point.z * self.row3.y + self.row4.y;
        reference.z = point.x * self.row1.z + point.y * self.row2.z + point.z * self.row3.z + self.row4.z;

        let w = point.x * self.row1.w + point.y * self.row2.w + point.z * self.row3.w + self.row4.w;

        if w != 1f32 {
            reference.x /= w;
            reference.y /= w;
            reference.z /= w;
        }

        return reference;
    }

    pub fn as_arr(&self) -> [f32; 16] {
        return [
            self.row1.x, self.row1.y, self.row1.z, self.row1.w,
            self.row2.x, self.row2.y, self.row2.z, self.row2.w,
            self.row3.x, self.row3.y, self.row3.z, self.row3.w,
            self.row4.x, self.row4.y, self.row4.z, self.row4.w,
        ];
    }

    //
    // TRS constructions
    //
    pub fn trs_vec3(translation : Vector3, rotation : Vector3, scale : Vector3) -> Mat4 {
        return Mat4::scale_vec3(scale) * Mat4::rotation_vec3(rotation) * Mat4::translate_vec3(translation);
    }


    pub fn translate_vec3(translation : Vector3) -> Mat4 {
        let mut clone = Mat4::identity();

        clone.row4 = Vector4::new(translation.x, translation.y, translation.z, 1.0f32);
        
        return clone;
    }

    pub fn scale_vec3(scale : Vector3) -> Mat4 {
        let mut clone = Mat4::identity();

        clone.row1.x = scale.x;
        clone.row2.y = scale.y;
        clone.row3.z = scale.z;

        return clone;
    }

    pub fn rot_x(x: f32) -> Mat4 {
        let mut rot = Mat4::identity();

        rot.row2 = Vector4::new(0.0, x.cos(), -x.sin(), 0.0);
        rot.row3 = Vector4::new(0.0, x.sin(), x.cos(), 0.0);

        return rot
    }

    pub fn rot_y(y: f32) -> Mat4 {
        let mut rot = Mat4::identity();

        rot.row1 = Vector4::new(y.cos(), 0.0, y.sin(), 0.0);
        rot.row3 = Vector4::new(-y.sin(), 0.0, y.cos(), 0.0);

        return rot
    }

    pub fn rot_z(z: f32) -> Mat4 {
        let mut rot = Mat4::identity();

        rot.row1 = Vector4::new(z.cos(), -z.sin(), 0.0, 0.0);
        rot.row2 = Vector4::new(z.sin(), z.cos(), 0.0, 0.0);

        return rot
    }

    pub fn rotation_vec3(rotation : Vector3) -> Mat4 {
        return Mat4::rot_x(rotation.x * DEG_TO_RAD) * Mat4::rot_y(rotation.y * DEG_TO_RAD) * Mat4::rot_z(rotation.z * DEG_TO_RAD);
    }

    // TODO
    pub fn inverse(&self) -> Mat4 {
        return Mat4::default();
    }

    pub fn transpose(&self) -> Mat4 {
        return Mat4 { 
            row1: self.column1(), 
            row2: self.column2(), 
            row3: self.column3(), 
            row4: self.column4() 
        }
    }

    // TODO
    pub fn determinant(&self) -> f32 {
        return 0f32;
    }

    //
    // Columns
    //

    pub fn column1(&self) -> Vector4 {
        return Vector4 {
            x: self.row1.x,
            y: self.row2.x,
            z: self.row3.x,
            w: self.row4.x
        };
    }

    pub fn column2(&self) -> Vector4 {
        return Vector4 {
            x: self.row1.y,
            y: self.row2.y,
            z: self.row3.y,
            w: self.row4.y
        };
    }

    pub fn column3(&self) -> Vector4 {
        return Vector4 {
            x: self.row1.z,
            y: self.row2.z,
            z: self.row3.z,
            w: self.row4.z
        };
    }

    pub fn column4(&self) -> Vector4 {
        return Vector4 {
            x: self.row1.w,
            y: self.row2.w,
            z: self.row3.w,
            w: self.row4.w
        };
    }
}