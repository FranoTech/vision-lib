include_directories (${LVL_SOURCE_DIR}/include)
SET(LIBSRCS iolib.c utilib.c improclib.c newedgelib.c )
# Create a library called "lvl" which includes the source files listed
IF(WIN32)
	add_library (lvl STATIC ${LIBSRCS})
ELSE(WIN32)
   add_library (lvl SHARED ${LIBSRCS})
ENDIF(WIN32)

# Install it (make install) as a shared library
install(TARGETS lvl
		LIBRARY DESTINATION lib${LIB_SUFFIX}
		ARCHIVE DESTINATION lib${LIB_SUFFIX}
		RUNTIME DESTINATION bin )
