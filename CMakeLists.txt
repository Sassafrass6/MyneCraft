cmake_minimum_required(VERSION 2.8)
project(ProjectName)

## Macros and Functions ##

# Sets variables module_name_SOURCES and HEADERS to include as source in a target.
# Will also configure_file any *.h.in and *.hpp.in to the binary directory.
macro(add_project_module module_name module_path)
	file(GLOB MODULE_${module_name}_SOURCES
		${PROJECT_SOURCE_DIR}/Source/${module_path}/*.c
		${PROJECT_SOURCE_DIR}/Source/${module_path}/*.cpp
		${PROJECT_SOURCE_DIR}/Source/${module_path}/*.mm
		${PROJECT_SOURCE_DIR}/Source/${module_path}/*.h
		${PROJECT_SOURCE_DIR}/Source/${module_path}/*.hpp
	)
	file(GLOB MODULE_${module_name}_CONFIGURE_HEADERS
		${PROJECT_SOURCE_DIR}/Source/${module_path}/*.h.in
		${PROJECT_SOURCE_DIR}/Source/${module_path}/*.hpp.in
	)

	foreach(__f ${MODULE_${module_name}_CONFIGURE_HEADERS})
		string(REPLACE ".in" "" __f_name ${__f})
		string(REPLACE "${PROJECT_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" __f_name ${__f_name})
		configure_file(${__f} ${__f_name})
		list(APPEND MODULE_${module_name}_CONFIGURED_HEADERS ${__f_name})
		message(STATUS "Configured ${__f_name}")
	endforeach()

	list(APPEND PROJECT_MODULE_SOURCES ${MODULE_${module_name}_SOURCES} ${MODULE_${module_name}_CONFIGURED_HEADERS})
	list(APPEND PROJECT_MODULE_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/Source/${module_path} ${CMAKE_BINARY_DIR}/Source/${module_path})
	source_group(${module_name} FILES ${MODULE_${module_name}_SOURCES} ${MODULE_${module_name}_CONFIGURE_HEADERS} ${MODULE_${module_name}_CONFIGURED_HEADERS})
	set_source_files_properties(${MODULE_${module_name}_CONFIGURED_HEADERS} PROPERTIES GENERATED ON)

	message(STATUS "Module added: ${module_name} at ${module_path}")
endmacro()

## End Macros ##

set(OGRE_HOME "$ENV{OGRE_HOME}" CACHE PATH "Path to Ogre SDK")

if(CMAKE_SYSTEM_NAME MATCHES "Darwin") # MacOSX
	set(OGRE_CMAKE_PATH "${OGRE_HOME}/CMake" CACHE PATH "Path to Ogre's CMake scripts")
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
	set(OGRE_CMAKE_PATH "${OGRE_HOME}/lib/OGRE/cmake" CACHE PATH "Path to Ogre's CMake scripts")
else() # other, including Windows
	set(OGRE_CMAKE_PATH "${OGRE_HOME}/CMake" CACHE PATH "Path to Ogre's CMake scripts")
endif()

message(STATUS "OGRE_HOME is ${OGRE_HOME}")
message(STATUS "OGRE_CMAKE_PATH is ${OGRE_CMAKE_PATH}")
message(STATUS "If the above two variables are wrong, edit CMakeCache.txt and set them correctly.")

set(CMAKE_MODULE_PATH "${OGRE_CMAKE_PATH}" ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/CMake)
set(CMAKE_FRAMEWORK_PATH ${CMAKE_FRAMEWORK_PATH} "${OGRE_HOME}/lib/macosx/Release")

# Using C++11 so let's ensure that's set on compilers that don't implicitly use it
if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "--std=c++11")
endif()
set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")

find_package(OGRE REQUIRED COMPONENTS Overlay RenderSystem_GL)
find_package(OIS REQUIRED)
if(APPLE)
	find_library(CoreFoundation_LIBRARY CoreFoundation REQUIRED)
	find_library(Cocoa_LIBRARY Cocoa REQUIRED)
	find_library(IOKit_LIBRARY IOKit REQUIRED)
	find_library(QuartzCore_LIBRARY QuartzCore REQUIRED)
	find_library(OpenGL_LIBRARY OpenGL REQUIRED)
endif()

# Interface setup
if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
	set(INTERFACE "MacOSX")
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
	set(INTERFACE "Windows")
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
	set(INTERFACE "Linux")
else() # Unknown
	message(WARNING "Unknown platform, not sure what INTERFACE to use!")
	set(INTERFACE "Linux")
endif()
message(STATUS "Interface: ${INTERFACE}")

set(CMAKE_DEBUG_POSTFIX "_d")

# Module List
add_project_module(Core      Core)
add_project_module(Interface Interface/${INTERFACE})
# End module list

add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE ${PROJECT_MODULE_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_MODULE_INCLUDE_DIRS})

set_target_properties(${PROJECT_NAME} PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Binaries
	LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Binaries
	ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Binaries
	PDB_OUTPUT_DIRECTORY     ${CMAKE_BINARY_DIR}/Binaries
)

# Library linking and including
target_link_libraries(${PROJECT_NAME} PRIVATE
	${OGRE_LIBRARIES}
	${OGRE_Overlay_LIBRARIES}
	${OGRE_RenderSystem_GL_LIBRARIES}
	${OIS_LIBRARIES}
	${CoreFoundation_LIBRARY}
	${Cocoa_LIBRARY}
	${IOKit_LIBRARY}
	${QuartzCore_LIBRARY}
	${OpenGL_LIBRARY}
)

target_include_directories(${PROJECT_NAME} PRIVATE
	${OGRE_INCLUDE_DIRS}
	${OGRE_Overlay_INCLUDE_DIRS}
	${OGRE_RenderSystem_GL_INCLUDE_DIRS}
	${OGRE_HOME}/include/OGRE/RenderSystems/GL
	${OIS_INCLUDE_DIR}
)

# On Windows, copy DLLs to bin path.
# If you link more libraries or plugins, make sure to add commands here.
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different $<$<CONFIG:Release>:${OGRE_BINARY_REL}> $<$<NOT:$<CONFIG:Release>>:${OGRE_BINARY_DBG}> $<TARGET_FILE_DIR:${PROJECT_NAME}>
		COMMAND ${CMAKE_COMMAND} -E copy_if_different $<$<CONFIG:Release>:${OGRE_RenderSystem_GL_REL}> $<$<NOT:$<CONFIG:Release>>:${OGRE_RenderSystem_GL_DBG}> $<TARGET_FILE_DIR:${PROJECT_NAME}>
		COMMENT "Copying Ogre DLLs to build directory for debugging."
	)
endif()