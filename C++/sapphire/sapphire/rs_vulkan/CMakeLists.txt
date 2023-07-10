set(VAL_SDL_PATH ${THIRD_PARTY_DIR}/SDL/include)

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(VAL_DEBUG ON)
endif()

add_subdirectory(VAL)

set(SAPPHIRE_RS_VULKAN_SOURCES
        "rendering/vulkan_render_server.cpp"
        "rendering/vulkan_render_target_data.cpp"
        "rendering/vulkan_graphics_buffer.cpp"
        "rendering/vulkan_mesh_buffer.cpp"
        "rendering/vulkan_shader.cpp"
        "rendering/vulkan_texture.cpp"
        "rendering/vulkan_material.cpp"
        )

set(SAPPHIRE_RS_VULKAN_INCLUDE_DIRS
        ${CODE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${THIRD_PARTY_DIR}/SDL/include
        ${THIRD_PARTY_DIR}/glm/glm
        ${VAL_INCLUDE_DIRS}
)

if (IMGUI_SUPPORT)
    list(APPEND SAPPHIRE_RS_VULKAN_SOURCES "${THIRD_PARTY_DIR}/imgui/backends/imgui_impl_vulkan.cpp")
    list(APPEND SAPPHIRE_RS_VULKAN_INCLUDE_DIRS "${THIRD_PARTY_DIR}/imgui")
endif()

add_library(SapphireVulkanRS STATIC ${SAPPHIRE_RS_VULKAN_SOURCES})

target_include_directories(SapphireVulkanRS PUBLIC ${SAPPHIRE_RS_VULKAN_INCLUDE_DIRS})

target_link_libraries(SapphireVulkanRS PUBLIC VAL)

target_compile_definitions(SapphireVulkanRS PUBLIC ${SAPPHIRE_DEFINITIONS})

target_compile_options(SapphireVulkanRS PUBLIC ${SAPPHIRE_COMPILE_OPTIONS})

set(PRELUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/preludes)
set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(COMPILE_SCRIPT ${SCRIPT_DIR}/compile_vulkan_glsl.py)
set(MERGE_SCRIPT ${SCRIPT_DIR}/merge_spv_vert_frag.py)
set(GEN_SCRIPT ${SCRIPT_DIR}/gen_resource_header.py)

add_custom_target(SAPPHIRE_RS_VULKAN_GEN_HEADERS
        py ${COMPILE_SCRIPT} ${SHADER_DIR}/error.vert.spv vert ${PRELUDE_DIR}/vert_prelude.glsl ${SHADER_DIR}/error.glsl &&
        py ${COMPILE_SCRIPT} ${SHADER_DIR}/error.frag.spv frag ${PRELUDE_DIR}/frag_prelude.glsl ${SHADER_DIR}/error.glsl &&
        py ${GEN_SCRIPT} ${SHADER_DIR}/error.vert.spv ${SHADER_DIR}/error.spv.vert.gen.h &&
        py ${GEN_SCRIPT} ${SHADER_DIR}/error.frag.spv ${SHADER_DIR}/error.spv.frag.gen.h &&

        py ${COMPILE_SCRIPT} ${SHADER_DIR}/depth_pass.vert.spv vert ${PRELUDE_DIR}/vert_prelude.glsl ${SHADER_DIR}/depth_pass.glsl &&
        py ${COMPILE_SCRIPT} ${SHADER_DIR}/depth_pass.frag.spv frag ${PRELUDE_DIR}/frag_prelude.glsl ${SHADER_DIR}/depth_pass.glsl &&
        py ${GEN_SCRIPT} ${SHADER_DIR}/depth_pass.vert.spv ${SHADER_DIR}/depth_pass.spv.vert.gen.h &&
        py ${GEN_SCRIPT} ${SHADER_DIR}/depth_pass.frag.spv ${SHADER_DIR}/depth_pass.spv.frag.gen.h
        )

add_dependencies(SapphireVulkanRS SAPPHIRE_RS_VULKAN_GEN_HEADERS)