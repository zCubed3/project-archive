project(VAL)

set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED true)

find_package(Vulkan REQUIRED)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/VulkanMemoryAllocator)

set(VAL_SOURCES
        "val/val_releasable.cpp"
        "val/val_instance.cpp"
        "val/val_extension.cpp"
        "val/val_layer.cpp"
        "val/val_queue.cpp"
        "val/data/val_buffer.cpp"
        "val/data/val_staging_buffer.cpp"
        #"val/data/val_pooled_buffer.cpp"
        "val/images/val_image.cpp"
        "val/pipelines/val_render_pass.cpp"
        "val/pipelines/val_render_pass_builder.cpp"
        "val/pipelines/val_descriptor_set.cpp"
        "val/pipelines/val_descriptor_set_info.cpp"
        "val/pipelines/val_descriptor_set_builder.cpp"
        "val/pipelines/val_pipeline.cpp"
        "val/pipelines/val_pipeline_builder.cpp"
        "val/pipelines/val_shader_module.cpp"
        "val/pipelines/val_vertex_input_builder.cpp"
        "val/render_targets/val_render_target.cpp"
        "val/render_targets/val_image_render_target.cpp"
        "val/render_targets/val_window_render_target.cpp"
        )

set(VAL_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/VulkanMemoryAllocator/include
        ${Vulkan_INCLUDE_DIRS}
        )

set(VAL_LINK_LIBRARIES
        ${Vulkan_LIBRARY}
        VulkanMemoryAllocator
        )

# TODO: Make this optional
set(VAL_SDL_SUPPORT ON)

add_library(VAL STATIC ${VAL_SOURCES})
target_link_libraries(VAL PUBLIC ${VAL_LINK_LIBRARIES})
target_include_directories(VAL PUBLIC ${VAL_INCLUDE_DIRS})

if (VAL_SDL_SUPPORT)
    message("Building VAL with SDL support!")
    target_include_directories(VAL PUBLIC ${VAL_SDL_PATH})
    target_compile_definitions(VAL PUBLIC SDL_SUPPORT)
endif()

if (VAL_DEBUG)
    target_compile_definitions(VAL PUBLIC DEBUG)
endif()

set(VAL_INCLUDE_DIRS ${VAL_INCLUDE_DIRS} PARENT_SCOPE)