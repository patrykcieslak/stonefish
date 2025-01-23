#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Stonefish::Stonefish" for configuration "Release"
set_property(TARGET Stonefish::Stonefish APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Stonefish::Stonefish PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libStonefish.so"
  IMPORTED_SONAME_RELEASE "libStonefish.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS Stonefish::Stonefish )
list(APPEND _IMPORT_CHECK_FILES_FOR_Stonefish::Stonefish "${_IMPORT_PREFIX}/lib/libStonefish.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
