cmake_minimum_required(VERSION 3.10)
project(Stonefish)

# Options
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

option(BUILD_TESTS "Build applications testing different features of the Stonefish library" OFF)

# Set up CMAKE flags
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-Wall -Wno-reorder -DBT_EULER_DEFAULT_ZYX -DBT_USE_DOUBLE_PRECISION")
set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Find required libraries
find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
find_package(Freetype REQUIRED)
find_package(Bullet 2.88 EXACT REQUIRED)

# Add include directories
include_directories(
    Library/include
    ${OPENGL_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
    ${BULLET_INCLUDE_DIRS}
)

# Build list of library source files
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Library/src/*.cpp")

# Define targets
if(BUILD_TESTS)
    # Create tests and use library locally (has to be disabled when installing system-wide!)
    add_library(Stonefish_test SHARED ${SOURCES})
    target_link_libraries(Stonefish_test ${BULLET_LIBRARIES} ${FREETYPE_LIBRARIES} ${OPENGL_LIBRARIES} ${SDL2_LIBRARIES})
    add_definitions(-DSHADER_DIR_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/Library/shaders/\") #Modifies shader path of the library
    add_subdirectory(Tests)
else()
    # Create shared library to be installed system-wide
    add_library(Stonefish SHARED ${SOURCES})
    target_link_libraries(Stonefish ${BULLET_LIBRARIES} ${FREETYPE_LIBRARIES} ${OPENGL_LIBRARIES} ${SDL2_LIBRARIES})

    # Install library in the system
    install(
        TARGETS Stonefish 
        LIBRARY DESTINATION /usr/local/lib
    )

    install(DIRECTORY Library/include/ DESTINATION /usr/local/include/Stonefish)
    install(DIRECTORY Library/shaders/ DESTINATION /usr/local/share/Stonefish/shaders)
    install(FILES Library/stonefish.pc DESTINATION /usr/local/lib/pkgconfig)
endif()