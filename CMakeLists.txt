cmake_minimum_required(VERSION 3.24)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)

include(FetchContent)

set(PROJECT_NAME genre_reveal_party)
set(SERIAL_TARGET genre_reveal_party)
set(MPI_TARGET genre_reveal_party_mpi)
set(Boost_DIR ${CMAKE_SOURCE_DIR}/boost_1_85_0/lib/cmake/Boost-1.85.0)
set(Boost_USE_MULTITHREADED TRUE)
message(Boost_DIR=${Boost_DIR})

project(${PROJECT_NAME} VERSION 0.1.0 LANGUAGES CXX)

set(SOURCE_DIR src)
set(HEADER_FILES ${SOURCE_DIR}/point.hpp ${SOURCE_DIR}/cluster.hpp)
set(SOURCE_FILES ${SOURCE_DIR}/cluster.cpp)
set(MAIN_FILE ${SOURCE_DIR}/main.cpp)
add_executable(${SERIAL_TARGET} ${HEADER_FILES} ${SOURCE_FILES} ${MAIN_FILE})

set(SOURCE_DIR mpi_src)
set(HEADER_FILES ${SOURCE_DIR}/point.hpp ${SOURCE_DIR}/cluster.hpp)
set(SOURCE_FILES ${SOURCE_DIR}/cluster.cpp)
set(MAIN_FILE ${SOURCE_DIR}/main.cpp)
add_executable(${MPI_TARGET} ${HEADER_FILES} ${SOURCE_FILES} ${MAIN_FILE})

FetchContent_Declare(
	rapidcsv
	GIT_REPOSITORY https://github.com/d99kris/rapidcsv
)
FetchContent_MakeAvailable(rapidcsv)
file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})


set_property(TARGET ${SERIAL_TARGET} PROPERTY CXX_STANDARD ${CMAKE_CXX_STANDARD})
set_property(TARGET ${MPI_TARGET} PROPERTY CXX_STANDARD ${CMAKE_CXX_STANDARD})

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${SERIAL_TARGET} PRIVATE /W4 /permissive-)
    target_compile_options(${MPI_TARGET} PRIVATE /W4 /permissive-)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${SERIAL_TARGET} PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(${MPI_TARGET} PRIVATE -Wall -Wextra -pedantic)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${SERIAL_TARGET} PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(${MPI_TARGET} PRIVATE -Wall -Wextra -pedantic)
endif()

target_link_libraries(${SERIAL_TARGET} rapidcsv)
target_link_libraries(${MPI_TARGET} rapidcsv)

find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})

find_package(Boost REQUIRED COMPONENTS mpi serialization)
include_directories(${Boost_INCLUDE_DIRS})

target_link_libraries(${MPI_TARGET} ${MPI_LIBRARIES} ${Boost_LIBRARIES})
