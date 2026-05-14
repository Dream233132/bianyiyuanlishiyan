# Makefile for C-- Manual Lexer
# 手动实现的词法分析器编译脚本

CXX = g++
CXXFLAGS = -std=c++11 -Wall

# 目标文件
TARGET = lexer
SOURCE = lexer.cpp

# 默认目标
all: $(TARGET)

# 编译词法分析器
$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) $(SOURCE) -o $(TARGET)

# 测试正确的程序
test_correct: $(TARGET)
	@echo "=== 测试正确的C--程序 ==="
	./$(TARGET) test_correct.c

# 测试错误的程序
test_error: $(TARGET)
	@echo "=== 测试含有错误的C--程序 ==="
	./$(TARGET) test_error.c

# 运行所有测试
test: test_correct test_error

# 清理生成的文件
clean:
	rm -f $(TARGET) *.exe

# Windows环境清理
clean_win:
	del /Q $(TARGET).exe 2>nul

.PHONY: all test test_correct test_error clean clean_win
