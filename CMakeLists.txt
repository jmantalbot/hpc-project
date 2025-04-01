# CMake file based on those used for CS3460 C++ with Dr. Dean Mathias
cmake_minimum_required(VERSION 3.24)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)

include(FetchContent)


set(PROJECT_NAME genre_reveal_party)
set(SERIAL_TARGET genre_reveal_party)
set(MPI_TARGET genre_reveal_party_mpi)

# set(UNIT_TEST_RUNNER UnitTestRunner)
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

# set(UNIT_TEST_FILES test.cpp)
# add_executable(${UNIT_TEST_RUNNER} ${SOURCE_FILES} ${HEADER_FILES} ${UNIT_TEST_FILES})




FetchContent_Declare(
	rapidcsv
	GIT_REPOSITORY https://github.com/d99kris/rapidcsv
)
FetchContent_MakeAvailable(rapidcsv)
file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})



set_property(TARGET ${SERIAL_TARGET} PROPERTY CXX_STANDARD ${CMAKE_CXX_STANDARD})
set_property(TARGET ${MPI_TARGET} PROPERTY CXX_STANDARD ${CMAKE_CXX_STANDARD})
# set_property(TARGET ${UNIT_TEST_RUNNER} PROPERTY CXX_STANDARD ${CMAKE_CXX_STANDARD})

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${SERIAL_TARGET} PRIVATE /W4 /permissive-)
    target_compile_options(${MPI_TARGET} PRIVATE /W4 /permissive-)
    # target_compile_options(${UNIT_TEST_RUNNER} PRIVATE /W4 /permissive-)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${SERIAL_TARGET} PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(${MPI_TARGET} PRIVATE -Wall -Wextra -pedantic)
    # target_compile_options(${UNIT_TEST_RUNNER} PRIVATE -Wall -Wextra -pedantic)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${SERIAL_TARGET} PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(${MPI_TARGET} PRIVATE -Wall -Wextra -pedantic)
    # target_compile_options(${UNIT_TEST_RUNNER} PRIVATE -Wall -Wextra -pedantic)
endif()

target_link_libraries(${SERIAL_TARGET} rapidcsv)

# https://cliutils.gitlab.io/modern-cmake/chapters/packages/MPI.html
find_package(MPI REQUIRED)
include_directories(SYSTEM ${MPI_INCLUDE_PATH})
if(NOT TARGET MPI::MPI_CXX)
    add_library(MPI::MPI_CXX IMPORTED INTERFACE)

    set_property(TARGET MPI::MPI_CXX
                 PROPERTY INTERFACE_COMPILE_OPTIONS ${MPI_CXX_COMPILE_FLAGS})
    set_property(TARGET MPI::MPI_CXX
                 PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${MPI_CXX_INCLUDE_PATH}")
    set_property(TARGET MPI::MPI_CXX
                 PROPERTY INTERFACE_LINK_LIBRARIES ${MPI_CXX_LINK_FLAGS} ${MPI_CXX_LIBRARIES})
endif()
target_link_libraries(${MPI_TARGET} rapidcsv MPI::MPI_CXX)


# #
# # gtest
# #
# FetchContent_Declare(
# 	googletest
# 	GIT_REPOSITORY 	https://github.com/google/googletest.git
# 	GIT_TAG			release-1.11.0
# )

# set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# FetchContent_MakeAvailable(googletest)


# target_link_libraries(${UNIT_TEST_RUNNER} gtest_main)
