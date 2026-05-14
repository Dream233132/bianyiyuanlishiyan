# Makefile for C-- Lexer using Flex
# 编译和运行词法分析器

CC = gcc
FLEX = flex
CFLAGS = -Wall -fexec-charset=GBK

# 目标文件
TARGET = lexer
LEX_SOURCE = lexer.l
LEX_OUTPUT = lex.yy.c

# 默认目标
all: $(TARGET)

# 编译词法分析器
$(TARGET): $(LEX_SOURCE)
	$(FLEX) $(LEX_SOURCE)
	$(CC) $(CFLAGS) $(LEX_OUTPUT) -o $(TARGET)

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
	rm -f $(TARGET) $(LEX_OUTPUT) *.exe

# Windows环境清理
clean_win:
	del /Q $(TARGET).exe $(LEX_OUTPUT) 2>nul

.PHONY: all test test_correct test_error clean clean_win
