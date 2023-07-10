set(SOURCES
        "${DEPENDS_DIR}/imgui/imgui.cpp"
        "${DEPENDS_DIR}/imgui/imgui_draw.cpp"
        "${DEPENDS_DIR}/imgui/imgui_demo.cpp"
        "${DEPENDS_DIR}/imgui/imgui_tables.cpp"
        "${DEPENDS_DIR}/imgui/imgui_widgets.cpp"
        "${DEPENDS_DIR}/imgui/backends/imgui_impl_sdl.cpp"
        "${DEPENDS_DIR}/imgui/backends/imgui_impl_opengl3.cpp"

        "${COMMON_DIR}/data/meshes/obj.cpp"
        "${COMMON_DIR}/data/meshes/mbsm.cpp"
        "${COMMON_DIR}/data/meshes/mmdl.cpp"

        "${ENGINE_DIR}/src/world/timing.cpp"
        "${ENGINE_DIR}/src/world/world.cpp"
        "${ENGINE_DIR}/src/world/actor.cpp"
        "${ENGINE_DIR}/src/world/behavior.cpp"
        "${ENGINE_DIR}/src/world/transform.cpp"

        "${ENGINE_DIR}/src/world/behaviors/camera.cpp"
        "${ENGINE_DIR}/src/world/behaviors/light.cpp"

        "${ENGINE_DIR}/src/rendering/renderer.cpp"
        "${ENGINE_DIR}/src/rendering/viewport.cpp"
        "${ENGINE_DIR}/src/rendering/render_target.cpp"
        "${ENGINE_DIR}/src/rendering/lighting.cpp"

        "${ENGINE_DIR}/src/assets/shader.cpp"
        "${ENGINE_DIR}/src/assets/mesh.cpp"
        "${ENGINE_DIR}/src/assets/texture.cpp"

        "${ENGINE_DIR}/src/data/console/console.cpp"

        "${ENGINE_DIR}/src/data/engine_context.cpp"

        "${ENGINE_DIR}/src/input/axis.cpp"
        "${ENGINE_DIR}/src/input/input_server.cpp"

        "src/behaviors/freecam.cpp"

        "src/game/gamegamemodule.cpp"

        "src/main.cpp"
)

set(INCLUDE_DIRS
    "${ENGINE_DIR}/src"
    "${GAME_DIR}/src"
    "${DEPENDS_DIR}/glm"
    "${DEPENDS_DIR}/imgui"
    "${DEPENDS_DIR}/MikkTSpace"
    "${COMMON_DIR}"
)

set(LINK_DIRS)

if (DEFINED WIN32)
    set(LINK_DIRS "${DEPENDS_DIR}/openvr/lib/win64" "${DEPENDS_DIR}/vulkan/lib" "${DEPENDS_DIR}/SDL2/lib/x64" "${DEPENDS_DIR}/glew/lib/Release/x64" "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
    set(LIBRARIES "SDL2" "SDL2main" "glew32" "opengl32")

    set(INCLUDE_DIRS ${INCLUDE_DIRS} "${DEPENDS_DIR}/SDL2/include" "${DEPENDS_DIR}/glew/include")
else ()
    set(LINK_DIRS "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
    set(LIBRARIES "SDL2" "SDL2main" "GL" "GLEW")
    set(INCLUDE_DIRS ${INCLUDE_DIRS})
endif ()

# TODO: Come up with a name for the damn game!
add_library(MantaGame SHARED ${SOURCES})
target_include_directories(MantaGame PUBLIC ${INCLUDE_DIRS})
target_link_directories(MantaGame PUBLIC ${LINK_DIRS})
target_link_libraries(MantaGame PUBLIC ${LIBRARIES})

set_target_properties(MantaGame PROPERTIES
        OUTPUT_NAME "game"
)
