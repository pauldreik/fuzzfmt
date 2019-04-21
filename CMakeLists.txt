# for fuzzing libfmt http://fmtlib.net/
#
# by Paul Dreik 20190420
# https://www.pauldreik.se/


cmake_minimum_required(VERSION 3.10)

project(fmt_fuzzers LANGUAGES CXX)

add_definitions(-DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION=1)

add_subdirectory(fmt)


# settings this links in a main. useful for reproducing,
# kcov, gdb, afl, valgrind.
# (note that libFuzzer can also reproduce, just pass it the files)
option(reproduce_mode "enables the reproduce mode, instead of libFuzzer" On)

#find all fuzzers.
file(GLOB SOURCES "fuzz*.cpp")

if(reproduce_mode)
  set(prefix reproducer_)
  add_definitions(-DIMPLEMENT_MAIN=1)
else()
  # this assumes clang is used
  string(APPEND CMAKE_CXX_FLAGS " -fsanitize=fuzzer")
  set(prefix fuzzer_)
endif()

macro(implement_fuzzer sourcefile)
  get_filename_component(basename ${sourcefile} NAME_WE)
  add_executable(${prefix}${basename} ${sourcefile})
  target_link_libraries(${prefix}${basename}  PRIVATE fmt)
  set_property(TARGET ${prefix}${basename} PROPERTY CXX_STANDARD 17)
endmacro()

foreach(X IN ITEMS ${SOURCES})
    implement_fuzzer(${X})
endforeach()
