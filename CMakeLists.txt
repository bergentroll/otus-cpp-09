# To the extent possible under law, bayan has waived all copyright and related
# or neighboring rights to program.
#
# Written by 2020 Anton Karmanov <bergentroll@insiberia.net>.

cmake_minimum_required(VERSION 3.12.4)

if($ENV{TRAVIS_BUILD_NUMBER})
  set(VERSION 0.0.$ENV{TRAVIS_BUILD_NUMBER})
else()
  set(VERSION 0.0.0)
endif()
project(bayan VERSION ${VERSION})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

### Dependencies ##############################################################

set(SRC_PATH "src/")
set(TEST_PATH "test/")
include_directories("inc/")

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(
  Boost 1.65
  COMPONENTS program_options
  OPTIONAL_COMPONENTS unit_test_framework)

### Main targets ##############################################################

add_library(
  ${CMAKE_PROJECT_NAME}-obj
  ${SRC_PATH}/lazy_digest.cpp
  ${SRC_PATH}/bayan.cpp)
target_include_directories(
  ${CMAKE_PROJECT_NAME}-obj
  PRIVATE ${Boost_INCLUDE_DIR})
target_link_libraries(
  ${CMAKE_PROJECT_NAME}-obj
  stdc++fs)

add_executable(${CMAKE_PROJECT_NAME} ${SRC_PATH}/main.cpp)
target_include_directories(
  ${CMAKE_PROJECT_NAME}
  PRIVATE ${Boost_INCLUDE_DIR})
target_link_libraries(
  ${CMAKE_PROJECT_NAME}
  ${CMAKE_PROJECT_NAME}-obj
  stdc++fs
  ${Boost_PROGRAM_OPTIONS_LIBRARY})

### Tests #####################################################################

if(Boost_UNIT_TEST_FRAMEWORK_FOUND)
  add_executable(test-suite ${TEST_PATH}/test.cpp)

  target_link_libraries(
    test-suite
    PRIVATE
    ${CMAKE_PROJECT_NAME}-obj
    Boost::unit_test_framework)
  add_test(test-suite test-suite)
else()
  message(
    WARNING "Test suite will not be launched because Boost.Test not found.")
endif()

enable_testing()

### Packing ###################################################################

include(GNUInstallDirs)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/bayan.1
  ${CMAKE_CURRENT_BINARY_DIR}/bayan.1)
add_custom_command(
  OUTPUT bayan.1.gz
  PRE_BUILD
  COMMAND gzip -k bayan.1
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "Compressing man page"
  VERBATIM)
add_custom_target(
  compress-man
  ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/bayan.1.gz)
install(TARGETS ${CMAKE_PROJECT_NAME} RUNTIME DESTINATION bin)
install(
  FILES ${CMAKE_BINARY_DIR}/bayan.1.gz
  DESTINATION ${CMAKE_INSTALL_MANDIR}/man1/)

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_CONTACT bergentroll@insiberia.net)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>=2.15)")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

include(CPack)
