cmake_minimum_required(VERSION 3.14)
project(sapphire)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${CMAKE_CXX_COMPILER_ID}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_BUILD_TYPE}/")
set(CMAKE_PDB_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

set(ROOT_DIR ${CMAKE_SOURCE_DIR})
set(SCRIPT_DIR ${CMAKE_SOURCE_DIR}/scripts)
set(CODE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sapphire)
set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

#
# Options
#

# TODO: Replace this with "headless mode"
option(RS_OPENGL4_SUPPORT "Compiles and links OpenGL4 Render Server support into the engine" ON)
option(RS_VULKAN_SUPPORT "Compiles and links Vulkan Render Server support into the engine" ON)

#set(RS_OPENGL4_SUPPORT OFF)
#set(RS_VULKAN_SUPPORT OFF)

option(IMGUI_SUPPORT "Compiles ImGui support into the engine" ON)

#set(IMGUI_SUPPORT OFF)

# TODO: Allow disabling SDL?

option(DISABLE_RTTI "Disables RTTI support (in favor of the engine type system)" ON)

#
# Compiler things
#

# TODO: Handle this for Clang and GCC
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    message("Compiling with MSVC")


    set(SAPPHIRE_COMPILE_OPTIONS)
    set(SAPPHIRE_DEFINITIONS "_HAS_EXCEPTIONS=0" "_CRT_SECURE_NO_WARNINGS")
endif()

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    message("Debug build!")
    list(APPEND SAPPHIRE_DEFINITIONS "DEBUG")
endif()

#
# Projects
#

# Core: Depends on nothing but the OS
add_subdirectory(${CODE_DIR}/core)

# TODO: Null driver to remove SDL dependency?
add_subdirectory(${THIRD_PARTY_DIR}/SDL)
list(APPEND SAPPHIRE_DEFINITIONS "SDL_SUPPORT" "GLM_FORCE_DEPTH_ZERO_TO_ONE")

if (DISABLE_RTTI)
    if (WIN32)
        string(REPLACE "/GR" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    endif()
endif()

if (IMGUI_SUPPORT)
    list(APPEND SAPPHIRE_DEFINITIONS IMGUI_SUPPORT)
endif()

if (RS_OPENGL4_SUPPORT)
    message("Compiling Sapphire with OpenGL4 Render Server support!")

    list(APPEND ENGINE_LINK_LIBRARIES SapphireOpenGL4RS)
    list(APPEND ENGINE_DEFINITIONS RS_OPENGL4_SUPPORT)

    add_subdirectory(${CODE_DIR}/rs_opengl4)
endif()

if (RS_VULKAN_SUPPORT)
    message("Compiling Sapphire with Vulkan Render Server support!")

    list(APPEND ENGINE_LINK_LIBRARIES SapphireVulkanRS)
    list(APPEND ENGINE_DEFINITIONS RS_VULKAN_SUPPORT)

    add_subdirectory(${CODE_DIR}/rs_vulkan)
endif()

add_subdirectory(${CODE_DIR}/engine)
