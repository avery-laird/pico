cmake_minimum_required(VERSION 3.9)
project(PieceTable C)

set(CMAKE_C_STANDARD 99)

add_executable(PieceTable PieceTable.h PieceTable.c ValidateTree.c ValidateTree.h types.h)

add_executable(Tests Tests.c PieceTable.h PieceTable.c)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

add_executable(Pico pico.c PieceTable.c PieceTable.h)
target_link_libraries(Pico ${CURSES_LIBRARIES})
