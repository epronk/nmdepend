# to get boost_filesystem with cygwin /cygdrive style paths build with
# bjam "-sBUILD=<define>BOOST_POSIX debug release"

INCLUDE (${CMAKE_ROOT}/Modules/CheckIncludeFiles.cmake)

SET(INCLUDES "")
CHECK_INCLUDE_FILES("${INCLUDES};bfd.h" HAVE_BFD_H)
CHECK_INCLUDE_FILES("${INCLUDES};demangle.h" HAVE_DEMANGLE_H)

FIND_PACKAGE(Binutils)
FIND_PACKAGE(ZLIB)
FIND_PACKAGE(Boost 1.48.0 COMPONENTS filesystem program_options system REQUIRED)


SET(CMAKE_CXX_FLAGS "-Wall -W -O2")

IF(COVERAGE)
 # SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
ENDIF(COVERAGE)

IF (CYGWIN)
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_POSIX")
ENDIF (CYGWIN)

INCLUDE_DIRECTORIES(
	${CMAKE_SOURCE_DIR}/src
    ${Boost_INCLUDE_DIR}
#    ${CommonC++_INCLUDE_DIR}
#	${Binutils_INCLUDE_DIR}
)

ADD_LIBRARY(core
  Analyser.cpp
  Bfd.cpp
  Demangler.cpp
  Entity.cpp
  Factory.cpp
  Graph.cpp
  ObjectFile.cpp
  ObjectPackage.cpp
  Symbol.cpp
  SymbolStore.cpp
  )

TARGET_LINK_LIBRARIES(core
	${BFD_LIBRARY}
 	${IBERTY_LIBRARY}
 	${Boost_LIBRARIES}
)

IF(CYGWIN)
  TARGET_LINK_LIBRARIES(core /bin/cygintl-2.dll)
ELSE(CYGWIN)
  TARGET_LINK_LIBRARIES(core ${ZLIB_LIBRARIES} ${CMAKE_DL_LIBS})
ENDIF (CYGWIN)

ADD_EXECUTABLE(nmdepend main.cpp)
TARGET_LINK_LIBRARIES(nmdepend core)

IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(test)
ENDIF(BUILD_TESTING)

INSTALL_TARGETS(/bin nmdepend)
