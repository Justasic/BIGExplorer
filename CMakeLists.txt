cmake_minimum_required(VERSION 2.8)

# Force external build
if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR} AND NOT WIN32)
	message(FATAL_ERROR "You can not use CMake to build from the root of it's source tree! Remove the CMakeCache.txt file from this directory, then create a separate directory (either below this directory or elsewhere), and then re-run CMake from there.")
endif(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR} AND NOT WIN32)

# Get the git revision location for the branch we're on
if(EXISTS "${CMAKE_SOURCE_DIR}/.git/HEAD")
	file(READ ${CMAKE_SOURCE_DIR}/.git/HEAD GIT_HEAD_LOC)
	string(LENGTH ${GIT_HEAD_LOC} HEAD_LEN)
	math(EXPR LEN "${HEAD_LEN} - 5")
	string(SUBSTRING ${GIT_HEAD_LOC} 5 ${LEN} GIT_HEAD)
	# Weird nastery to remove newlines which screw up the if statement below.
	set(GIT_SHA_PATH "${CMAKE_SOURCE_DIR}/.git/${GIT_HEAD}")
	string(REGEX REPLACE "(\r?\n)+$" "" GIT_SHA_PATH "${GIT_SHA_PATH}")
endif(EXISTS "${CMAKE_SOURCE_DIR}/.git/HEAD")

# Get the git revision we're on for the version string
if(EXISTS "${GIT_SHA_PATH}")
	file(READ "${GIT_SHA_PATH}" VERSION_STR)
	string(REGEX REPLACE "(\r?\n)+$" "" VERSION_STR "${VERSION_STR}")
	# Get the length of the string
	string(LENGTH ${VERSION_STR} VERSION_LEN)
	# Subtract 7 from the string's length
	math(EXPR VERSION_NUM_LEN "${VERSION_LEN} - ${VERSION_LEN} + 7")
	# Extract the value from the string
	string(SUBSTRING ${VERSION_STR} 0 ${VERSION_NUM_LEN} VERSION_GIT)
endif(EXISTS "${GIT_SHA_PATH}")

# Set our version strings
SET(VERSION_SIMPLE "${PROJECT_MAJOR_VERSION}.${PROJECT_MINOR_VERSION}.${PROJECT_PATCH_LEVEL}")
SET(VERSION_FULL "${PROJECT_MAJOR_VERSION}.${PROJECT_MINOR_VERSION}.${PROJECT_PATCH_LEVEL}-${VERSION_GIT}")

# Collect our source files.
file(GLOB SOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.cpp")
list(APPEND SOURCE_FILES ${LOADER_FILES})
list(SORT SOURCE_FILES)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Find clang (if the system has it)
find_program(CLANG clang)
find_program(CLANGXX clang++)

# prefer clang instead of gcc (or whatever shit compiler they're using) This can be disabled with
# the NO_CLANG option
option(NO_CLANG "Don't prefer clang for compilation" OFF)
if (NOT NO_CLANG)
	if (CLANG)
		set(CMAKE_C_COMPILER ${CLANG})
	endif (CLANG)

	# In case we need to compile C++ in the future (which I doubt we will)
	if (CLANGXX)
		set(CMAKE_CXX_COMPILER ${CLANGXX})
	endif (CLANGXX)
endif (NOT NO_CLANG)

project(BIGExplorer CXX)
# this allows the check_type_size to work without causing cmake to fail.
enable_language(C)

# Check for platform-specific things we need
include (CheckTypeSize)
include (CheckIncludeFile)
include (CheckLibraryExists)
include (CheckFunctionExists)
include (CheckCXXSourceCompiles)
include (CheckCXXCompilerFlag)

check_type_size(int8_t HAVE_INT8_T)
check_type_size(uint8_t HAVE_UINT8_T)
check_type_size(int16_t HAVE_INT16_T)
check_type_size(uint16_t HAVE_UINT16_T)
check_type_size(uint32_t HAVE_UINT32_T)
check_type_size(int64_t HAVE_INT64_T)
check_type_size(uint64_t HAVE_UINT64_T)

check_type_size(u_int8_t HAVE_U_INT8_T)
check_type_size(u_int16_t HAVE_U_INT16_T)
check_type_size(u_int32_t HAVE_U_INT32_T)
check_type_size(u_int64_t HAVE_U_INT64_T)

check_cxx_compiler_flag(-std=c++11 HAVE_CXX_11)

if (NOT HAVE_CXX_11)
	message(FATAL_ERROR "Your compiler must support C++11!")
endif (NOT HAVE_CXX_11)

# Required for Qt
#set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Widgets REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_BINARY_DIR})

# Enable color diagnostics when building with Ninja + clang.
if ("${CMAKE_GENERATOR}" STREQUAL "Ninja" AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "${CMALE_CXX_FLAGS} -fcolor-diagnostics")
endif ("${CMAKE_GENERATOR}" STREQUAL "Ninja" AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")

# C++11 support enabled.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -Werror=undef -Wno-unused-parameter")
#set(CXXFLAGS "${CXXFLAGS} ${CMAKE_CXX_FLAGS}")

# Thanks to this StackOverflow question I managed to get this shit to work the way I want.
# http://stackoverflow.com/questions/19761767/qt-5-cmake-fails-with-undefined-reference-to-vtable-on-hello-world-with-inc-sr
qt5_wrap_cpp(QT_UICRAP_HEADER ${CMAKE_SOURCE_DIR}/include/mainwindow.h)
qt5_wrap_ui(QT_UICRAP_GUI ${CMAKE_SOURCE_DIR}/src/explorer.ui)
set(SOURCE_FILES ${SOURCE_FILES} ${QT_UICRAP_HEADER})

set_source_files_properties(${SOURCE_FILES} PROPERTIES LANGUAGE CXX COMPILE_FLAGS "${CXXFLAGS}")
add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${QT_UICRAP_GUI})

# Link Qt with the project.
target_link_libraries(${PROJECT_NAME} Qt5::Widgets)
