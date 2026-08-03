set(OpenGL_GL_PREFERENCE "GLVND")

include(CMakeFindDependencyMacro)
find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenMP REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(OPENEXR REQUIRED IMPORTED_TARGET OpenEXR)

include(${CMAKE_CURRENT_LIST_DIR}/StonefishTargets.cmake)