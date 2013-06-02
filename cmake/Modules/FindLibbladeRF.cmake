if(NOT LIBBLADERF_FOUND)
  pkg_check_modules (LIBBLADERF_PKG libbladerf)
  find_path(LIBBLADERF_INCLUDE_DIR NAMES bladerf.h
    PATHS
    ${LIBBLADERF_PKG_INCLUDE_DIRS}
    /usr/include
    /usr/local/include
  )

  find_library(LIBBLADERF_LIBRARIES NAMES bladerf
    PATHS
    ${LIBBLADERF_PKG_LIBRARY_DIRS}
    /usr/lib
    /usr/local/lib
  )

if(LIBBLADERF_INCLUDE_DIR AND LIBBLADERF_LIBRARIES)
  set(LIBBLADERF_FOUND TRUE CACHE INTERNAL "libbladerf found")
  message(STATUS "Found libbladerf: ${LIBBLADERF_INCLUDE_DIR}, ${LIBBLADERF_LIBRARIES}")
else(LIBBLADERF_INCLUDE_DIR AND LIBBLADERF_LIBRARIES)
  set(LIBBLADERF_FOUND FALSE CACHE INTERNAL "libbladerf found")
  message(STATUS "libbladerf not found.")
endif(LIBBLADERF_INCLUDE_DIR AND LIBBLADERF_LIBRARIES)

mark_as_advanced(LIBBLADERF_INCLUDE_DIR LIBBLADERF_LIBRARIES)

endif(NOT LIBBLADERF_FOUND)
