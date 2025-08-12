# 获取当前目录下的所有源文件
file(GLOB EXAMPLE_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)

# 创建可执行文件
add_executable(testserver ${EXAMPLE_SRCS})

# 链接必要的库，比如刚刚我们写好的在 src 文件 CMakeLists 中 muduo-core_lib 静态库，还有全局链接库
target_link_libraries(testserver muduo_core ${LIBS})

# 设置编译选项
target_compile_options(testserver PRIVATE -std=c++11 -Wall)

# 设置 testserver 可执行文件输出目录
set_target_properties(testserver PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
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
