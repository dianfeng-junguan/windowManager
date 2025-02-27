cmake_minimum_required(VERSION 3.15)
project(wndman)

file(GLOB_RECURSE SRC_FILES src/*.c)

add_executable(wndman ${SRC_FILES})
target_include_directories(wndman PUBLIC include)
target_compile_definitions(wndman PRIVATE -DDEBUG)
if(DEBUG)
    target_compile_options(wndman PRIVATE -g -emain)
else(DEBUG)
    target_compile_options(wndman PRIVATE -g -fno-pie -fno-pic -nostdlib -fno-stack-protector -nostartfiles -I include -m64 -O0 -fno-strict-aliasing -mcmodel=large -fPIC)

endif(DEBUG)
