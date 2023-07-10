use serde::{ Serialize, Deserialize };
use rsnumerics::{ vector3::Vector3, vector2::Vector2 };
use bincode;

use std::mem::size_of;
use std::fs::File;
use std::io::{ BufWriter, Write, BufReader, Read, SeekFrom, Seek };

// A basic prototype vertex structure, used mainly for writing to and from a manta mdl
#[derive(Debug, Default, Clone, Copy, Serialize, Deserialize)]
#[repr(C, packed)]
pub struct MantaMdlVertex {
    pub position : [f32; 3],
    pub normal : [f32; 3],
    pub uv0 : [f32; 2],
}

// The actual model header
#[derive(Debug, Default, Clone, Copy, Serialize, Deserialize)]
#[repr(C, packed)]
pub struct MantaMdlHeader { // aka MMDL, lacks support for skinned meshes
    pub ident: [u8; 4], // Idea from id tech MDL format, we use MMDL as our identifier string
    pub vert_count : u64,
    pub vert_offset : u64,
    pub indice_count : u64,
    pub indice_offset : u64,
    pub name_len : u16,
    pub name_offset : u64,
}

pub struct MantaMdl {
    pub name : String,
    pub header : MantaMdlHeader,
    pub vertices : Vec<MantaMdlVertex>,
    pub indices : Vec<u32>
}

// UTF-8 for MMDL
pub const MMDL_IDENT : [u8; 4] = [ 0x4d, 0x4d, 0x44, 0x4c ];

impl MantaMdl {
    // Updates values within the header to allow for writing
    pub fn update_header(&mut self) {
        self.header.ident = MMDL_IDENT;
        self.header.vert_count = self.vertices.len() as u64;
        self.header.indice_count = self.indices.len() as u64;
        self.header.name_len = self.name.len() as u16;
    }

    pub fn write_to_file(&mut self, path : &str) {
        let mut file = &File::create(path).expect("Failed opening file!");

        // We need to write the initial header
        let initial_header_buf = bincode::serialize(&self.header).expect("Failed to serialize header!");
        file.write(&initial_header_buf).expect("Failed to write header");

        // Vert offset
        self.header.vert_offset = file.seek(SeekFrom::Current(0)).expect("Failed to get vertex offset");

        for vert in &self.vertices {
            let vert_buf = bincode::serialize(vert).expect("Failed serializing a vertex!");
            file.write(&vert_buf).expect("Failed writing a vert");
        }

        // Indice offset
        self.header.indice_offset = file.seek(SeekFrom::Current(0)).expect("Failed to get indice offset");

        for indice in &self.indices {
            let indice_buf = bincode::serialize(indice).expect("Failed serializing an indice!");
            file.write(&indice_buf).expect("Failed writing an indice");
        }

        // Name offset
        self.header.name_offset = file.seek(SeekFrom::Current(0)).expect("Failed to get name offset");
        file.write(&self.name.as_bytes()).expect("Failed writing name string!");

        // Finally write the final header
        file.seek(SeekFrom::Start(0)).expect("Failed to seek back to start of file!");

        let header_buf = bincode::serialize(&self.header).expect("Failed to serialize header!");
        file.write(&header_buf).expect("Failed to write header");
    }

    pub fn from_file(path : &str) -> MantaMdl {
        let mut file = &File::open(path).expect("Failed to open file!");

        let mut header_buf = Vec::new();
        file.take(size_of::<MantaMdlHeader>() as u64).read_to_end(&mut header_buf).expect("Failed reading header");

        let header = bincode::deserialize::<MantaMdlHeader>(&header_buf).expect("Failed to read header");

        file.seek(SeekFrom::Start(header.vert_offset)).expect("Failed seeking to vertice offset");
        let mut vertices = Vec::new();
        for v in 0 .. header.vert_count {
            let mut vert_buf = Vec::new();
            file.take(size_of::<MantaMdlVertex>() as u64).read_to_end(&mut vert_buf).expect("Failed reading a vertex");

            let vert = bincode::deserialize::<MantaMdlVertex>(&vert_buf).expect("Failed deserializing a vertex");
            vertices.push(vert);
        }

        file.seek(SeekFrom::Start(header.indice_offset)).expect("Failed seeking to indice offset");
        let mut indices = Vec::new();
        for i in 0 .. header.indice_count {
            let mut indice_buf = Vec::new();
            file.take(size_of::<u32>() as u64).read_to_end(&mut indice_buf).expect("Failed reading an indice");

            let indice = bincode::deserialize::<u32>(&indice_buf).expect("Failed deserializing an indice");

            indices.push(indice)
        }

        //file.seek(SeekFrom::Start(header.name_offset)).expect("Failed seeking to header name offset");
        let mut name_buf = Vec::new();
        file.take(header.name_len as u64).read_to_end(&mut name_buf).expect("Failed reading header name");
        let name = String::from_utf8(name_buf).expect("Failed reading UTF8 header name!");

        return MantaMdl {
            header: header, 
            name: name, 
            vertices: vertices, 
            indices: indices
        };
    }
}