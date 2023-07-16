cmake_minimum_required(VERSION 3.25)
project(sdl_test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

##########################################################################################
# Add dependencies with CPM
##########################################################################################

# Fetch CPM

set(CPM_DOWNLOAD_VERSION 0.38.2)
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
if (NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
    message(STATUS "Downloading CPM.cmake v${CPM_DOWNLOAD_VERSION}")
    file(DOWNLOAD https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake ${CPM_DOWNLOAD_LOCATION})
endif ()
include(${CPM_DOWNLOAD_LOCATION})

# Add dependencies
CPMAddPackage("gh:fmtlib/fmt#10.0.0")
CPMAddPackage(
        GITHUB_REPOSITORY erincatto/box2d
        VERSION 2.4.1
        OPTIONS "BOX2D_BUILD_TESTBED OFF" "BOX2D_BUILD_UNIT_TESTS OFF" "BOX2D_BUILD_DOCS OFF" "BOX2D_BUILD_EXAMPLES OFF")

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

target_link_libraries(${PROJECT_NAME} fmt::fmt box2d)

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
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sUSE_SDL=2 -sASSERTIONS=1 -sWASM=1 -Os -Wall -sTOTAL_MEMORY=67108864 -sFORCE_FILESYSTEM=1 --preload-file ${resource_dir} --shell-file ../sources/minshell.html")
    set(CMAKE_EXECUTABLE_SUFFIX ".html") # This line is used to set your executable to build with the emscripten html template so that you can directly open it.
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")
    target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_PATH=/${resource_dir}/) # Set the asset path macro in release mode to a relative path that assumes the assets folder is in the same directory as the game executable
endif ()
