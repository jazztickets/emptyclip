cmake_minimum_required(VERSION 2.8)

# define constants
add_definitions(-DGAME_VERSION_STRING="2.0.0")

# projects
project(emptyclip)

# add extra find modules
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

# add custom target to generate version.h
add_custom_target(
	version
	${CMAKE_COMMAND}
	-D SRC=${PROJECT_SOURCE_DIR}/cmake/version.h.in
	-D DST=${PROJECT_SOURCE_DIR}/src/version.h
	-P ${PROJECT_SOURCE_DIR}/cmake/version.cmake
)

#set(EXTRA_LIBS)

# mingw
if(WIN32)
	set(EXTRA_LIBS ${EXTRA_LIBS} winmm ws2_32)
endif()


# set default build type
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

# compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -std=c++11 -msse -mfpmath=sse -Wall -Wextra -Wno-unused-result -Wno-unused-parameter")

# set output path
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})

# find libraries
find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
find_package(OpenAL REQUIRED)
find_package(Vorbis REQUIRED)
find_package(Ogg REQUIRED)
find_package(Freetype REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Threads REQUIRED)

# includes
include_directories("src/")
include_directories(${SDL2_INCLUDE_DIR})
include_directories(${OPENAL_INCLUDE_DIR})
include_directories(${VORBIS_INCLUDE_DIR})
include_directories(${OGG_INCLUDE_DIR})
include_directories(${ZLIB_INCLUDE_DIRS})
include_directories(${FREETYPE_INCLUDE_DIRS})

# add source files
file(GLOB MAIN_SRC "src/*.cpp" "src/*.h")
file(GLOB OBJECTS_SRC "src/objects/*.cpp" "src/objects/*.h")
file(GLOB UI_SRC "src/ui/*.cpp" "src/ui/*.h")
file(GLOB STATES_SRC "src/states/*.cpp" "src/states/*.h")
file(GLOB PNGLITE_SRC "src/pnglite/*.c" "src/pnglite/*.h")

# build executable
add_executable(${CMAKE_PROJECT_NAME}
	${MAIN_SRC}
	${OBJECTS_SRC}
	${UI_SRC}
	${STATES_SRC}
	${PNGLITE_SRC}
)
add_dependencies(${CMAKE_PROJECT_NAME} version)

# link libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
	${OPENGL_LIBRARIES}
	${SDL2_LIBRARY}
	${OPENAL_LIBRARY}
	${VORBIS_LIBRARIES}
	${OGG_LIBRARIES}
	${LUA_LIBRARIES}
	${FREETYPE_LIBRARIES}
	${ZLIB_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	${EXTRA_LIBS}
)

if(WIN32)
else()

	# linux installation
	install(TARGETS ${CMAKE_PROJECT_NAME} RUNTIME DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/ DESTINATION share/games/${CMAKE_PROJECT_NAME} PATTERN "${PROJECT_SOURCE_DIR}/working/*" )
	install(FILES ${PROJECT_SOURCE_DIR}/deployment/${CMAKE_PROJECT_NAME}.png DESTINATION share/pixmaps)
	install(FILES ${PROJECT_SOURCE_DIR}/deployment/${CMAKE_PROJECT_NAME}.desktop DESTINATION share/applications)
	install(FILES ${PROJECT_SOURCE_DIR}/CHANGELOG DESTINATION share/doc/${CMAKE_PROJECT_NAME})
	install(FILES ${PROJECT_SOURCE_DIR}/LICENSE DESTINATION share/doc/${CMAKE_PROJECT_NAME})
	install(FILES ${PROJECT_SOURCE_DIR}/README DESTINATION share/doc/${CMAKE_PROJECT_NAME})

	# generate the script to launch the program
	configure_file(${PROJECT_SOURCE_DIR}/deployment/${CMAKE_PROJECT_NAME} ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles)
	install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${CMAKE_PROJECT_NAME} DESTINATION bin)
endif()
