cmake_minimum_required(VERSION 3.24)
project(homework2 VERSION 0.1.0 LANGUAGES CXX)

set(BINARY a.out)
set(CMAKE_BUILD_TYPE Debug)

include(FetchContent)

FetchContent_Declare(
	rapidcsv
	GIT_REPOSITORY https://github.com/d99kris/rapidcsv
)
FetchContent_MakeAvailable(rapidcsv)

add_executable(${BINARY} main.cpp)
target_link_libraries(${BINARY} rapidcsv)
