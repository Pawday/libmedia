function(make_llvm_fuzz TARGET)
    target_cxx_23(${TARGET})
    target_compile_options(${TARGET} PRIVATE "-fsanitize=fuzzer")
    target_link_options(${TARGET} PRIVATE "-fsanitize=fuzzer")
    target_link_libraries(${TARGET} PRIVATE libmedia.headers)
endfunction()

add_executable(box_fuzz box.cc)
make_llvm_fuzz(box_fuzz)
