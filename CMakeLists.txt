cmake_minimum_required(VERSION 3.22.1) # Must be 3.22.1 to work with Android
project(sdl_test VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

##########################################################################################
# Add dependencies with FetchContent
##########################################################################################

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

function(add_git_dependency libName gitURL gitTag)
    FetchContent_Declare(${libName}
            GIT_REPOSITORY ${gitURL}
            GIT_TAG ${gitTag}
            GIT_SHALLOW TRUE
            GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable(${libName})
endfunction()

# Add SDL
option(BUILD_SHARED_LIBS "Build SDL2 as a shared library" OFF) # don't build the shared library for SDL
add_git_dependency(
        SDL2
        https://github.com/libsdl-org/SDL.git
        release-2.28.2
)

# Add SDL Image
option(SDL2IMAGE_BUILD_SHARED_LIBS "Build SDL2_Image as a shared library" OFF) # don't build the shared library for SDL_image
option(SDL2IMAGE_INSTALL "" OFF) # disable install target for SDL Image
add_git_dependency(
        SDL2_image
        https://github.com/libsdl-org/SDL_image.git
        release-2.6.3
)

# Add SDL Mixer
option(SDL2MIXER_VENDORED "" ON)
option(SDL2MIXER_BUILD_SHARED_LIBS "Build SDL2_Mixer as a shared library" OFF)
add_git_dependency(
        SDL2_mixer
        https://github.com/libsdl-org/SDL_mixer.git
        release-2.6.3
)

# Add SDL_TTF
option(SDL2TTF_BUILD_SHARED_LIBS "Build SDL2_TTF as a shared library" OFF)
option(SDL2TTF_VENDORED "" ON)
add_git_dependency(
        SDL2_ttf
        https://github.com/libsdl-org/SDL_ttf.git
        release-2.20.2
)

##########################################################################################
# Project executable setup
##########################################################################################

# Adding our source files
# Define PROJECT_SOURCES as a list of all source files
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/sources/*.cpp")

# Define PROJECT_INCLUDE to be the path to the include directory of the project
set(PROJECT_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/sources/")

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE})

target_link_libraries(${PROJECT_NAME} SDL2::SDL2main SDL2::SDL2-static SDL2_image::SDL2_image-static SDL2_mixer::SDL2_mixer-static freetype SDL2_ttf::SDL2_ttf-static)

##########################################################################################
# Project build settings
##########################################################################################

# Set the build type to Debug or Release
add_definitions(-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})

# Set common compiler flags
# set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wswitch")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# Setting ASSETS_PATH
if (CMAKE_BUILD_TYPE MATCHES "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -DDEBUG")

    if (NOT EMSCRIPTEN)
        # Set the asset path macro to the absolute path on the dev machine
        target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_PATH=${CMAKE_CURRENT_SOURCE_DIR}/${resource_dir}/)
    endif ()
else ()
    if (NOT EMSCRIPTEN)
        # Set the asset path macro in release mode to a relative path that assumes the assets folder is in the same directory as the game executable
        target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_PATH=./${resource_dir}/)
    endif ()
endif ()

# Directory that contain resources (i.e. images, fonts, etc)
set(resource_dir assets)

# Build list of resource files
set(resource_files)
file(GLOB resource ${resource_dir}/*)
list(APPEND resource_files ${resource})

# Copy all of the resource files to the destination
file(COPY ${resource_files} DESTINATION "${resource_dir}/")

if (EMSCRIPTEN)
    # Tell Emscripten to build an .html file.
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sUSE_SDL=2 -sASSERTIONS=0 -sWASM=1 -Os -sTOTAL_MEMORY=67108864 -sFORCE_FILESYSTEM=1 -sALLOW_MEMORY_GROWTH=1 -sEXPORTED_RUNTIME_METHODS=cwrap --preload-file ${resource_dir} --shell-file ${CMAKE_CURRENT_LIST_DIR}/sources/minshell.html")
    set(CMAKE_EXECUTABLE_SUFFIX ".html") # This line is used to set your executable to build with the emscripten html template so that you can directly open it.
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")
    target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_PATH=/${resource_dir}/) # Set the asset path macro in release mode to a relative path that assumes the assets folder is in the same directory as the game executable
endif ()
