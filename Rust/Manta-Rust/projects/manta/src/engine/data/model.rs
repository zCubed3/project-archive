use rsnumerics::{vector2::Vector2, vector3::Vector3};

use glad::opengl::{VertexBuffer, VertexArray, IndexBuffer};
use glad::opengl;

use std::mem;

use libc::c_void;

use std::fs;
use std::io::{BufRead, BufReader};

use manta_formats::mmdl;

#[repr(C, packed)]
#[derive(Clone, Copy, Default)]
pub struct Vertex {
    pub position : Vector3,
    pub normal : Vector3,
    pub uv : Vector2
}

impl Vertex {
    pub fn new(position : Vector3, normal : Vector3, uv : Vector2) -> Vertex {
        return Vertex {
            position: position,
            normal: normal,
            uv: uv,
        }
    }
}

impl std::cmp::PartialEq for Vertex {
    fn eq(&self, other : &Self) -> bool {
        return self.position == other.position && self.normal == other.normal && self.uv == other.uv;
    }
}

pub struct Model {
    pub vertices : Vec<Vertex>,
    pub triangles : Vec<u32>,

    pub vao : VertexArray,
    pub ibo : IndexBuffer,
    pub vbo : Option<VertexBuffer>
}

impl Model {
    pub fn new() -> Model {
        let mut mdl = Model {
            vertices: Vec::new(),
            triangles: Vec::new(),

            vao: VertexArray::new(),
            ibo: IndexBuffer::new(),
            vbo: None
        };

        mdl.vbo = Some(mdl.vao.create_vertex_buffer());

        return mdl;
    }

    fn update_vbo(&self) {
        let vbo = self.vbo.unwrap();

        let vertex_size = mem::size_of::<Vertex>() as i64;
        let length = self.vertices.len() as i64 * vertex_size;

        let mut vert_ptrs : Vec<*const Vertex> = Vec::new();

        for index in 0 .. self.vertices.len() {
            vert_ptrs.push(&self.vertices[index]);
        }
        
        let data_ptr = self.vertices.as_ptr();
        vbo.set_vertex_data(length, data_ptr as *const c_void);
    }

    fn update_ibo(&self) {
        self.ibo.set_indices(&self.triangles);
    }

    pub fn update_buffers(&self) {
        self.update_vbo();
        self.update_ibo();
    }

    pub fn draw(&self) {
        self.vao.bind();
        self.vbo.unwrap().bind();
        self.ibo.bind();

        opengl::gl_enable_vertex_attrib_array(0);
        opengl::gl_enable_vertex_attrib_array(1);
        opengl::gl_enable_vertex_attrib_array(2);

        opengl::gl_vertex_attrib(0, 3, opengl::GL_FLOAT, false, 4 * 8, 0);
        opengl::gl_vertex_attrib(1, 3, opengl::GL_FLOAT, false, 4 * 8, 4 * 3);
        opengl::gl_vertex_attrib(2, 2, opengl::GL_FLOAT, false, 4 * 8, 4 * 3);

        opengl::gl_draw_elements(opengl::GL_TRIANGLES, self.triangles.len() as i32);
    }

    // Loads a model from a given mmdl file
    pub fn load_mmdl(path : &str) -> Model {
        let mmdl = mmdl::MantaMdl::from_file(path);
        let mut mdl = Model::new();

        for vert in mmdl.vertices {
            mdl.vertices.push(Vertex::new(Vector3::from_array(vert.position), Vector3::from_array(vert.normal), Vector2::from_array(vert.uv0)));
        }

        for indice in mmdl.indices {
            mdl.triangles.push(indice);
        }

        mdl.update_buffers();

        return mdl;
    }

    // Loads a model from a given obj file
    pub fn load_obj(path : &str) -> Model {
        let mut mdl = Model::new();
        
        let file = fs::File::open(path).unwrap();
        let reader = BufReader::new(file);

        let mut positions : Vec<Vector3> = Vec::new(); 
        let mut normals : Vec<Vector3> = Vec::new(); 
        let mut uvs : Vec<Vector2> = Vec::new(); 

        let mut faces : Vec<[u32; 3]> = Vec::new();

        for line in reader.lines() {
            let content = line.unwrap();
            
            if content.starts_with("v ") {
                let mut components : Vec<f32> = Vec::new();
                for word in content.split(" ") {
                    let float_parse = word.parse::<f32>();

                    if float_parse.is_ok() {
                        components.push(float_parse.unwrap());
                    }
                }

                if components.len() == 3 {
                    let pos = Vector3::new(components[0], components[1], components[2]);
                    positions.push(pos);
                }
            }

            if content.starts_with("vn") {
                let mut components : Vec<f32> = Vec::new();
                for word in content.split(" ") {
                    let float_parse = word.parse::<f32>();

                    if float_parse.is_ok() {
                        components.push(float_parse.unwrap());
                    }
                }

                if components.len() == 3 {
                    let normal = Vector3::new(components[0], components[1], components[2]);
                    normals.push(normal);
                }
            }

            if content.starts_with("vt") {
                let mut components : Vec<f32> = Vec::new();
                for word in content.split(" ") {
                    let float_parse = word.parse::<f32>();

                    if float_parse.is_ok() {
                        components.push(float_parse.unwrap());
                    }
                }

                if components.len() == 2 {
                    let uv = Vector2::new(components[0], components[1]);
                    uvs.push(uv);
                }
            }

            if content.starts_with("f ") {
                let raw_face = content.replace("f ", "");
            
                for raw_indices in raw_face.split(" ") {
                    let mut face : [u32; 3] = [ 0, 0, 0 ];
                    let mut last_index = 0;
                    for indice in raw_indices.split("/") {
                        let indice_parse = indice.parse::<u32>();

                        if indice_parse.is_ok() {
                            face[last_index] = indice_parse.unwrap() - 1;
                            last_index += 1;
                        }
                    }

                    faces.push(face);
                }
            }
        }

        for face in faces {
            let position = positions[face[0] as usize];
            let uv = uvs[face[1] as usize];
            let normal = normals[face[2] as usize];

            let vertex = Vertex::new(position, normal, uv);

            let mut similar = false;
            for test_vertex_index in 0 .. mdl.vertices.len() {
                let test_vertex = mdl.vertices[test_vertex_index];
                if vertex == test_vertex {
                    similar = true;
                    mdl.triangles.push(test_vertex_index as u32);
                    break;
                }
            }

            if !similar {
                mdl.triangles.push(mdl.vertices.len() as u32);
                mdl.vertices.push(vertex);
            }
        }

        mdl.update_buffers();
        return mdl;
    }
}