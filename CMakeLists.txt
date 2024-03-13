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
    ${CMAKE_SOURCE_DIR}/include/MU_3SL_interface.c
)

# 添加可执行文件，并指定源文件
add_executable(mu src/calibration_offline_data_file.c)
target_link_libraries(mu m MyLibrary)
# target_link_libraries(mu MU_3SL_interface)

target_link_options(mu PRIVATE "-Wl,-rpath,/home/dingdangda/ic-mu/muproject/lib")

# find_library(MU_3SL_INTERFACE_LIB NAMES MU_3SL_interface.lib PATHS ${CMAKE_SOURCE_DIR}/lib)
# find_library(MU_3SL_INTERFACE_64_LIB NAMES MU_3SL_interface_64.lib PATHS ${CMAKE_SOURCE_DIR}/lib)
# target_link_libraries(mu ${MU_3SL_INTERFACE_LIB} ${MU_3SL_INTERFACE_64_LIB})
