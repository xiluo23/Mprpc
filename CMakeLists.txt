cmake_minimum_required(VERSION 3.28)
project(mprpc)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/example)
include_directories(${PROJECT_SOURCE_DIR}/src/include)


link_directories(${PROJECT_SOURCE_DIR}/lib)

add_compile_options(-g)
add_subdirectory(src)
add_subdirectory(example)