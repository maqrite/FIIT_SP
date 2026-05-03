include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(
        sys_prog_assctv_cntnr_indxng_tr_b_tr_tests
        b_tree_tests.cpp)

target_link_libraries(
        sys_prog_assctv_cntnr_indxng_tr_b_tr_tests
        PRIVATE
        GTest::gtest_main)

target_link_libraries(
        sys_prog_assctv_cntnr_indxng_tr_b_tr_tests
        PRIVATE
        sys_prog_assctv_cntnr_indxng_tr_b_tr)
