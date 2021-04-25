set(JSON_TESTS
    test_construction.cpp
    test_getter.cpp
    test_lexer.cpp
    test_parser.cpp
    test_ptr_getter.cpp
    test_ref_getter.cpp
    test_serializer.cpp
    )

foreach(test ${JSON_TESTS})
    get_filename_component(test_base ${test} NAME_WE)
    add_executable(${test_base} ${test})
    target_compile_options(${test_base} PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic -Werror -O2>
    )
    target_link_libraries(${test_base} PRIVATE
        ${PROJECT_NAME}::${PROJECT_NAME}
        GTest::GTest
        GTest::Main
    )
    add_test(NAME ${test_base} COMMAND ${test_base})
endforeach()

target_compile_options(test_construction PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wno-sized-deallocation>
)
