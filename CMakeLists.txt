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

set(GTEST_SRCDIR "/usr/src/gtest/")
set(GTEST_FOUND Yes)
if(NOT EXISTS "${GTEST_SRCDIR}/CMakeLists.txt")
  set(GTEST_SRCDIR "/usr/src/googletest/")
endif()
if(NOT EXISTS "${GTEST_SRCDIR}/CMakeLists.txt")
  set(GTEST_FOUND No)
endif()

find_package(Boost 1.65 COMPONENTS program_options REQUIRED)

### Main targets ##############################################################

add_executable(${CMAKE_PROJECT_NAME} ${SRC_PATH}/main.cpp)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIR})
target_link_libraries(
  ${CMAKE_PROJECT_NAME}
  stdc++fs
  ${Boost_PROGRAM_OPTIONS_LIBRARY})

### Tests #####################################################################

if(GTEST_FOUND)
  add_subdirectory(${GTEST_SRCDIR}/ ${CMAKE_BINARY_DIR}/gtest/ EXCLUDE_FROM_ALL)

  add_executable(test_suite ${TEST_PATH}/test.cpp)
  target_link_libraries(
    test_suite
    PRIVATE
    gtest
    gtest_main)
  add_test(test_suite test_suite)
else()
  message(
    WARNING
    "Test suite will not be launched because GTest sources is not found at "
    "${GTEST_SRCDIR}.")
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
  COMMAND gzip bayan.1
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
