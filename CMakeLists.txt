cmake_minimum_required(VERSION 2.8)

find_package(Boost 1.48 COMPONENTS system)
include_directories("${Boost_INCLUDE_DIRS}")
link_directories("${Boost_LIBRARY_DIRS}")

set(CMAKE_CXX_FLAGS "-std=c++0x -ggdb")

file(GLOB sources *.cpp)

foreach(source ${sources})
    string(REGEX REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/(.*)\\.cpp" "\\1" target ${source})
    add_executable(${target} ${source})
    target_link_libraries(${target} ${Boost_LIBRARIES} pthread)
endforeach(source)


