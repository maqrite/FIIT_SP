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
        ./include)

target_link_libraries(
        sys_prog_assctv_cntnr_indxng_tr_b_tr
        PUBLIC
        sys_prog_assctv_cntnr)

target_link_libraries(
        sys_prog_assctv_cntnr_indxng_tr_b_tr
        PUBLIC
        ${BOOST_LIBRARIES})

target_include_directories(
        sys_prog_assctv_cntnr_indxng_tr_b_tr
        PUBLIC
        ${Boost_INCLUDE_DIRS})

target_link_directories(
        sys_prog_assctv_cntnr_indxng_tr_b_tr
        PUBLIC
        ${Boost_LIBRARY_DIR})

add_subdirectory(tests)
