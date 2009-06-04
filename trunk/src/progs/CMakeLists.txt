include_directories (${LVL_SOURCE_DIR}/include)
link_directories (${LVL_BINARY_DIR}/lib)

# Add executable that is built from the source files 
# The extensions are automatically found.

if(COMMAND cmake_policy)
      cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)    

SET(SOURCES newef tobw quarter contrast report reflect asc2vis vis2asc tocol paint bnd2img)
FOREACH(source ${SOURCES})
	ADD_EXECUTABLE(${source} ${source}.c)
	IF(WIN32)
	 target_link_libraries (${source} lvl)
	ELSE()
    target_link_libraries (${source} lvl m)
	ENDIF(WIN32) 
ENDFOREACH(source)

INSTALL(TARGETS ${SOURCES} RUNTIME DESTINATION bin)
INSTALL(PROGRAMS batchef DESTINATION bin)
