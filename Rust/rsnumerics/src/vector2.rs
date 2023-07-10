#[repr(C, packed)]
#[derive(Debug, Default, Clone, Copy)]
pub struct Vector2 {
    pub x : f32,
    pub y : f32
}

impl Vector2 {
    pub fn new(x : f32, y : f32) -> Vector2 {
        return Vector2 { x: x, y: y };
    }

    pub fn as_array(&self) -> [f32; 2] {
        return [ self.x, self.y ];
    }

    pub fn from_array(arr : [f32; 2]) -> Vector2 {
        return Vector2::new(arr[0], arr[1]);
    }
}

impl std::cmp::PartialEq for Vector2 {
    fn eq(&self, other: &Vector2) -> bool {
        return self.x == other.x && self.y == other.y;
    }
}