#INCLUDE(UseLATEX.cmake)
#ADD_LATEX_DOCUMENT(lvl.tex DEFAULT_PDF)

# Build the manual .pdf file with two runs of pdflatex

# if(${CMAKE_CURRENT_SOURCE_DIR}/doc/lvl.tex IS_NEWER_THAN ${CMAKE_CURRENT_SOURCE_DIR}/doc/lvl.pdf)
# add_custom_target( 
#       pdf ALL
#       COMMAND pdflatex lvl.tex >pdflog
#       COMMAND pdflatex lvl.tex >pdflog
#       WORKING_DIRECTORY doc
#       DEPENDS lvl.tex
#       COMMENT building pdflatex manual
# 		SOURCES lvl.tex)
# INSTALL(FILES lvl.pdf DESTINATION doc)
# endif(${CMAKE_CURRENT_SOURCE_DIR}/doc/lvl.tex IS_NEWER_THAN ${CMAKE_CURRENT_SOURCE_DIR}/doc/lvl.pdf)

#EXECUTE_PROCESS(COMMAND pdflatex lvl.tex OUTPUT_QUIET WORKING_DIRECTORY doc)
#EXECUTE_PROCESS(COMMAND pdflatex lvl.tex OUTPUT_QUIET WORKING_DIRECTORY doc)

# Installation commands
if(${CMAKE_CURRENT_SOURCE_DIR}/doc/license.txt IS_NEWER_THAN ${CMAKE_INSTALL_PREFIX}/doc/license.txt)
INSTALL(FILES license.txt DESTINATION doc)
endif(${CMAKE_CURRENT_SOURCE_DIR}/doc/license.txt IS_NEWER_THAN ${CMAKE_INSTALL_PREFIX}/doc/license.txt)
INSTALL(FILES lvl.pdf DESTINATION doc)