# 显示详细过程： make VERBOSE=on make VERBOSE=1 make V=1

#指定使用该CMakeList.txt文件需要的cmake最低版本
cmake_minimum_required(VERSION 3.5)

# for gdb -g
#set(CMAKE_BUILD_TYPE "Debug")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -ggdb3 -g3 ")


set(CMAKE_CXX_STANDARD 17)

#find_package(PkgConfig)
#include(GNUInstallDirs)


set(CMAKE_VERBOSE_MAKEFILE 2)

#指定项目信息
project(repo_mirror_push)
set(TARGET_NAME ${CMAKE_PROJECT_NAME})



# include and source
include_directories(inc/base)    #头文件路径
include_directories(inc/core)

aux_source_directory(src ALL_SRCS)  # 查找src目录下的所有源文件, 并将名称保存到ALL_SRCS变量
aux_source_directory(src/base ALL_SRCS)
aux_source_directory(src/core ALL_SRCS)
message("source:  ${ALL_SRCS}")

set(CMAKE_STRIP "strip")
set(CMAKE_BUILD_TYPE "RELEASE")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s -Wno-unused-command-line-argument")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s -Wno-unused-command-line-argument")

#指定生成目标
add_executable(${TARGET_NAME} ${ALL_SRCS})

target_link_libraries(${TARGET_NAME})


