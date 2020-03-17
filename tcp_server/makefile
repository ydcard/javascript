#set by yindi
#date: 20180816

#生成的可执行文件名称
TARGET = DSVTcpServer

#配置各种路径
WORK_DIR = $(shell pwd) 
INCDIR := ./inc
SRCDIR := ./src
TARGET_DIR := ./out

#指定各种编译规则,包含交叉编译
CXX = g++
CC = gcc
AR = ar

#指定即将被编译的文件
#SOURCES := \
#	main.cpp\
#	dsv_secur.cpp\
#	handle_msg.cpp\
#	ByteBuffer.cpp
		  
SOURCES := $(wildcard ./src/*.cpp)

#.o文件规则 
OBJECTS = $(SOURCES:.cpp=.o)
#OBJECTS = $(addprefix $(SRCDIR)/, $(SOURCES:.cpp=.o))
#OBJECTS = $(patsubst %.cpp,%.o,$(SOURCES))
#OBJECTS = \
#	main.o\
#	dsv_secur.o\
#	handle_msg.o\
#	ByteBuffer.o


#编译规则 
#CFLAGS += -Wall -g
CFLAGS += -std=c++11
LIBS = -lcrypto -lpthread -lssl	
INCLUDE_PATH = -I$(INCDIR)				
#VPATH = 
 
$(TARGET_DIR)/$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(CFLAGS) $(LIBS)

#$(OBJECTS): $(SOURCES)
#	$(CXX) $(CFLAGS) $(INCLUDE_PATH) -o $@ -c $<

$(OBJECTS): $(SRCDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CFLAGS) $(INCLUDE_PATH) -o $@  -c $<


all: $(OBJECTS) $(TARGET_DIR)/$(TARGET)

clean:
	rm -rf $(OBJECTS) $(TARGET_DIR)/$(TARGET)
	
