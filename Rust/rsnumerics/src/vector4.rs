#[repr(C, packed)]
#[derive(Debug, Default, Clone, Copy)]
pub struct Vector4 {
    pub x : f32,
    pub y : f32,
    pub z : f32,
    pub w : f32
}

impl std::ops::Mul for Vector4 {
    type Output = Self;

    fn mul(self, rhs: Self) -> Self::Output {
        return Vector4 {
            x: self.x * rhs.x,
            y: self.y * rhs.y,
            z: self.z * rhs.z,
            w: self.w * rhs.w,
        }
    }
}

impl Vector4 {
    pub fn new(x : f32, y : f32, z : f32, w : f32) -> Vector4 {
        return Vector4 { x: x, y: y, z: z, w: w };
    }

    // Takes all the components and returns the sum of them
    // This is particularly useful for matrix math
    pub fn sum(&self) -> f32 {
        return self.x + self.y + self.z + self.w;
    }
}