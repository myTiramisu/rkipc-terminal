cmake_minimum_required(VERSION 3.10)

#  设置SDK路径
set(SDK_PATH "/home/yuanjl/luckfox-pico")

# 设置编译器
set(CMAKE_C_COMPILER "${SDK_PATH}/tools/linux/toolchain/arm-rockchip830-linux-uclibcgnueabihf/bin/arm-rockchip830-linux-uclibcgnueabihf-gcc")
set(CMAKE_CXX_COMPILER "${SDK_PATH}/tools/linux/toolchain/arm-rockchip830-linux-uclibcgnueabihf/bin/arm-rockchip830-linux-uclibcgnueabihf-g++")

#Processing Parameters 处理参数
if(DEFINED EXAMPLE_DIR)
    message(STATUS "Selected source directory: ${EXAMPLE_DIR}")
else()
    message(FATAL_ERROR "No source directory selected!")
    return()
endif()

if(DEFINED EXAMPLE_NAME)
    message(STATUS "Set project name: ${EXAMPLE_NAME}")
else()
    message(FATAL_ERROR "Project name not specified!")
    return()
endif()

# 创建项目
project(${EXAMPLE_NAME})

# 链接目录
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/arm-rockchip830-linux-uclibcgnueabihf)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/ 3rdparty.out)

# 设置RPATH
set(CMAKE_INSTALL_RPATH "$ORIGIN/lib")
add_definitions(-DRV1106_1103)

#Opencv 4 查找OpenCV
set(OpenCV_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/cmake/opencv4")
find_package(OpenCV REQUIRED)
#Thread
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# 源文件
set(SRC_DIR "${EXAMPLE_DIR}/src")
set(CODE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/code")
file(GLOB SRC_FILES 
                    "${CODE_DIR}/param/*.c"
                    "${CODE_DIR}/osd/*.c"
                    "${CODE_DIR}/*.c"
                    "${CODE_DIR}/*.cc"
                    "${SRC_DIR}/*.cc" 

)  
add_executable(${PROJECT_NAME} ${SRC_FILES})

add_compile_options(-g -Wall
                    -DISP_HW_V30 -DRKPLATFORM=ON -DARCH64=OFF
                    -DROCKIVA -DUAPI2
                    -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 
                    )
# Set linker flags to handle runtime paths for shared libraries
# set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath-link,${CMAKE_CURRENT_SOURCE_DIR}/lib:/usr/lib")


# Set linker flags to handle runtime paths for shared libraries
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-as-needed,-rpath-link,${CMAKE_CURRENT_SOURCE_DIR}/lib:/usr/lib")

link_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib/arm-rockchip830-linux-uclibcgnueabihf")

target_link_libraries(${PROJECT_NAME}  
                    ${OpenCV_LIBS}
                    ${LIBRKNNRT}
                    Threads::Threads
                    rockiva
                    sample_comm
                    rockit
                    rockchip_mpp
                    rkaiq
                    pthread
                    rtsp
                    freetype
                    iconv
                    )
# 头文件
target_include_directories(${PROJECT_NAME} PRIVATE                        
                            ${OpenCV_INCLUDE_DIRS}
                            ${CMAKE_CURRENT_SOURCE_DIR}/code
                            ${EXAMPLE_DIR}/include
                            ${CMAKE_CURRENT_SOURCE_DIR}
                            # ${CMAKE_CURRENT_SOURCE_DIR}/utils
                            ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/rknpu2/include
                            ${CMAKE_CURRENT_SOURCE_DIR}/common
                            ${CMAKE_CURRENT_SOURCE_DIR}/common/isp3.x   
                            ${CMAKE_CURRENT_SOURCE_DIR}/include
                            ${CMAKE_CURRENT_SOURCE_DIR}/include/rknn
                            ${CMAKE_CURRENT_SOURCE_DIR}/include/rkaiq
                            ${CMAKE_CURRENT_SOURCE_DIR}/include/rkaiq/uAPI2
                            ${CMAKE_CURRENT_SOURCE_DIR}/include/rkaiq/common
                            ${CMAKE_CURRENT_SOURCE_DIR}/include/rkaiq/xcore
                            ${CMAKE_CURRENT_SOURCE_DIR}/include/rkaiq/algos
                            ${CMAKE_CURRENT_SOURCE_DIR}/include/rkaiq/iq_parser
                            ${CMAKE_CURRENT_SOURCE_DIR}/include/rkaiq/iq_parser_v2
                            ${CMAKE_CURRENT_SOURCE_DIR}/include/rkaiq/smartIr
                            ${CMAKE_CURRENT_SOURCE_DIR}/include/freetype2
                            ${CMAKE_CURRENT_SOURCE_DIR}/code/osd
                            ${CMAKE_CURRENT_SOURCE_DIR}/code/param
                          )
                          

message(STATUS "model dir PATH: ${EXAMPLE_DIR}/model")

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/install/${PROJECT_NAME}_demo")
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
SET(MODEL_DIR "${EXAMPLE_DIR}/model")
file(GLOB MODEL_FILES "${MODEL_DIR}/*")  
install(FILES ${MODEL_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/model)                    