#设置最低版本和项目名称
cmake_minimum_required(VERSION 3.0)
project(muduo-core) #锁定项目的工作目录

#设置全局的c++标准
set (CMAKE_CXX_STANDARD 11) 
set (CMAKE_CXX_STANDARD_REQUIRED True)

#链接必要的库，比如刚刚我们写好的在src文件Cmakelists中muduo-core_lib静态库，还有全局链接库
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
#示例静态库的输出
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib) # 设置静态库输出路径
#设置全局链接库
set(LIBS
    pthread
)

#添加子目录
add_subdirectory(src)
add_subdirectory(example)
