cmake_minimum_required(VERSION 3.1.1)

set (CMAKE_CXX_STANDARD 11)

set(PROJECT_NAME switched-tag-uart-to-pc-renderer)
project (${PROJECT_NAME} LANGUAGES C CXX)

option(DATAPATH "Path to data Directory" ON)

#-DCMAKE_BUILD_TYPE=Release
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -Wall -DDATADIR=${DATAPATH} -fPIE")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -Wall -DDATADIR=${DATAPATH} -fPIE")


#SDL2 specific stuff and its add on libraries
INCLUDE(FindPkgConfig)

#pkg_check_modules(openal REQUIRED openal)

# Let CMake search for a raylib-config.cmake
find_package(raylib 3.5 QUIET) 


set ( PROJECT_LINK_LIBS  raylib)


#For the shared library:

link_directories( /usr/lib /usr/local/lib)

 
 

set ( PROJECT_INCLUDE_DIR  ${SDL2_INCLUDE_DIRS} ./src )

include_directories(/usr/include /usr/local/include ${PROJECT_INCLUDE_DIR})

if (MINGW)
    # need to link with mingw32 before SDL2
    list(APPEND _link_libraries mingw32)
endif ()

if(WIN32)
	set ( PROJECT_LINK_LIBS  ${PROJECT_LINK_LIBS} opengl32 gdi32 winmm)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWITH_WINMM")
endif()


#Specific thread library linking to make miniaudio work
if( ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD" )
	set ( PROJECT_LINK_LIBS  ${PROJECT_LINK_LIBS} pthread m )
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
	message( STATUS "System detected is FreeBSD")
	
elseif( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
	set ( PROJECT_LINK_LIBS  ${PROJECT_LINK_LIBS} pthread m dl )
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
	message(STATUS "System detected is Linux")
else ()

endif()


set(SOURCES 
src/main.cpp
			 )

#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

#make executable sphere from simple-sphere.cpp 
add_executable(${PROJECT_NAME} ${SOURCES})
#link libraries
target_link_libraries(${PROJECT_NAME} ${PROJECT_LINK_LIBS} )

#make install ops
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
