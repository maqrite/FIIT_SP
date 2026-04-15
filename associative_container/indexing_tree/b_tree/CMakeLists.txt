cmake_minimum_required(VERSION 3.15)
project(B_Tree_Lab)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Boost REQUIRED)

add_library(
        sys_prog_assctv_cntnr_indxng_tr_b_tr
        include/b_tree.h
        src/hhh.cpp)

target_include_directories(
        sys_prog_assctv_cntnr_indxng_tr_b_tr
        PUBLIC
        ./include
        ${Boost_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../allocator/allocator/include
)

target_link_libraries(
        sys_prog_assctv_cntnr_indxng_tr_b_tr
        PUBLIC
        sys_prog_assctv_cntnr
        ${BOOST_LIBRARIES})

target_link_directories(
        sys_prog_assctv_cntnr_indxng_tr_b_tr
        PUBLIC
        ${Boost_LIBRARY_DIR})


add_executable(my_tree_app 
        src/main.cpp 
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../allocator/allocator/src/pp_allocator.cpp 
)

target_link_libraries(my_tree_app 
        PRIVATE 
        sys_prog_assctv_cntnr_indxng_tr_b_tr
)

target_include_directories(my_tree_app 
        PRIVATE 
        ./include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../allocator/allocator/include)

# add_subdirectory(tests)
