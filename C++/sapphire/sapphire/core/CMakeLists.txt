set(SAPPHIRE_CORE_SOURCES
        "data/color.cpp"
        "data/rect.cpp"
        "data/string_tools.cpp"
        "data/size_tools.cpp"
        "data/timing.cpp"

        "config/config_file.cpp"

        "fs/virtual_file_system.cpp"
        "fs/pak_file_system.cpp"

        "platforms/platform.cpp"
        )

if (WIN32)
    list(APPEND SAPPHIRE_CORE_SOURCES "platforms/win32_platform.cpp")
endif()

add_library(SapphireCore STATIC ${SAPPHIRE_CORE_SOURCES})

target_include_directories(SapphireCore PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CODE_DIR}
        )

target_compile_definitions(SapphireCore PUBLIC
        ${SAPPHIRE_DEFINITIONS}
        )

target_compile_options(SapphireCore PUBLIC
        ${SAPPHIRE_COMPILE_OPTIONS}
        )