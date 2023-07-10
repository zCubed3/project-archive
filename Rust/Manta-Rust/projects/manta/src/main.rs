pub mod engine;
use engine::Engine;

use rsnumerics::{color::Color, vector3::Vector3, mat4::Mat4};
use engine::camera::Camera;

use engine::{data::model::Model, data::shader::VertFragShader};

use manta_formats::mmdl;

fn main() {
    let mut engine = &mut Engine::new(1280, 720, "Manta Rust");

    //engine.clear_color = Some(Color::from_hex("#00000000").unwrap());
    engine.clear_color = Some(Color::from_hex("#1A237E").unwrap());

    let mdl = Model::load_obj("test.obj");

    let shader = VertFragShader::compile_from_file("test.glsl").unwrap();

    let mut mat_test = Mat4::identity();
    engine.active_camera = Camera::new(Vector3::default(), 90.0f32);

    let mut mmdl = mmdl::MantaMdl {
        header: mmdl::MantaMdlHeader::default(), 
        name: String::from("TestMdl"), 
        vertices: Vec::new(), 
        indices: Vec::new()
    };

    for vert in &mdl.vertices {
        let mut mvert = mmdl::MantaMdlVertex { position: vert.position.as_array(), normal: vert.normal.as_array(), uv0: vert.uv.as_array() };
        mmdl.vertices.push(mvert);
    }

    for indice in &mdl.triangles {
        mmdl.indices.push(*indice);
    }

    mmdl.update_header();
    mmdl.write_to_file("Test.mmdl");
    
    let mmdl_read = Model::load_mmdl("Test.mmdl");

    loop {
        engine.begin_render();

        let m_values = mat_test.as_arr();

        shader.draw_with(m_values, engine);

        //mdl.draw();
        mmdl_read.draw();

        engine.end_render();

        engine.update();

        if engine.should_shutdown() { break; }

        mat_test = Mat4::trs_vec3(Vector3::new(engine.timing.elapsed.cos(), engine.timing.elapsed.sin(), -2f32), 
            Vector3::ZERO,
            Vector3::ONE
        )
    }
}
