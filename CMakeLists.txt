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

set(LIBMEDIA_FILEVIEW_USE_MMAP OFF)
set(LIBMEDIA_FILEVIEW_USE_WIN32 OFF)
set(LIBMEDIA_FILEVIEW_USE_STD OFF)

set(LIBMEDIA_FILEVIEW_AUTODETECT ON)
if (${LIBMEDIA_FILEVIEW_USE_MMAP} OR ${LIBMEDIA_FILEVIEW_USE_WIN32} OR ${LIBMEDIA_FILEVIEW_USE_STD})
    set(LIBMEDIA_FILEVIEW_AUTODETECT OFF)
endif()

if(${LIBMEDIA_FILEVIEW_AUTODETECT})
    if (WIN32)
        set(LIBMEDIA_FILEVIEW_USE_WIN32 ON)
    elseif(UNIX)
        set(LIBMEDIA_FILEVIEW_USE_MMAP ON)
    else()
        set(LIBMEDIA_FILEVIEW_USE_STD ON)
    endif()
endif()

add_executable(mp4_dump mp4_dump.cc)


add_library(libmedia.fileview STATIC)
target_cxx_23(libmedia.fileview)

if (${LIBMEDIA_FILEVIEW_USE_WIN32})
    message(STATUS "Use WINAPI for FileView")
    target_sources(libmedia.fileview PRIVATE file_view_win32.cc)
endif()

if (${LIBMEDIA_FILEVIEW_USE_MMAP})
    message(STATUS "Use Posix API for FileView")
    target_sources(libmedia.fileview PRIVATE file_view_mmap.cc)
endif()

if(${LIBMEDIA_FILEVIEW_USE_STD})
    message(WARNING "Use c++ API for FileView (slow)")
    target_sources(libmedia.fileview PRIVATE file_view_std.cc)
endif()

target_link_libraries(mp4_dump PRIVATE libmedia.headers libmedia.fileview)

add_library(ct_tests OBJECT
    basic_box_test.cc 
)


target_link_libraries(ct_tests PRIVATE libmedia.headers)

target_cxx_23(mp4_dump)
target_cxx_23(ct_tests)


if(LIBMEDIA_BUILD_LLVM_FUZZ)
    add_subdirectory(llvm_fuzz)
endif()
