cmake_minimum_required(VERSION 3.10)

project(anrool_json VERSION 1.0 LANGUAGES CXX)

option(ANROOL_BUILD_TESTS "Build tests" OFF)

add_subdirectory(include)

if (ANROOL_BUILD_TESTS)
    find_package(GTest REQUIRED)
    enable_testing()
    add_subdirectory(test)
endif()
