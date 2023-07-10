use std::fmt;

#[repr(C, packed)]
#[derive(Default, Clone, Copy)]
// Colors are basically just reskinned Vector4's with some different behavior
pub struct Color {
    pub r : f32,
    pub g : f32,
    pub b : f32,
    pub a : f32
}

#[derive(Debug, Clone)]
pub struct HexParseError {
    pub reason : String
}

impl HexParseError {
    pub fn new(reason : &str) -> HexParseError {
        return HexParseError { reason: reason.to_string() };
    }
}

impl fmt::Display for HexParseError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        return write!(f, "Couldn't parse hex for reason: {}", self.reason);
    }
}


impl Color {
    pub fn new(r : f32, g : f32, b : f32, a : Option<f32>) -> Color {
        let mut real_a = 1.0f32;

        if a.is_some() { real_a = a.unwrap() }
        
        return Color {
            r: r,
            g: g,
            b: b,
            a: real_a
        }
    }

    // Converts rgba u8's to an RGBA populated color
    pub fn from_rgba(r : u8, g : u8, b : u8, a : u8) -> Color {
        return Color {
            r: r as f32 / 255.0f32,
            g: g as f32 / 255.0f32,
            b: b as f32 / 255.0f32,
            a: a as f32 / 255.0f32
        }
    }

    // Constructs a color from a string of hexidecimal values
    pub fn from_hex(hex : &str) -> Result<Color, HexParseError> {
        // If this starts with a hashtag we don't care
        let sanitized_hex = hex.trim_start_matches("#");

        if sanitized_hex.len() >= 6 {
            let r = u8::from_str_radix(&sanitized_hex[0..2], 16).expect("Failed to parse red channel!");
            let g = u8::from_str_radix(&sanitized_hex[2..4], 16).expect("Failed to parse green channel!");
            let b = u8::from_str_radix(&sanitized_hex[4..6], 16).expect("Failed to parse blue channel!");

            if sanitized_hex.len() == 8 {
                let a = u8::from_str_radix(&sanitized_hex[6..8], 16).expect("Failed to parse alpha channel!");
                return Ok(Color::from_rgba(r, g, b, a));
            }

            if sanitized_hex.len() == 6 {
                return Ok(Color::from_rgba(r, g, b, 255));
            }
        }

        if sanitized_hex.len() == 8 {

        }

        return Err(HexParseError::new("Hex was either malformed or in an unknown format!"));
    }
}