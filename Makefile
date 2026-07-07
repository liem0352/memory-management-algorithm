#Makefile for Memory Management System

CC = gcc
CFLAGS = -Wall -g -std=c99
TARGET = memory_management
SOURCE = memory_management.c

#默认目标
all: $(TARGET)

#编译目标文件
$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

#清理编译文件
clean:
	del $(TARGET).exe

#运行程序
run: $(TARGET)
	./$(TARGET)

#调试版本
debug: CFLAGS += -DDEBUG
debug: $(TARGET)

.PHONY: all clean run debug