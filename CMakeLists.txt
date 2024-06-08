cmake_minimum_required(VERSION 3.20)

project(libmedia VERSION 0.0.1)

add_executable(mp4_dump mp4_dump.cc
    file_view_mmap.cc
    #file_view_std.cc
)

set_property(TARGET mp4_dump PROPERTY CXX_STANDARD 23)
set_property(TARGET mp4_dump PROPERTY CXX_STANDARD_REQUIRED TRUE)
set_property(TARGET mp4_dump PROPERTY CXX_EXTENSIONS OFF)
