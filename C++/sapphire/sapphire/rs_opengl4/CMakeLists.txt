set(SAPPHIRE_RS_OPENGL4_SOURCES
        "glad/src/glad.c"

        "rendering/opengl4_render_server.cpp"
        "rendering/opengl4_render_target_data.cpp"
        "rendering/opengl4_mesh_buffer.cpp"
        "rendering/opengl4_graphics_buffer.cpp"
        "rendering/opengl4_shader.cpp"
        "rendering/opengl4_texture.cpp"
        "rendering/opengl4_material.cpp"
        )

set(SAPPHIRE_RS_OPENGL4_INCLUDE_DIRS
        "glad/include"
        ${CODE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${THIRD_PARTY_DIR}/SDL/include
        ${THIRD_PARTY_DIR}/glm/glm
        ${THIRD_PARTY_DIR}/imgui
)

if (IMGUI_SUPPORT)
    list(APPEND SAPPHIRE_RS_OPENGL4_SOURCES "${THIRD_PARTY_DIR}/imgui/backends/imgui_impl_opengl3.cpp")
    list(APPEND SAPPHIRE_RS_OPENGL4_INCLUDE_DIRS "${THIRD_PARTY_DIR}/imgui")
endif()

add_library(SapphireOpenGL4RS STATIC ${SAPPHIRE_RS_OPENGL4_SOURCES})

target_include_directories(SapphireOpenGL4RS PUBLIC ${SAPPHIRE_RS_OPENGL4_INCLUDE_DIRS})

target_compile_definitions(SapphireOpenGL4RS PUBLIC ${SAPPHIRE_DEFINITIONS})

target_compile_options(SapphireOpenGL4RS PUBLIC ${SAPPHIRE_COMPILE_OPTIONS})

set(PRELUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/preludes)
set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(GEN_SCRIPT ${SCRIPT_DIR}/gen_resource_header.py)

add_custom_target(SAPPHIRE_RS_OPENGL4_GEN_HEADERS
        py ${GEN_SCRIPT} ${PRELUDE_DIR}/vert_prelude.glsl ${PRELUDE_DIR}/vert_prelude.glsl.gen.h mode=text &&
        py ${GEN_SCRIPT} ${PRELUDE_DIR}/frag_prelude.glsl ${PRELUDE_DIR}/frag_prelude.glsl.gen.h mode=text &&
        py ${GEN_SCRIPT} ${SHADER_DIR}/error.glsl ${SHADER_DIR}/error.glsl.gen.h mode=text
        )

add_dependencies(SapphireOpenGL4RS SAPPHIRE_RS_OPENGL4_GEN_HEADERS)