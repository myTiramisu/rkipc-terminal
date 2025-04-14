cmake_minimum_required(VERSION 3.10)

project(onvif_server C)

set(CMAKE_C_STANDARD 99)

# 读取 SDK_PATH 环境变量
if(NOT DEFINED ENV{SDK_PATH})
    message(FATAL_ERROR "SDK_PATH 环境变量未设置！请先执行: \nexport SDK_PATH=/path/to/your/luckfox-pico")
endif()

set(SDK_PATH $ENV{SDK_PATH})
message(STATUS "SDK_PATH is set to ${SDK_PATH}")

# 设置交叉编译工具链
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(TOOLCHAIN_PATH ${SDK_PATH}/tools/linux/toolchain/arm-rockchip830-linux-uclibcgnueabihf/bin)
set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/arm-rockchip830-linux-uclibcgnueabihf-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/arm-rockchip830-linux-uclibcgnueabihf-g++)

# 如果你需要链接到 SDK 中的 sysroot
set(CMAKE_FIND_ROOT_PATH ${SDK_PATH}/tools/linux/toolchain/arm-rockchip830-linux-uclibcgnueabihf/arm-rockchip830-linux-uclibcgnueabihf/sysroot)

# 编译器选项
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -DWITH_DOM -DWITH_OPENSSL")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -DWITH_NO_C_LOCALE")

# 包含头文件路径
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 源文件列表
set(SOURCES
    # soap生成的文件
    soapC.c
    soapServer.c
    soapClient.c
    stdsoap2.c

    # 服务器代码
    onvif_server.c
    onvif_server_interface.c

    # ONVIF服务实现
    service_device.c
    service_media.c
    service_ptz.c
    service_event.c

    # 工具函数
    utils.c
)

# 生成可执行文件
add_executable(onvif_server ${SOURCES})

# 链接系统所需库
target_link_libraries(onvif_server m pthread)
