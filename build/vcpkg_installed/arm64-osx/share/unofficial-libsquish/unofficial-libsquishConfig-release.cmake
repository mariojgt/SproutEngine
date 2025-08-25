#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "unofficial::libsquish::squish" for configuration "Release"
set_property(TARGET unofficial::libsquish::squish APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(unofficial::libsquish::squish PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libsquish.a"
  )

list(APPEND _cmake_import_check_targets unofficial::libsquish::squish )
list(APPEND _cmake_import_check_files_for_unofficial::libsquish::squish "${_IMPORT_PREFIX}/lib/libsquish.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
