cmake_minimum_required(VERSION 3.24)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)

include(FetchContent)

set(PROJECT_NAME genre_reveal_party)
set(SERIAL_TARGET genre_reveal_party)
set(OMP_TARGET genre_reveal_party_omp)

project(${PROJECT_NAME} VERSION 0.1.0 LANGUAGES CXX)

set(SOURCE_DIR src)
set(HEADER_FILES ${SOURCE_DIR}/point.hpp ${SOURCE_DIR}/cluster.hpp)
set(SOURCE_FILES ${SOURCE_DIR}/cluster.cpp)
set(MAIN_FILE ${SOURCE_DIR}/main.cpp)
add_executable(${SERIAL_TARGET} ${HEADER_FILES} ${SOURCE_FILES} ${MAIN_FILE})

set(SOURCE_DIR ompsrc)
set(HEADER_FILES ${SOURCE_DIR}/point.hpp ${SOURCE_DIR}/cluster.hpp)
set(SOURCE_FILES ${SOURCE_DIR}/cluster.cpp)
set(MAIN_FILE ${SOURCE_DIR}/main.cpp)
add_executable(${OMP_TARGET} ${HEADER_FILES} ${SOURCE_FILES} ${MAIN_FILE})

FetchContent_Declare(
	rapidcsv
	GIT_REPOSITORY https://github.com/d99kris/rapidcsv
)
FetchContent_MakeAvailable(rapidcsv)
file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set_property(TARGET ${SERIAL_TARGET} PROPERTY CXX_STANDARD ${CMAKE_CXX_STANDARD})
set_property(TARGET ${OMP_TARGET} PROPERTY CXX_STANDARD ${CMAKE_CXX_STANDARD})

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${SERIAL_TARGET} PRIVATE /W4 /permissive-)
    target_compile_options(${OMP_TARGET} PRIVATE /W4 /permissive-)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${SERIAL_TARGET} PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(${OMP_TARGET} PRIVATE -Wall -Wextra -pedantic)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${SERIAL_TARGET} PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(${OMP_TARGET} PRIVATE -Wall -Wextra -pedantic)
endif()

target_link_libraries(${SERIAL_TARGET} rapidcsv)

find_package(OpenMP REQUIRED)
target_link_libraries(${OMP_TARGET} rapidcsv OpenMP::OpenMP_CXX)