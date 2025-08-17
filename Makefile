# 编译器选择（默认为 g++）
CXX ?= g++

# 调试模式开关（默认开启调试）
DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CXXFLAGS += -g -Wall -Wextra
else
    CXXFLAGS += -O2
endif

# 链接库选项
LDFLAGS = -lpthread -lmysqlclient

# 包含路径
INCLUDES = -I./utils -I./log -I./CGImysql

# 源文件列表
SRCS = main.cpp \
       ./log/log.cpp \
	   ./CGImysql/sql_connection_pool.cpp \

# 生成的目标文件列表
OBJS = $(SRCS:.cpp=.o)

# 最终目标可执行文件
TARGET = server

# 默认构建目标
all: $(TARGET)

# 链接生成可执行文件
$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)
	rm -f $(OBJS)

# 通用编译规则（.cpp -> .o）
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# 清理生成的文件
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean