cmake_minimum_required(VERSION 3.20)

project(libmedia)


add_executable(Main Main.cc)

set_property(TARGET Main PROPERTY CXX_STANDARD 20)
set_property(TARGET Main PROPERTY CXX_STANDARD_REQUIRED TRUE)
set_property(TARGET Main PROPERTY CXX_EXTENSIONS OFF)
