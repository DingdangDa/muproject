cmake_minimum_required(VERSION 3.10)  # 设定所需的 CMake 最低版本

project(MyProject)  # 设置项目名称

enable_testing()

add_subdirectory(src)  # 添加子目录

# 添加包含目录
include_directories(${CMAKE_SOURCE_DIR}/include)

link_directories(${CMAKE_SOURCE_DIR}/lib)

# 创建一个库并添加 include 文件夹中的 .c 文件
add_library(MyLibrary STATIC 
    ${CMAKE_SOURCE_DIR}/include/csv_file.c
    ${CMAKE_SOURCE_DIR}/include/mu_3sl_calibration_adjustments.c
    ${CMAKE_SOURCE_DIR}/include/mu_3sl_nonius_curve.c
)

# 添加可执行文件，并指定源文件
add_executable(mu src/calibration_offline_data_file.c)
target_link_libraries(mu MU_3SL_interface m MyLibrary)

target_link_options(mu PRIVATE "-Wl,-rpath,/home/dingdangda/ic-mu/muproject/lib")