cmake_minimum_required(VERSION 2.8)

# define constants
add_definitions(-DGAME_VERSION_STRING="2.0.0")

# projects
project(emptyclip)

# add extra find modules
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")
set(EXTRA_LIBS)

if(MSVC)
	set(EXTRA_LIBS ${EXTRA_LIBS} winmm)
	add_definitions("-DNOMINMAX -D_USE_MATH_DEFINES")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
	
	set(FREETYPE_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/extlibs/include")
	set(OPENAL_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/extlibs/include/AL")

	set(FREETYPE_LIBRARY "freetype.lib")
	set(ZLIB_LIBRARIES "zlib.lib")
	set(VORBIS_LIBRARY debug ${PROJECT_SOURCE_DIR}/extlibs/debug/libvorbis_d.lib optimized ${PROJECT_SOURCE_DIR}/extlibs/release/libvorbis.lib)
	set(VORBISFILE_LIBRARY debug ${PROJECT_SOURCE_DIR}/extlibs/debug/libvorbisfile_d.lib optimized ${PROJECT_SOURCE_DIR}/extlibs/release/libvorbisfile.lib)
	set(OGG_LIBRARY debug ${PROJECT_SOURCE_DIR}/extlibs/debug/libogg_d.lib optimized ${PROJECT_SOURCE_DIR}/extlibs/release/libogg.lib)
	set(OPENAL_LIBRARY debug ${PROJECT_SOURCE_DIR}/extlibs/debug/OpenAL32.lib optimized ${PROJECT_SOURCE_DIR}/extlibs/release/OpenAL32.lib)

	include_directories("${PROJECT_SOURCE_DIR}/extlibs/include")
	include_directories("${PROJECT_SOURCE_DIR}/extlibs/include/zlib")
	link_directories("${PROJECT_SOURCE_DIR}/extlibs/")
	
	set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/)
elseif(UNIX)
	add_definitions("-Wall")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

	# set default build type
	if(NOT CMAKE_BUILD_TYPE)
		set(CMAKE_BUILD_TYPE Release)
	endif()
	
	set(VORBISFILE_LIBRARY vorbisfile)
	
	find_package(ZLIB REQUIRED)
	if(NOT ZLIB_FOUND)
		message(SEND_ERROR "Could not find zlib!")
	endif()
	
	find_package(OpenAL REQUIRED)
	if(NOT OPENAL_FOUND)
		message(SEND_ERROR "Could not find OpenAL!")
	endif()

	set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})
endif()

# find libraries
find_package(OpenGL REQUIRED)
if(NOT OPENGL_FOUND)
	message(SEND_ERROR "Could not find OpenGL!")
endif()

find_package(SDL2 REQUIRED)
if(NOT SDL2_FOUND)
	message(SEND_ERROR "Could not find SDL2!")
endif()

find_package(Freetype REQUIRED)
if(NOT FREETYPE_FOUND)
	message(SEND_ERROR "Could not find Freetype 2!")
endif()

# includes
include_directories(${FREETYPE_INCLUDE_DIRS})
include_directories(${SDL2_INCLUDE_DIR})
include_directories(${OPENAL_INCLUDE_DIR})
include_directories("src/")

# add source files
file(GLOB MAIN_SRC "src/*.cpp" "src/*.h")
file(GLOB OBJECTS_SRC "src/objects/*.cpp" "src/objects/*.h")
file(GLOB UI_SRC "src/ui/*.cpp" "src/ui/*.h")
file(GLOB STATES_SRC "src/states/*.cpp" "src/states/*.h")
file(GLOB PNGLITE_SRC "src/pnglite/*.c" "src/pnglite/*.h")

# add virtual folders
source_group(main FILES ${MAIN_SRC})
source_group(objects FILES ${OBJECTS_SRC})
source_group(ui FILES ${UI_SRC})
source_group(states FILES ${STATES_SRC})

# build executable
add_executable(${CMAKE_PROJECT_NAME}
	${MAIN_SRC}
	${OBJECTS_SRC}
	${UI_SRC}
	${STATES_SRC}
	${PNGLITE_SRC}
)

# link libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
	${OPENGL_LIBRARIES}
	${FREETYPE_LIBRARIES}
	${SDL2_LIBRARY}
	${OPENAL_LIBRARY}
	${ZLIB_LIBRARIES}
	${VORBIS_LIBRARY}
	${VORBISFILE_LIBRARY}
	${OGG_LIBRARY}
	${EXTRA_LIBS}
)

if(MSVC)
	add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "$ENV{SDL2DIR}/lib/x86/SDL2.dll" ${PROJECT_SOURCE_DIR}/working)
else()
	
	# linux installation
	install(TARGETS ${CMAKE_PROJECT_NAME} RUNTIME DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/fonts DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/maps DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/music DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/sounds DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/tables DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/textures DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(FILES ${PROJECT_SOURCE_DIR}/deployment/emptyclip.png DESTINATION share/pixmaps)
	install(FILES ${PROJECT_SOURCE_DIR}/deployment/emptyclip.desktop DESTINATION share/applications)
	install(FILES ${PROJECT_SOURCE_DIR}/CHANGELOG DESTINATION share/doc/${CMAKE_PROJECT_NAME})
	install(FILES ${PROJECT_SOURCE_DIR}/LICENSE DESTINATION share/doc/${CMAKE_PROJECT_NAME})
	install(FILES ${PROJECT_SOURCE_DIR}/README DESTINATION share/doc/${CMAKE_PROJECT_NAME})
	
	# generate the script to launch the program
	configure_file(${PROJECT_SOURCE_DIR}/deployment/${CMAKE_PROJECT_NAME} ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles)
	install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${CMAKE_PROJECT_NAME} DESTINATION bin)
endif()
