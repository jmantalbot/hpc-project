cmake_minimum_required(VERSION 3.24)
project(genre_reveal_party VERSION 0.1.0 LANGUAGES CXX)

set(BINARY genre_reveal_party.out)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)

include(FetchContent)

FetchContent_Declare(
	rapidcsv
	GIT_REPOSITORY https://github.com/d99kris/rapidcsv
)
FetchContent_MakeAvailable(rapidcsv)

file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(${BINARY} src/main.cpp)
target_link_libraries(${BINARY} rapidcsv)
