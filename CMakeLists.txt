cmake_minimum_required(VERSION 4.0)

project(TerminalVisualizer C)

set(CMAKE_C_STANDARD 11)

add_executable(TerminalVisualizer main.c)

# link PDH for Windows CPU stats
if(WIN32)
    target_link_libraries(TerminalVisualizer pdh)
endif()
