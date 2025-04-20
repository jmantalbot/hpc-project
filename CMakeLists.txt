# -- General Setup --
cmake_minimum_required(VERSION 3.24)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)

include(FetchContent)

set(PROJECT_NAME genre_reveal_party)
project(${PROJECT_NAME} VERSION 0.1.0 LANGUAGES CXX)

FetchContent_Declare(
	rapidcsv
	GIT_REPOSITORY https://github.com/d99kris/rapidcsv
)
FetchContent_MakeAvailable(rapidcsv)
file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

find_package(OpenMP REQUIRED)
find_package(MPI REQUIRED)
include_directories(${MPI_INCLUDE_PATH})
set(Boost_DIR ${CMAKE_SOURCE_DIR}/boost_1_88_0/lib/cmake/Boost-1.88.0)
set(Boost_USE_MULTITHREADED TRUE)
message(Boost_DIR=${Boost_DIR})
find_package(Boost REQUIRED COMPONENTS mpi serialization)
include_directories(${Boost_INCLUDE_DIRS})

# -- Serial Target --
set(SERIAL_TARGET genre_reveal_party)

set(SOURCE_DIR src)
set(HEADER_FILES ${SOURCE_DIR}/point.hpp ${SOURCE_DIR}/shared_cluster.hpp)
set(SOURCE_FILES ${SOURCE_DIR}/shared_cluster.cpp)
set(MAIN_FILE ${SOURCE_DIR}/main.cpp)
add_executable(${SERIAL_TARGET} ${HEADER_FILES} ${SOURCE_FILES} ${MAIN_FILE})

set_property(TARGET ${SERIAL_TARGET} PROPERTY CXX_STANDARD ${CMAKE_CXX_STANDARD})
target_compile_definitions(${SERIAL_TARGET} PRIVATE SERIAL_TARGET)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${SERIAL_TARGET} PRIVATE /W4 /permissive-)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${SERIAL_TARGET} PRIVATE -Wall -Wextra)
endif()

target_link_libraries(${SERIAL_TARGET} rapidcsv)

# -- OMP Target --
set(OMP_TARGET genre_reveal_party_omp)

set(SOURCE_DIR src)
set(HEADER_FILES ${SOURCE_DIR}/point.hpp ${SOURCE_DIR}/shared_cluster.hpp)
set(SOURCE_FILES ${SOURCE_DIR}/shared_cluster.cpp)
set(MAIN_FILE ${SOURCE_DIR}/main.cpp)
add_executable(${OMP_TARGET} ${HEADER_FILES} ${SOURCE_FILES} ${MAIN_FILE})

set_property(TARGET ${OMP_TARGET} PROPERTY CXX_STANDARD ${CMAKE_CXX_STANDARD})
target_compile_definitions(${OMP_TARGET} PRIVATE OMP_TARGET)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${OMP_TARGET} PRIVATE /W4 /permissive-)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${OMP_TARGET} PRIVATE -Wall -Wextra)
endif()

target_link_libraries(${OMP_TARGET} rapidcsv OpenMP::OpenMP_CXX)

# -- MPI Target --
set(MPI_TARGET genre_reveal_party_mpi)


set(SOURCE_DIR src)
set(HEADER_FILES ${SOURCE_DIR}/point.hpp ${SOURCE_DIR}/distributed_cluster.hpp)
set(SOURCE_FILES ${SOURCE_DIR}/distributed_cluster.cpp)
set(MAIN_FILE ${SOURCE_DIR}/main.cpp)
add_executable(${MPI_TARGET} ${HEADER_FILES} ${SOURCE_FILES} ${MAIN_FILE})

set_property(TARGET ${MPI_TARGET} PROPERTY CXX_STANDARD ${CMAKE_CXX_STANDARD})
target_compile_definitions(${MPI_TARGET} PRIVATE MPI_TARGET)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${MPI_TARGET} PRIVATE /W4 /permissive-)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${MPI_TARGET} PRIVATE -Wall -Wextra)
endif()

target_link_libraries(${MPI_TARGET} rapidcsv ${MPI_LIBRARIES} ${Boost_LIBRARIES})

# -- CUDA Target --
set(CUDA_TARGET genre_reveal_party_cuda)

enable_language(CUDA)
set(CMAKE_CUDA_STANDARD 11)

set(SOURCE_DIR src)
set(HEADER_FILES ${SOURCE_DIR}/point.hpp ${SOURCE_DIR}/cuda_cluster.hpp)
set(SOURCE_FILES ${SOURCE_DIR}/cuda_cluster.cu)
set(MAIN_FILE ${SOURCE_DIR}/main.cpp)
add_executable(${CUDA_TARGET} ${HEADER_FILES} ${SOURCE_FILES} ${MAIN_FILE})
target_compile_definitions(${CUDA_TARGET} PRIVATE CUDA_TARGET)

set_property(TARGET ${CUDA_TARGET} PROPERTY CUDA_ARCHITECTURES "native")
set_property(TARGET ${CUDA_TARGET} PROPERTY CUDA_SEPARABLE_COMPILATION ON)
set_property(TARGET ${CUDA_TARGET} PROPERTY CXX_STANDARD ${CMAKE_CXX_STANDARD})

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${CUDA_TARGET} PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:-Xcompiler=/W4>)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${CUDA_TARGET} PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:--compiler-options -Wall,-Wextra>)
endif()

target_link_libraries(${CUDA_TARGET} rapidcsv)
target_compile_options(${CUDA_TARGET} PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:-O3>)
target_compile_options(${CUDA_TARGET} PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:--default-stream per-thread>)

# -- CUDA w/ MPI Target --

