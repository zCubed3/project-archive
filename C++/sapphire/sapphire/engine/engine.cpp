#include "engine.h"

#include <SDL.h>

#ifdef WIN32
#include <core/platforms/win32_platform.h>
#endif

#include <core/data/string_tools.h>
#include <core/data/timing.h>
#include <core/fs/pak_file_system.h>
#include <engine/assets/asset_loader.h>
#include <engine/assets/material_asset.h>
#include <engine/assets/static_mesh_asset.h>
#include <engine/assets/texture_asset.h>
#include <engine/console/console.h>
#include <engine/rendering/lighting/light.h>
#include <engine/rendering/render_server.h>
#include <engine/rendering/shader.h>
#include <engine/rendering/texture.h>
#include <engine/rendering/texture_render_target.h>
#include <engine/rendering/window_render_target.h>
#include <engine/scene/mesh_actor.h>
#include <engine/scene/world.h>
#include <engine/typing/class_registry.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/quaternion.hpp>

#ifdef RS_OPENGL4_SUPPORT
#include <rs_opengl4/rendering/opengl4_render_server.h>
#endif

#ifdef RS_VULKAN_SUPPORT
#include <rs_vulkan/rendering/vulkan_render_server.h>
#endif

#if defined(IMGUI_SUPPORT)
#include <backends/imgui_impl_sdl.h>
#include <imgui.h>

#include <engine/editor/editor.h>
#endif

void Engine::initialize_configs() {
    platform->create_folder("config");

    //
    // Engine config
    //
    engine_config.set_string("sRenderServer", "Rendering", "Vulkan");

    engine_config.read_from_path("config/engine.secf");
    engine_config.write_to_path("config/engine.secf");
}

bool Engine::initialize_rendering() {
    std::string user_render_server = engine_config.try_get_string("sRenderServer", "Rendering", "Vulkan");

    if (StringTools::compare(user_render_server, "vulkan")) {
#ifdef RS_VULKAN_SUPPORT
        render_server = new VulkanRenderServer();
        return true;
#else
        std::cout << "Error: Vulkan support was not compiled into the engine!" << std::endl;
#endif
    }

    if (StringTools::compare(user_render_server, "opengl4")) {
#ifdef RS_OPENGL4_SUPPORT
        render_server = new OpenGL4RenderServer();
        return true;
#else
        std::cout << "Error: OpenGL 4 support was not compiled into the engine!" << std::endl;
#endif
    }

    if (render_server == nullptr) {
        std::cout << "Error: Unrecognized render server '" << user_render_server << "'!" << std::endl;
        return false;
    }

    return false;
}

bool Engine::create_main_window() {
    std::string main_window_name = get_default_window_title();

    main_window = SDL_CreateWindow(
            main_window_name.c_str(),
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            1280,
            720,
            render_server->get_sdl_window_flags() | SDL_WINDOW_RESIZABLE);

    main_window_rt = new WindowRenderTarget(main_window);
    SDL_SetWindowData(main_window, "RT", main_window_rt);

    SDL_MaximizeWindow(main_window);

    if (!render_server->initialize(main_window)) {
        std::cout << render_server->get_error() << std::endl;
        return false;
    }

    render_server->populate_render_target_data(main_window_rt);

#if defined(IMGUI_SUPPORT)
    render_server->initialize_imgui(main_window_rt);
#endif

    return true;
}

bool Engine::initialize() {
#ifdef WIN32
    platform = Win32Platform::create_win32_platform();
#endif

    timing = Timing::get_singleton();

    AssetLoader::register_engine_asset_loaders();
    Console::register_defaults();

    initialize_configs();

    SDL_Init(SDL_INIT_EVERYTHING);

    if (!initialize_rendering()) {
        return false;
    }

    render_server->register_rs_asset_loaders();

    create_main_window();

    AssetLoader::load_all_placeholders();

#if defined(IMGUI_SUPPORT)
    editor = new Editor();
    editor->initialize(this);
#endif

    return true;
}

bool Engine::shutdown() {
#if defined(IMGUI_SUPPORT)
    editor->shutdown(this);
#endif

    return true;
}

bool Engine::engine_loop() {
    if (render_server == nullptr) {
        return false;
    }

    timing->new_frame();

    // TODO: Update the world properly
    universe.tick(this);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
#if defined(IMGUI_SUPPORT)
        if (SDL_GetWindowFlags(main_window) & SDL_WINDOW_INPUT_FOCUS) {
            ImGui::SetCurrentContext(main_window_rt->imgui_context);
            ImGui_ImplSDL2_ProcessEvent(&event);
        }
#endif

        if (event.type == SDL_QUIT) {
            return false;
        }

        if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
            render_server->on_window_resized(SDL_GetWindowFromID(event.window.windowID));
        }
    }

    render_server->begin_frame();

    // TODO: Temporary lighting
    //light->render_shadows(render_server, world);

    // TODO: If editor
    editor->draw(this);

    render_server->begin_target(main_window_rt);
    render_server->begin_imgui(main_window_rt);

    editor->draw_editor_gui(this);

    render_server->end_imgui(main_window_rt);
    render_server->end_target(main_window_rt);

    render_server->end_frame();

    render_server->present(main_window);

    return true;
}

std::string Engine::get_default_window_title() const {
    std::string name = "Sapphire (";
    name += render_server->get_name();
    name += ")";

#ifdef DEBUG
    name += " - DEBUG BUILD";
#endif

    return name;
}

void Engine::set_window_title(const std::string &title) {
    SDL_SetWindowTitle(main_window, title.c_str());
}

#if defined(DEBUG)
bool Engine::create_test_world() {
    mesh = std::reinterpret_pointer_cast<MeshAsset>(AssetLoader::load_asset("test.obj"));
    material = std::reinterpret_pointer_cast<MaterialAsset>(AssetLoader::load_asset("test.semd"));

    skybox_mesh = std::reinterpret_pointer_cast<MeshAsset>(AssetLoader::load_asset("test_skybox.obj"));
    skybox_material = std::reinterpret_pointer_cast<MaterialAsset>(AssetLoader::load_asset("test_skybox.semd"));

    std::shared_ptr<TextureAsset> cubemap = std::reinterpret_pointer_cast<TextureAsset>(AssetLoader::load_asset("test_cube.setd"));

    world = new World();
    world->name = "TestWorld";
    world->skybox = cubemap;

    universe.add_world(std::shared_ptr<World>(world));

    light = new Light();

    {
        MeshActor *actor = new MeshActor();
        actor->mesh_asset = mesh;
        actor->material_asset = material;

        actors.push_back(actor);
        world->add_actor(actor);
    }

    {
        MeshActor *actor = new MeshActor();
        actor->mesh_asset = skybox_mesh;
        actor->material_asset = skybox_material;

        actors.push_back(actor);
        world->add_actor(actor);
    }

    return true;
}
#endif