cmake_minimum_required(VERSION 3.20)

project(libmedia VERSION 0.0.1)

option(LIBMEDIA_BUILD_LLVM_FUZZ "" OFF)

add_library(libmedia.headers INTERFACE)
target_include_directories(libmedia.headers INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

function(target_cxx_23 TARGET)
    set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 23)
    set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD_REQUIRED TRUE)
    set_property(TARGET ${TARGET} PROPERTY CXX_EXTENSIONS OFF)
endfunction()

add_executable(mp4_dump mp4_dump.cc
    file_view_mmap.cc
    #file_view_std.cc
)

target_link_libraries(mp4_dump PRIVATE libmedia.headers)

add_library(ct_tests OBJECT
    basic_box_test.cc
)


target_link_libraries(ct_tests PRIVATE libmedia.headers)

target_cxx_23(mp4_dump)
target_cxx_23(ct_tests)


if(LIBMEDIA_BUILD_LLVM_FUZZ)
    add_subdirectory(llvm_fuzz)
endif()
