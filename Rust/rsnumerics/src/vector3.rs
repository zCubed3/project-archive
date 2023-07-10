use super::vector4::Vector4;

#[repr(C, packed)]
#[derive(Debug, Default, Clone, Copy)]
pub struct Vector3 {
    pub x : f32,
    pub y : f32,
    pub z : f32
}

impl Vector3 {
    pub fn new(x : f32, y : f32, z : f32) -> Vector3 {
        return Vector3 { x: x, y: y, z: z };
    }

    pub fn from_single(single : f32) -> Vector3 {
        return Vector3 { x: single, y: single, z: single }
    }

    pub fn to_vec4(&self) -> Vector4 {
        return Vector4 { x: self.x, y: self.y, z: self.z, w: 1.0f32 };
    }

    // Because I am just a lowly high school student who's stunted in math
    // https://www.tutorialspoint.com/cplusplus-program-to-compute-cross-product-of-two-vectors
    pub fn cross(&self, rhs : &Self) -> Vector3 {
        return Vector3 { 
            x: self.y * rhs.z - self.z * rhs.y, 
            y: -(self.x * rhs.z - self.z * rhs.x), 
            z: self.x * rhs.y - self.y - rhs.x 
        };
    }

    pub fn as_array(&self) -> [f32; 3] {
        return [ self.x, self.y, self.z ];
    }

    pub fn from_array(arr : [f32; 3]) -> Vector3 {
        return Vector3::new(arr[0], arr[1], arr[2]);
    }

    pub const ONE : Vector3 = Vector3 { x: 1.0, y: 1.0, z: 1.0 };
    pub const ZERO : Vector3 = Vector3 { x: 0.0, y: 0.0, z: 0.0 };
}

impl std::cmp::PartialEq for Vector3 {
    fn eq(&self, other: &Vector3) -> bool {
        return self.x == other.x && self.y == other.y && self.z == other.z;
    }
}