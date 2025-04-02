cmake_minimum_required(VERSION 3.30)
project(PA2)

set(CMAKE_CXX_STANDARD 11)

add_executable(PA2 prog2.cpp
        parser.h
        lex.cpp
        A_Beshay_parser.cpp)
