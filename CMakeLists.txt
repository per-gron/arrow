cmake_minimum_required(VERSION 2.8)
project(arrow)

add_definitions(
  -std=c++11
  -DGTEST_USE_OWN_TR1_TUPLE=1
)

set(arw_sources
  src/base/checks.cc
  src/base/checks.h
  src/storage/handle.h
  src/storage/local_stack.h
  src/storage/local.h
  src/storage/member.h
  src/storage/optional.h
  src/storage/persistent_pool.h
  src/storage/persistent.h
  src/storage/storage_descriptor.h
  src/storage/type_storage.h)

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
  test/base/checks_test.cc
  test/main.cc
  test/storage/handle_test.cc
  test/storage/local_stack_test.cc
  test/storage/member_test.cc
  test/storage/optional_test.cc
  test/storage/persistent_pool_test.cc
  test/storage/storage_descriptor_test.cc
  test/storage/type_storage_test.cc)

add_executable(arw_test ${arw_test_sources})
target_link_libraries(arw_test arw gtest)
target_include_directories(arw_test PUBLIC ${gtest_include_root})
target_include_directories(arw_test PRIVATE "src")

add_custom_target(lint python vendor/cpplint.py ${arw_sources} ${arw_test_sources}
                  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
