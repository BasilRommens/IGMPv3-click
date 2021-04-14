cmake_minimum_required(VERSION 3.17)
project(TCSP_IGMPv3)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_custom_target(TCSP_IGMPv3 COMMAND make -C ${TCSP_IGMPv3_SOURCE_DIR}
        CLION_EXE_DIR=${PROJECT_BINARY_DIR})

add_executable(test mylib)
target_include_directories(mylib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(main PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/elements)
