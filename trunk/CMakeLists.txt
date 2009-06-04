# The name of our project is "LVL". CMakeLists files in this project can 
# refer to the root source directory of the project as ${LVL_SOURCE_DIR} and 
# to the root binary directory of the project as ${LVL_BINARY_DIR}.

cmake_minimum_required (VERSION 2.4) 

IF(WIN32)
	ADD_DEFINITIONS(-TP)
ENDIF(WIN32)
 
# this specifies the optional clang compiler (llvm + clang must be installed)
# comment this line out to use your default C compiler or choose another
# SET(CMAKE_C_COMPILER_INIT ccc) 

project (LVL)
# SET(CMAKE_INSTALL_PREFIX S:/lvl/trunk/build/ms)  # normally set via cmake GUI.

# Recurse into subdirectories.
# This does not run another cmake. 
# The same process will walk through this list 
# and execute CMakeLists.txt therein:

add_subdirectory (src/lib) 
add_subdirectory (src/progs)
add_subdirectory (include)
add_subdirectory (doc)
