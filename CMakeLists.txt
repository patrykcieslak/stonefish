cmake_minimum_required(VERSION 2.6)
project(Stonefish)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -DBT_EULER_DEFAULT_ZYX -DBT_USE_DOUBLE_PRECISION")

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
find_package(Freetype REQUIRED)
find_package(Bullet 2.88 EXACT REQUIRED)

include_directories(
    Library/include
    ${OPENGL_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
    ${BULLET_INCLUDE_DIRS}
)

file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Library/src/*.cpp")

add_library(Stonefish SHARED ${SOURCES})
target_link_libraries(Stonefish ${BULLET_LIBRARIES} ${FREETYPE_LIBRARIES} ${OPENGL_LIBRARIES} ${SDL2_LIBRARIES} GLEW)

install(
    TARGETS Stonefish 
    LIBRARY DESTINATION /usr/local/lib
)

install(DIRECTORY Library/include/ DESTINATION /usr/local/include/Stonefish)
install(DIRECTORY Library/shaders/ DESTINATION /usr/local/share/Stonefish/shaders)
install(FILES Library/stonefish.pc DESTINATION /usr/local/lib/pkgconfig)
