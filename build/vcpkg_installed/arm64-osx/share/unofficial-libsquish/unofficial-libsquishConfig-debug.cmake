#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "unofficial::libsquish::squish" for configuration "Debug"
set_property(TARGET unofficial::libsquish::squish APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(unofficial::libsquish::squish PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/debug/lib/libsquishd.a"
  )

list(APPEND _cmake_import_check_targets unofficial::libsquish::squish )
list(APPEND _cmake_import_check_files_for_unofficial::libsquish::squish "${_IMPORT_PREFIX}/debug/lib/libsquishd.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
