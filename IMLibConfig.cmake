# - Try to find IMLib
#  Once done this will define
#  IMLIB_FOUND - System has IMLib
#  IMLIB_INCLUDE_DIRS - The IMLib include directories
#  IMLIB_LIBRARIES - The libraries needed to use IMLib

find_path(IMLIB_INCLUDE_DIR IMLib/IMLib.h)
find_library(IMLIB_LIBRARY NAMES IMLib IMLIB HINTS "/usr/local/lib/")

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set IMLIB_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(IMLIB  DEFAULT_MSG
                                  IMLIB_LIBRARY IMLIB_INCLUDE_DIR)

mark_as_advanced(IMLIB_INCLUDE_DIR IMLIB_LIBRARY )

set(IMLIB_LIBRARIES ${IMLIB_LIBRARY} )
set(IMLIB_INCLUDE_DIRS ${IMLIB_INCLUDE_DIR} )