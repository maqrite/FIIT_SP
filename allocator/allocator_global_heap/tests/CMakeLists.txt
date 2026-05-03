include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(
        sys_prog_allctr_allctr_glbl_hp_tests
        allocator_global_heap_tests.cpp)

target_link_libraries(
        sys_prog_allctr_allctr_glbl_hp_tests
        PRIVATE
        GTest::gtest_main)

target_link_libraries(
        sys_prog_allctr_allctr_glbl_hp_tests
        PRIVATE
        sys_prog_allctr_allctr_glbl_hp)
