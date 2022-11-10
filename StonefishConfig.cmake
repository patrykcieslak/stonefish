set(OpenGL_GL_PREFERENCE "GLVND")

include(CMakeFindDependencyMacro)
find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenMP REQUIRED)

include(${CMAKE_CURRENT_LIST_DIR}/StonefishTargets.cmake)