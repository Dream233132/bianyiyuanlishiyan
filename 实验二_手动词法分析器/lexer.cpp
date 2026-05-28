/*
 * C--语言词法分析器 - 手动实现（状态转换图法）
 * 功能：使用状态转换图手工构造词法分析器
 */

#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <map>
#include <cstdlib>
using namespace std;

// 1. 种别码定义：为语言中每一种合法的基本单词分配一个唯一的整数ID
enum TokenType {
    WHILE = 1, IF = 2, ELSE = 3, SWITCH = 4, CASE = 5,      // 关键字
    ID = 6, NUM = 7,                                        // 标识符和数字常量
    PLUS = 8, QUESTION = 9, MULTIPLY = 10,                  // 算术与特殊运算符
    LE = 10, LT = 11, EQ = 12, ASSIGN = 13, COLON = 13,     // 关系与赋值运算符（注意代码中这里有些编码复用了，比如LE和MULTIPLY都是10，ASSIGN和COLON都是13，视具体实验要求而定）
    MINUS = 14, DIVIDE = 15, GT = 16, GE = 17,
    LPAREN = 18, RPAREN = 19, LBRACE = 20, RBRACE = 21,     // 界符 ( ) { }
    SEMICOLON = 22, COMMA = 23,                             // 界符 ; ,
    TOKEN_END = 0, TOKEN_ERROR = -1                         // 结束标志与错误标志
};

// 2. 关键字表：用于在识别出标识符后，判断它究竟是普通变量名还是保留字
map<string, TokenType> keywords = {
    {"while", WHILE}, {"if", IF}, {"else", ELSE},
    {"switch", SWITCH}, {"case", CASE}
};

// 3. Token结构：每次词法分析成功后返回的“二元组”实体
struct Token {
    TokenType type;  // 种别码
    string value;    // 单词的具体字符串（如 "123", "count", "+="）
    int line;        // 所在行号（用于报错定位）
};

// 4. 词法分析器核心类
class Lexer {
private:
    string input;      // 存放从文件中读取的全部源代码字符串
    size_t pos;        // 当前扫描到的字符在input中的索引位置
    int line;          // 记录当前解析到了第几行
    char currentChar;  // 记录当前正在考察的字符
    
    // 【基础动作】读取下一个字符，并维护行号
    void advance() {
        if (pos < input.length()) {
            currentChar = input[pos++];// 先复制，再指针pos++
            if (currentChar == '\n') {
                line++; // 如果读到换行符，行号加1
            }
        } else {
            currentChar = '\0'; // 读到末尾设为结束符
        }
    }
    
    // 【基础动作】超前探查（Lookahead）：看一眼下一个字符是什么，但不移动实际的指针 pos
    // 这对于识别 "==" 还是 "="，"<=" 还是 "<" 非常关键
    char peek() {
        if (pos < input.length()) {
            return input[pos];// 返回下一个字符，但不修改 pos 和 currentChar
        }
        return '\0';
    }
    
    // 跳过所有空白字符（空格、制表符、换行等）
    void skipWhitespace() {
        while (currentChar != '\0' && isspace(currentChar)) {
            advance();
        }
    }
    
    // 跳过代码中的注释（单行 // 和 多行 /* */）
    void skipComment() {
        if (currentChar == '/' && peek() == '/') {  // 当前字符是 '/'，并且下一个字符是 '/'
            // 处理单行注释 //
            pos++; // 跳过第二个 '/'
            // 一直往后找，直到遇到换行符或文件结束
            while (pos < input.length()) {
                if (input[pos] == '\n') {
                    line++;
                    pos++;
                    break;
                }
                pos++;
            }
            // 注释跳过完毕，更新当前字符为换行符后的第一个字符
            if (pos < input.length()) {
                currentChar = input[pos];
                pos++;
            } else {
                currentChar = '\0';
            }
        } else if (currentChar == '/' && peek() == '*') {  // 当前字符是 '/'，并且下一个字符是 '*'
            // 处理多行注释 /*
            pos++; // 跳过 '*'
            // 一直往后找，直到遇到连续的 "*/" 或文件结束
            while (pos < input.length()) {
                if (input[pos] == '\n') {
                    line++; // 多行注释内部可能有换行，需正确累加行号
                }
                // 检查是否遇到了 "*/" 的闭合标记
                if (input[pos] == '*' && pos + 1 < input.length() && input[pos + 1] == '/') {
                    pos += 2; // 跳过 "*/" 两个字符
                    break;
                }
                pos++;
            }
            // 注释跳过完毕，更新状态
            if (pos < input.length()) {
                currentChar = input[pos];
                pos++;
            } else {
                currentChar = '\0';
            }
        }
    }
    
    // 【状态图：标识符/关键字】匹配正则：[a-zA-Z_][a-zA-Z0-9_]*
    Token recognizeIdentifier() {
        Token token;
        token.line = line;
        string result = "";
        
        // 只要遇到的是字母、数字或下划线，就一直拼接（状态自旋）
        while (currentChar != '\0' && (isalnum(currentChar) || currentChar == '_')) {
            result += currentChar;
            advance();
        }
        
        // 查表判断：如果拼接出的字符串在关键字表中，说明是关键字；否则是普通标识符ID
        if (keywords.find(result) != keywords.end()) {
            token.type = keywords[result];
            token.value = result;
        } else {
            token.type = ID;
            token.value = result;
        }
        
        return token;
    }
    
    // 【状态图：数字常量】支持整数、小数(.)、科学计数法(e/E)
    Token recognizeNumber() {
        Token token;
        token.type = NUM;
        token.line = line;
        string result = "";
        
        // 步骤1：读取开头的整数部分
        while (currentChar != '\0' && isdigit(currentChar)) {
            result += currentChar;
            advance();
        }
        
        // 步骤2：检查是否有小数点（识别浮点数）
        if (currentChar == '.') {
            result += currentChar;
            advance();
            
            // 读取小数部分
            while (currentChar != '\0' && isdigit(currentChar)) {
                result += currentChar;
                advance();
            }
        }
        
        // 步骤3：检查是否有科学计数法标志 e 或 E
        if (currentChar == 'e' || currentChar == 'E') {
            result += currentChar;
            advance();
            
            // 检查指数部分的符号位 (+ 或 -)
            if (currentChar == '+' || currentChar == '-') {
                result += currentChar;
                advance();
            }
            
            // 读取指数部分的数字
            while (currentChar != '\0' && isdigit(currentChar)) {
                result += currentChar;
                advance();
            }
        }
        
        token.value = result;
        return token;
    }
    
public:
    // 构造函数：初始化时读取第一个字符
    Lexer(const string& source) : input(source), pos(0), line(1), currentChar('\0') {
        if (input.length() > 0) {
            currentChar = input[0];
            pos = 1;  
        }
    }
    
    // 【主入口】外部调用此方法，每次获取一个有效Token
    Token getNextToken() {
        while (currentChar != '\0') {
            // 1. 跳过空格和空白符
            if (isspace(currentChar)) {
                skipWhitespace();
                continue;
            }

            // 2. 跳过注释
            // 必须在处理除号 '/' 之前进行拦截，如果在处理除号之前没有先检查注释，
            //那么当遇到 "//" 或 "/*" 时就会误认为是除号，导致词法分析错误。
            if (currentChar == '/') {
                char next = peek();
                if (next == '/' || next == '*') {
                    skipComment(); // 处理注释
                    continue; // 跳过注释后，直接进入下一轮循环寻找新Token
                }
            }

            // 准备构建 Token
            Token token;
            token.line = line;
            
            // 3. 进入分支：如果以字母或下划线开头，走标识符识别状态机
            if (isalpha(currentChar) || currentChar == '_') {
                return recognizeIdentifier();// 识别标识符或关键字，并返回Token
            }
            
            // 4. 进入分支：如果以数字开头，走数字常量识别状态机
            if (isdigit(currentChar)) {
                return recognizeNumber();   // 识别数字常量，并返回Token
            }
            
            // 5. 进入分支：特殊符号与运算符识别
            switch (currentChar) {
                // 单字符运算符
                case '+':
                    token.type = PLUS; token.value = "+"; advance(); return token;
                case '-':
                    token.type = MINUS; token.value = "-"; advance(); return token;
                case '*':
                    token.type = MULTIPLY; token.value = "*"; advance(); return token;
                case '/': // 前面已经排除了注释，这里只能是除号
                    token.type = DIVIDE; token.value = "/"; advance(); return token;
                    
                // 复合运算符（需要超前探查 peek 的情况）
                case '<':
                    advance(); // 吞掉 '<'
                    if (currentChar == '=') { // 看看下一个是不是 '='
                        token.type = LE; token.value = "LE"; advance();
                    } else {
                        token.type = LT; token.value = "LT";
                    }
                    return token;
                    
                case '>':
                    advance(); // 吞掉 '>'
                    if (currentChar == '=') {
                        token.type = GE; token.value = "GE"; advance();
                    } else {
                        token.type = GT; token.value = "GT";
                    }
                    return token;
                    
                case '=':
                    advance(); // 吞掉 '='
                    if (currentChar == '=') {
                        token.type = EQ; token.value = "EQ"; advance();
                    } else {
                        token.type = ASSIGN; token.value = "=";
                    }
                    return token;
                    
                // 其他单字符界符
                case '?': token.type = QUESTION; token.value = "?"; advance(); return token;
                case ':': token.type = COLON; token.value = ":"; advance(); return token;
                case '(': token.type = LPAREN; token.value = "("; advance(); return token;
                case ')': token.type = RPAREN; token.value = ")"; advance(); return token;
                case '{': token.type = LBRACE; token.value = "{"; advance(); return token;
                case '}': token.type = RBRACE; token.value = "}"; advance(); return token;
                case ';': token.type = SEMICOLON; token.value = ";"; advance(); return token;
                case ',': token.type = COMMA; token.value = ","; advance(); return token;
                    
                // 非法字符处理：不属于语言定义的字符
                default:
                    token.type = TOKEN_ERROR;
                    token.value = string(1, currentChar);
                    cout << "词法错误：第" << line << "行出现非法字符 '" 
                         << currentChar << "'" << endl;
                    advance(); // 跳过错误字符，继续分析
                    return token;
            }
        }

        // 6. 循环结束，代表已经读到文件末尾 '\0'
        Token token;
        token.type = TOKEN_END;
        token.value = "EOF";
        token.line = line;
        return token;
    }
};

int main(int argc, char* argv[]) {
    // 解决 Windows 控制台中文乱码问题 (代码页 65001 即 UTF-8)
    #ifdef _WIN32
    system("chcp 65001 >nul");
    #endif

    // 参数校验：确保用户通过命令行传入了被测试的源文件路径
    if (argc < 2) {
        cerr << "用法: " << argv[0] << " <源文件>" << endl;
        return 1;
    }
    
    // 打开并读取测试代码文件
    ifstream file(argv[1]);
    if (!file.is_open()) {
        cerr << "无法打开文件: " << argv[1] << endl;
        return 1;
    }
    
    // 快速将整个文件流读取为一个巨大的 string 字符串
    string source((istreambuf_iterator<char>(file)),
                   istreambuf_iterator<char>());
    file.close();
    
    // 开始输出词法分析结果
    cout << "=== C--语言词法分析结果（手动实现）===" << endl;
    cout << "格式: (种别码, 值)" << endl << endl;
    
    // 实例化词法分析器
    Lexer lexer(source);
    Token token;
    
    // 循环驱动引擎：不断索要下一个Token，直到遇到文件结束符
    do {
        token = lexer.getNextToken();
        // 过滤掉 EOF 和报错的Token，只打印正常的Token对
        if (token.type != TOKEN_END && token.type != TOKEN_ERROR) {
            cout << "(" << token.type << ", " << token.value << ")" << endl;
        }
    } while (token.type != TOKEN_END);
    
    return 0;
}