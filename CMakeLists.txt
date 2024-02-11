cmake_minimum_required(VERSION 3.10)
project(Stonefish VERSION 1.3.0)

# Automatic configuration
configure_file(version.h.in version.h)

# Options
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
option(BUILD_TESTS "Build applications testing different features of the Stonefish library" OFF)
option(EMBED_RESOURCES "Embed internal resources in the library executable" OFF)

# Compile flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -ffast-math")
set(OpenGL_GL_PREFERENCE "GLVND")

# Find required libraries
find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenMP REQUIRED)

# Generate C++ code from all resource files (optional)
set(RESOURCES) # This variable stores array of generated resource files
if(EMBED_RESOURCES)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ResourceManager) # Build resource manager 
    file(GLOB_RECURSE RESOURCE_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/Library/shaders/*.*") # Find resource files
    rm_embed_resources(RESOURCES ${RESOURCE_FILES}) # Generate C++ code
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DEMBEDDED_RESOURCES -DRM_NO_EXCEPTIONS")
    add_definitions(-DSHADER_DIR_PATH=\"Library/shaders/\") #Sets shader path for resources
    include_directories(${RESOURCE_MANAGER_INCLUDE_DIRS})
endif()

# Add include directories
include_directories(
    ${PROJECT_BINARY_DIR}
    Library/include
    3rdparty
    ${OPENGL_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
)

# Find library source files
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Library/src/*.cpp")
file(GLOB_RECURSE SOURCES_3RD "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/*.c")

# List dependecies
set(LIBRARIES ${FREETYPE_LIBRARIES} ${OPENGL_LIBRARIES} ${SDL2_LIBRARIES})
if(OpenMP_CXX_FOUND)
    set(LIBRARIES ${LIBRARIES} ${OpenMP_CXX_LIBRARIES})
endif()

# Define targets
if(BUILD_TESTS)
    # Create tests and use library locally (has to be disabled when installing system-wide!)
    add_library(Stonefish_test SHARED
        ${SOURCES} 
        ${SOURCES_3RD} 
        ${RESOURCES}
    )
    target_link_libraries(Stonefish_test PUBLIC
        ${LIBRARIES}
    )
    target_compile_definitions(Stonefish_test PUBLIC 
        BT_EULER_DEFAULT_ZYX 
        BT_USE_DOUBLE_PRECISION
    )
    if(NOT EMBED_RESOURCES)
        #Sets shader path for the library
        target_compile_definitions(Stonefish_test PUBLIC
            SHADER_DIR_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/Library/shaders/\"
        )
    endif()
    add_subdirectory(Tests)
else()
    # Create shared library to be installed system-wide
    add_library(Stonefish SHARED 
        ${SOURCES} 
        ${SOURCES_3RD} 
        ${RESOURCES}
    )
    target_link_libraries(Stonefish PUBLIC
        ${LIBRARIES}
    )
    target_include_directories(Stonefish PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Library/include>"
        "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include/${CMAKE_PROJECT_NAME}>"
    )
    target_compile_definitions(Stonefish PUBLIC 
        BT_EULER_DEFAULT_ZYX 
        BT_USE_DOUBLE_PRECISION
    )
    if(NOT EMBED_RESOURCES)
        #Sets shader path for the library
        target_compile_definitions(Stonefish PUBLIC
            SHADER_DIR_PATH=\"${CMAKE_INSTALL_PREFIX}/share/Stonefish/shaders/\"
        ) 
    endif()

    # Install library in the system
    install(TARGETS Stonefish 
        EXPORT StonefishTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
    )
    
    # Install Stonefish headers
    install(FILES ${PROJECT_BINARY_DIR}/version.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${CMAKE_PROJECT_NAME}) # Version header
    install(DIRECTORY Library/include/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${CMAKE_PROJECT_NAME})
    
    # Install 3rdparty headers
    file(GLOB_RECURSE INCLUDES_3RD "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/*.h")
    foreach(f ${INCLUDES_3RD})
        file(RELATIVE_PATH frel "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty" ${f})
        get_filename_component(dir ${frel} DIRECTORY)
        install(FILES ${f} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/${CMAKE_PROJECT_NAME}/${dir})
    endforeach()
    
    # Install shaders
    if(NOT EMBED_RESOURCES)
        install(DIRECTORY Library/shaders/ DESTINATION ${CMAKE_INSTALL_PREFIX}/share/${CMAKE_PROJECT_NAME}/shaders) # Resources
    endif()

    # Install configuration scripts
    configure_file(stonefish.pc.in stonefish.pc)
    install(FILES ${PROJECT_BINARY_DIR}/stonefish.pc DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig) # Pkg-config configuration

    install(EXPORT StonefishTargets
        FILE StonefishTargets.cmake
        NAMESPACE Stonefish::
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/${CMAKE_PROJECT_NAME}
    )
    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(
        "StonefishConfigVersion.cmake"
        VERSION ${Stonefish_VERSION}
        COMPATIBILITY ExactVersion
    )
    install(FILES 
        "StonefishConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/StonefishConfigVersion.cmake" 
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/${CMAKE_PROJECT_NAME}
    )
endif()
