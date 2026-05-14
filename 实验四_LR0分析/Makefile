# Makefile for LR(0) Parser
# LR(0)语法分析器编译脚本

CXX = g++
CXXFLAGS = -std=c++11 -Wall

# 目标文件
TARGET = lr0_parser
SOURCE = lr0_parser.cpp

# 默认目标
all: $(TARGET)

# 编译分析器
$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) $(SOURCE) -o $(TARGET)

# 测试实验要求的文法
test: $(TARGET)
	@echo "=== 测试文法：E->aA|bB, A->cA|d, B->cB|d ==="
	./$(TARGET) grammar.txt acccd

# 只分析文法，不解析输入
analyze: $(TARGET)
	@echo "=== 分析文法 ==="
	./$(TARGET) grammar.txt

# 测试其他输入串
test_ad: $(TARGET)
	@echo "=== 测试输入串：ad ==="
	./$(TARGET) grammar.txt ad

test_bd: $(TARGET)
	@echo "=== 测试输入串：bd ==="
	./$(TARGET) grammar.txt bd

test_accccd: $(TARGET)
	@echo "=== 测试输入串：accccd ==="
	./$(TARGET) grammar.txt accccd

test_bcccd: $(TARGET)
	@echo "=== 测试输入串：bcccd ==="
	./$(TARGET) grammar.txt bcccd

# 清理生成的文件
clean:
	rm -f $(TARGET) *.exe

# Windows环境清理
clean_win:
	del /Q $(TARGET).exe 2>nul

.PHONY: all test analyze test_ad test_bd test_accccd test_bcccd clean clean_win
