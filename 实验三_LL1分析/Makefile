# Makefile for LL(1) Parser
# LL(1)语法分析器编译脚本

CXX = g++
CXXFLAGS = -std=c++11 -Wall

# 目标文件
TARGET = ll1_parser
SOURCE = ll1_parser.cpp

# 默认目标
all: $(TARGET)

# 编译分析器
$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) $(SOURCE) -o $(TARGET)

# 测试文法1（算术表达式）
test1: $(TARGET)
	@echo "=== 测试文法1：算术表达式 ==="
	./$(TARGET) grammar.txt "id + id * id"

# 测试文法2（if-else语句）
test2: $(TARGET)
	@echo "=== 测试文法2：if-else语句 ==="
	./$(TARGET) grammar2.txt "i b t a e a"

# 只分析文法，不解析输入
analyze1: $(TARGET)
	@echo "=== 分析文法1 ==="
	./$(TARGET) grammar.txt

analyze2: $(TARGET)
	@echo "=== 分析文法2 ==="
	./$(TARGET) grammar2.txt

# 运行所有测试
test: test1 test2

# 清理生成的文件
clean:
	rm -f $(TARGET) *.exe

# Windows环境清理
clean_win:
	del /Q $(TARGET).exe 2>nul

.PHONY: all test test1 test2 analyze1 analyze2 clean clean_win
