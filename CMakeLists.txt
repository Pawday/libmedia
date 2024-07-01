cmake_minimum_required(VERSION 3.20)

project(libmedia VERSION 0.0.1)



function(target_cxx_23 TARGET)
    set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 23)
    set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD_REQUIRED TRUE)
    set_property(TARGET ${TARGET} PROPERTY CXX_EXTENSIONS OFF)
endfunction()

add_executable(mp4_dump mp4_dump.cc
    file_view_mmap.cc
    #file_view_std.cc
)

add_library(ct_tests OBJECT
    basic_box_test.cc
)

target_cxx_23(mp4_dump)
target_cxx_23(ct_tests)
