# 编译器
CC = gcc

# 库路径
LIB_PATH = ../_install/include/
LIB_INS_PATH = ../_install/lib/

# 编译选项
CFLAGS = -Wall -g -I$(LIB_PATH)

# 链接选项
LDFLAGS = -L $(LIB_INS_PATH) -lgsoap -lpthread

# 目标可执行文件
TARGET = a.out

# 源文件
SRCS = main.c onvif_server.c duration.c soapServer.c stdsoap2.c soapC.c onvif_server_interface.c 
#  soapClient.c
# 对象文件
OBJS = $(SRCS:.c=.o)
OBJS := $(patsubst %,build/%,$(OBJS))

# 默认目标
all: build $(TARGET)

# 创建 build 文件夹
build:
	@mkdir -p build

# 链接对象文件生成可执行文件
$(TARGET): $(OBJS)
	@$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)


# 编译源文件生成对象文件
build/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 清理生成的文件
clean:
	rm -rf build
	rm -f $(TARGET)
