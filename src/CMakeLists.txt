#获取当前目录下的所有源文件
file(GLOB SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)

#创建静态库或共享库
add_library(muduo_lite SHARED ${SRC_FILES})

#设置头文件的路径
target_include_directories(muduo_lite PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_features(muduo_lite PUBLIC cxx_std_20)