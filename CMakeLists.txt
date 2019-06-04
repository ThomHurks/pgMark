cmake_minimum_required(VERSION 3.10.3)

project(pgMark VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH  ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR})

add_library(pugixml src/pugixml.cpp)

add_library(regex
        src/regex_parser/state.cpp
        src/regex_parser/state.h
        src/regex_parser/sre_parse.cpp
        src/regex_parser/sre_parse.h
        src/regex_parser/subpattern.cpp
        src/regex_parser/subpattern.h
        src/regex_parser/tokenizer.cpp
        src/regex_parser/tokenizer.h
        src/regex_parser/opcode.cpp
        src/regex_parser/opcode.h
        src/regex_parser/branch.cpp
        src/regex_parser/branch.h
        src/regex_parser/in.cpp
        src/regex_parser/in.h
        src/regex_parser/group_ref_exists.cpp
        src/regex_parser/group_ref_exists.h
        src/regex_parser/group_ref.cpp
        src/regex_parser/group_ref.h
        src/regex_parser/subpattern_opcode.cpp
        src/regex_parser/subpattern_opcode.h
        src/regex_parser/min_repeat.cpp
        src/regex_parser/min_repeat.h
        src/regex_parser/max_repeat.cpp
        src/regex_parser/max_repeat.h
        src/regex_parser/category_types.h
        src/regex_parser/character_sets.cpp
        src/regex_parser/character_sets.h)

add_executable(pgMark
        src/graph_generator.cpp
        src/graph_generator.h
        src/main.cpp
        src/configuration.cpp
        src/configuration.h
        src/schema.cpp
        src/schema.h
        src/random_distribution.cpp
        src/random_distribution.h
        src/attribute.cpp
        src/attribute.h
        src/affinity.h
        src/relation_distribution.h
        src/main.h
        src/node_attribute_generator.cpp
        src/node_attribute_generator.h
        src/random_string_generator.cpp
        src/random_string_generator.h)

set_target_properties(pgMark regex PROPERTIES
                      CXX_EXTENSIONS OFF
                      CXX_STANDARD 17
                      CXX_STANDARD_REQUIRED ON
                      CXX_CLANG_TIDY "clang-tidy;-checks=*,-cppcoreguidelines-pro-bounds-array-to-pointer-decay,-cppcoreguidelines-pro-type-member-init,-hicpp-no-array-decay,-hicpp-member-init,-fuchsia-default-arguments,-clang-diagnostic-unused-command-line-argument,-clang-diagnostic-unknown-warning-option,-readability-implicit-bool-conversion,-misc-unused-parameters;-fix;-fix-errors;"
                      CMAKE_CXX_CPPLINT ON
                      CMAKE_CXX_INCLUDE_WHAT_YOU_USE ON
                      CMAKE_LINK_WHAT_YOU_USE ON)

target_compile_features(pgMark PUBLIC cxx_std_17)
target_compile_features(regex PUBLIC cxx_std_17)

set(GCC_COMPILER_FLAGS -Wall -Wextra -Wformat=2 -Werror -Wfatal-errors -Wmissing-include-dirs -Wswitch-default
        -Wswitch-enum -Wfloat-equal -Wshadow -Wunsafe-loop-optimizations -Wcast-qual -Wconversion
        -Wsign-conversion -Wlogical-op -Wmissing-declarations -Wredundant-decls -Wunreachable-code -Winline
        -Wctor-dtor-privacy -Wstrict-null-sentinel -Wold-style-cast -Woverloaded-virtual -Wsign-promo
        -Wmultiple-inheritance -Wchkp -Wdouble-promotion -Wformat-overflow=2 -Wformat-signedness -Wformat-truncation=2
        -Wnull-dereference -Wuninitialized -Wsuggest-attribute=pure -Wsuggest-attribute=const
        -Wsuggest-attribute=noreturn -Wsuggest-attribute=format -Wmissing-format-attribute -Wsuggest-final-types
        -Wsuggest-final-methods -Wsuggest-override -Walloc-zero -Wduplicated-branches -Wduplicated-cond -Wpointer-arith
        -Wunused-macros -Wzero-as-null-pointer-constant -Wdate-time -Wuseless-cast -Wno-aggressive-loop-optimizations
        -pedantic-errors -Wno-unused-parameter)

set(CLANG_COMPILER_FLAGS -Werror -Weverything -pedantic-errors -Wno-c++98-compat -Wno-unknown-warning-option
        -Wno-c++98-compat-pedantic -Wno-padded -Wno-weak-vtables -Wno-covered-switch-default -Wno-exit-time-destructors
        -Wno-global-constructors -Wno-unused-parameter -Wno-unused-private-field)

target_compile_options(regex PUBLIC "${CLANG_COMPILER_FLAGS}")
target_compile_options(pgMark PUBLIC "${CLANG_COMPILER_FLAGS}")

target_link_libraries(pgMark pugixml regex)

message("Supported features = ${CMAKE_CXX_COMPILE_FEATURES}")
