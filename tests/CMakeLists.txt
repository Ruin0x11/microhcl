project(hcl)

cmake_minimum_required(VERSION 2.8)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# C++14 or later
enable_language(CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(${CMAKE_VERSION} VERSION_LESS 2.8.12)
    message("Your cmake version is ${CMAKE_VERSION}. Please update it to 2.8.12 or newer.")
    function(add_compile_options option)
        add_definitions(${option} ${ARGN})
    endfunction()
endif()

if(MSVC)
    add_compile_options("/MT$<$<CONFIG:Debug>:d>")
else()
    add_compile_options("-Wall" "-Wextra" "-std=c++11")
endif()

add_definitions(-DSRC_DIR="${CMAKE_SOURCE_DIR}")
#add_definitions(-DTESTCASE_DIR="${CMAKE_SOURCE_DIR}/../testcase")

# Force maps to be ordered for testing equality
add_definitions(-DMICROHCL_USE_MAP)

set (CATCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/catch2})
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})
include_directories(../include)

#add_executable(parse_stdin parse_stdin.cc)
#add_executable(parse_file parse_file.cc)
#add_executable(parse_file2 parse_file_2.cc)

set(TEST_SOURCES
  decoding_test.cpp
  lexer_test.cpp
  parser_test.cpp
  value_test.cpp)

add_executable(test_runner ${TEST_SOURCES} main.cpp)
target_link_libraries(test_runner Catch)
add_custom_command(TARGET test_runner POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   "${CMAKE_CURRENT_SOURCE_DIR}/test-fixtures"
                   "$<TARGET_FILE_DIR:test_runner>/tests/test-fixtures")
