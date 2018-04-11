cmake_minimum_required(VERSION 2.6)
SET(GCC_COVERAGE_COMPILE_FLAGS "-Wall -Wextra -pedantic")
project(POS-proj2)
add_executable(POS-proj2 main.c cmdparser.c cmdparser.h)
target_link_libraries(POS-proj2 pthread)


add_executable(redirectTest redirectTest.c)
add_executable(cmdparser cmdparser.c)