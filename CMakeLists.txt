cmake_minimum_required(VERSION 2.8)
project(arrow)

add_definitions(
  -std=c++11
  -DGTEST_USE_OWN_TR1_TUPLE=1
)

set(arw_sources
  src/checks.cc
  src/checks.h
  src/handle.h
  src/local_stack.h
  src/local.h
  src/member_handle_hooks.h
  src/member.h
  src/optional.h
  src/persistent_pool.h
  src/persistent.h
  src/storage_descriptor.h
  src/type_storage.h)

add_library(arw STATIC ${arw_sources})

set(gtest_root vendor/gtest)
set(gtest_src_root ${gtest_root}/src)
set(gtest_include_root ${gtest_root}/include)
set(gtest_sources
  ${gtest_src_root}/gtest-all.cc
  ${gtest_src_root}/gtest-death-test.cc
  ${gtest_src_root}/gtest-filepath.cc
  ${gtest_src_root}/gtest-internal-inl.h
  ${gtest_src_root}/gtest-port.cc
  ${gtest_src_root}/gtest-printers.cc
  ${gtest_src_root}/gtest-test-part.cc
  ${gtest_src_root}/gtest-typed-test.cc
  ${gtest_src_root}/gtest.cc
  ${gtest_src_root}/gtest_main.cc)

add_library(gtest STATIC ${gtest_sources})
target_include_directories(gtest PUBLIC ${gtest_include_root} ${gtest_root})

set(arw_test_sources
  test/cc/checks_test.cc
  test/cc/handle_test.cc
  test/cc/local_stack_test.cc
  test/cc/main.cc
  test/cc/member_handle_hooks_test.cc
  test/cc/member_test.cc
  test/cc/optional_test.cc
  test/cc/persistent_pool_test.cc
  test/cc/storage_descriptor_test.cc
  test/cc/type_storage_test.cc)

add_executable(arw_test ${arw_test_sources})
target_link_libraries(arw_test arw gtest)
target_include_directories(arw_test PUBLIC ${gtest_include_root})
target_include_directories(arw_test PRIVATE "src")

add_custom_target(lint python vendor/cpplint.py ${arw_sources} ${arw_test_sources}
                  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
