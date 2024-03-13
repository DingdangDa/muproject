cmake_minimum_required(VERSION 3.10)

project(MyProject)

enable_testing()

add_subdirectory(src)

include_directories(${CMAKE_SOURCE_DIR}/include)

link_directories(${CMAKE_SOURCE_DIR}/lib)

add_library(MuLib STATIC 
    ${CMAKE_SOURCE_DIR}/include/csv_file.c
    ${CMAKE_SOURCE_DIR}/include/mu_3sl_calibration_adjustments.c
    ${CMAKE_SOURCE_DIR}/include/mu_3sl_nonius_curve.c
)

add_executable(mu src/calibration_offline_data_file.c)
target_link_libraries(mu m MuLib)

# For My Ubuntu
target_link_libraries(mu MU_3SL_interface)
target_link_options(mu PRIVATE "-Wl,-rpath,/home/dingdangda/ic-mu/muproject/lib")

# For My Windows
find_library(MU_3SL_INTERFACE_LIB NAMES MU_3SL_interface.lib PATHS ${CMAKE_SOURCE_DIR}/lib)
find_library(MU_3SL_INTERFACE_64_LIB NAMES MU_3SL_interface_64.lib PATHS ${CMAKE_SOURCE_DIR}/lib)
target_link_libraries(mu ${MU_3SL_INTERFACE_LIB} ${MU_3SL_INTERFACE_64_LIB})
