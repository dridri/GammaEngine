find_package(ZLIB REQUIRED)

find_path(PNG16_INCLUDE_DIR png.h
  HINTS
    ENV PNG16_DIR
  PATH_SUFFIXES include/png16 include/png1.6 include/libpng16 include/libpng1.6 include
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt
  /usr
  /usr/local
  ${CMAKE_INCLUDE_PATH}
  NO_DEFAULT_PATH
)

if("${PNG16_INCLUDE_DIR}" MATCHES "")
find_path(PNG16_INCLUDE_DIR png.h
  HINTS
    ENV PNG16_DIR
  PATH_SUFFIXES include/png16 include/png1.6 include/libpng16 include/libpng1.6 include
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt
)
endif()

message(STATUS "${CMAKE_INCLUDE_PATH}")

find_library(PNG16_LIBRARY
  NAMES png16 png1.6 png-1.6
  HINTS
    ENV PNG16_DIR
  PATH_SUFFIXES lib
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /sw
  /opt/local
  /opt/csw
  /opt
)

if(PNG16_LIBRARY)
    set( PNG16_LIBRARIES "${PNG16_LIBRARY}" CACHE STRING "PNG Libraries")
endif()

message(STATUS "${PNG16_INCLUDE_DIR}")

if(PNG16_INCLUDE_DIR AND EXISTS "${PNG16_INCLUDE_DIR}/png.h")
  file(STRINGS "${PNG16_INCLUDE_DIR}/png.h" png16_version_str REGEX "^#define[ \t]+PNG_LIBPNG_VER_STRING[ \t]+\".+\"")

  string(REGEX REPLACE "^#define[ \t]+PNG_LIBPNG_VER_STRING[ \t]+\"([^\"]+)\".*" "\\1" PNG16_VERSION_STRING "${png16_version_str}")

  unset(png16_version_str)
endif()

#include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
# handle the QUIETLY and REQUIRED arguments and set PNG16_FOUND to TRUE if
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PNG16
                                  REQUIRED_VARS PNG16_LIBRARIES PNG16_INCLUDE_DIR
                                  VERSION_VAR PNG16_VERSION_STRING)

mark_as_advanced(PNG16_INCLUDE_DIR PNG16_LIBRARIES PNG16_LIBRARY PNG16_MATH_LIBRARY)

