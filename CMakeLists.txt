
cmake_minimum_required(VERSION 2.8.10 FATAL_ERROR)
project(t576)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
unset(CMAKE_MODULE_PATH)


find_package(GSL REQUIRED)
#use the ROOT things
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})

find_package(ROOT REQUIRED COMPONENTS RIO Tree MathMore)
include(${ROOT_USE_FILE})
#add_definitions(${ROOT_CXX_FLAGS})

if(DEFINED ENV{RUTIL_SOURCE_DIR})
  set(RUTIL_INCLUDE_DIR $ENV{RUTIL_SOURCE_DIR}/include)
else()
  set(RUTIL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
endif()


if(DEFINED ENV{RUTIL_INSTALL_DIR})
  set(RUTIL_INSTALL_DIR $ENV{RUTIL_INSTALL_DIR})
  set(LIB_INSTALL_DIR $ENV{RUTIL_INSTALL_DIR}/lib)
else()
  set(RUTIL_INSTALL_DIR /usr/local/)
  set(LIB_INSTALL_DIR /usr/local/lib)
endif()




include_directories(${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/include ${ROOT_INCLUDE_DIRS} ${RUTIL_INCLUDE_DIR} ${RUTIL_INCLUDE_DIR}/../)

#----------------------------------------------------------------------------




file(GLOB SOURCE_FILES ${PROJECT_SOURCE_DIR}/src/*.cc ${PROJECT_SOURCE_DIR}/src/*.cpp )
file(GLOB HEADER_FILES ${RUTIL_INCLUDE_DIR}/*.h ${RUTIL_INCLUDE_DIR}/*.hh)# ${GSL_INCLUDE_DIRS}/gsl/*.h)


#make the ROOT dictionaries and shared object library for reading the event tree

ROOT_GENERATE_DICTIONARY(G__RUtil ${HEADER_FILES} LINKDEF ${PROJECT_SOURCE_DIR}/LinkDef.h)


add_library(RUtil SHARED ${SOURCE_FILES} G__RUtil.cxx)


target_link_libraries(RUtil ${ROOT_LIBRARIES})

#----------------------------------------------------------------------------



install (FILES ${RUTIL_INCLUDE_DIR}/RUtil.hh DESTINATION ${RUTIL_INSTALL_DIR}/include/RUtil)


install (FILES ${PROJECT_BINARY_DIR}/libRUtil_rdict.pcm DESTINATION ${LIB_INSTALL_DIR})
install (TARGETS RUtil LIBRARY DESTINATION ${LIB_INSTALL_DIR})








