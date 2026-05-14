/*
 * C--语言词法分析器 - 手动实现（状态转换图法）
 * 功能：使用状态转换图手工构造词法分析器
 */

#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <map>
using namespace std;

// 种别码定义
enum TokenType {
    WHILE = 1, IF = 2, ELSE = 3, SWITCH = 4, CASE = 5,
    ID = 6, NUM = 7, PLUS = 8, QUESTION = 9, MULTIPLY = 10,
    LE = 10, LT = 11, EQ = 12, ASSIGN = 13, COLON = 13,
    MINUS = 14, DIVIDE = 15, GT = 16, GE = 17,
    LPAREN = 18, RPAREN = 19, LBRACE = 20, RBRACE = 21,
    SEMICOLON = 22, COMMA = 23, END = 0, ERROR = -1
};

// 关键字表
map<string, TokenType> keywords = {
    {"while", WHILE}, {"if", IF}, {"else", ELSE},
    {"switch", SWITCH}, {"case", CASE}
};

// Token结构
struct Token {
    TokenType type;
    string value;
    int line;
};

class Lexer {
private:
    string input;      // 输入字符串
    int pos;           // 当前位置
    int line;          // 当前行号
    char currentChar;  // 当前字符
    
    // 读取下一个字符
    void advance() {
        if (pos < input.length()) {
            currentChar = input[pos++];
            if (currentChar == '\n') {
                line++;
            }
        } else {
            currentChar = '\0';
        }
    }
    
    // 向前看一个字符（不移动位置）
    char peek() {
        if (pos < input.length()) {
            return input[pos];
        }
        return '\0';
    }
    
    // 跳过空白字符
    void skipWhitespace() {
        while (currentChar != '\0' && isspace(currentChar)) {
            advance();
        }
    }
    
    // 跳过注释
    void skipComment() {
        if (currentChar == '/' && peek() == '/') {
            // 单行注释
            while (currentChar != '\0' && currentChar != '\n') {
                advance();
            }
        } else if (currentChar == '/' && peek() == '*') {
            // 多行注释
            advance(); // 跳过 /
            advance(); // 跳过 *
            while (currentChar != '\0') {
                if (currentChar == '*' && peek() == '/') {
                    advance(); // 跳过 *
                    advance(); // 跳过 /
                    break;
                }
                advance();
            }
        }
    }
    
    // 识别标识符或关键字（状态0->1->1...）
    Token recognizeIdentifier() {
        Token token;
        token.line = line;
        string result = "";
        
        // 状态1：读取字母或下划线开头
        while (currentChar != '\0' && (isalnum(currentChar) || currentChar == '_')) {
            result += currentChar;
            advance();
        }
        
        // 检查是否为关键字
        if (keywords.find(result) != keywords.end()) {
            token.type = keywords[result];
            token.value = result;
        } else {
            token.type = ID;
            token.value = result;
        }
        
        return token;
    }
    
    // 识别数字（整数、浮点数、科学计数法）
    Token recognizeNumber() {
        Token token;
        token.type = NUM;
        token.line = line;
        string result = "";
        
        // 状态1：读取整数部分
        while (currentChar != '\0' && isdigit(currentChar)) {
            result += currentChar;
            advance();
        }
        
        // 状态2：检查小数点
        if (currentChar == '.') {
            result += currentChar;
            advance();
            
            // 状态3：读取小数部分
            while (currentChar != '\0' && isdigit(currentChar)) {
                result += currentChar;
                advance();
            }
        }
        
        // 状态4：检查科学计数法
        if (currentChar == 'e' || currentChar == 'E') {
            result += currentChar;
            advance();
            
            // 状态5：检查正负号
            if (currentChar == '+' || currentChar == '-') {
                result += currentChar;
                advance();
            }
            
            // 状态6：读取指数部分
            while (currentChar != '\0' && isdigit(currentChar)) {
                result += currentChar;
                advance();
            }
        }
        
        token.value = result;
        return token;
    }
    
public:
    Lexer(const string& source) : input(source), pos(0), line(1) {
        currentChar = input.length() > 0 ? input[0] : '\0';
    }
    
    // 获取下一个Token
    Token getNextToken() {
        while (currentChar != '\0') {
            // 跳过空白
            if (isspace(currentChar)) {
                skipWhitespace();
                continue;
            }
            
            // 跳过注释
            if (currentChar == '/' && (peek() == '/' || peek() == '*')) {
                skipComment();
                continue;
            }
            
            Token token;
            token.line = line;
            
            // 识别标识符或关键字
            if (isalpha(currentChar) || currentChar == '_') {
                return recognizeIdentifier();
            }
            
            // 识别数字
            if (isdigit(currentChar)) {
                return recognizeNumber();
            }
            
            // 识别运算符和特殊符号
            switch (currentChar) {
                case '+':
                    token.type = PLUS;
                    token.value = "+";
                    advance();
                    return token;
                    
                case '-':
                    token.type = MINUS;
                    token.value = "-";
                    advance();
                    return token;
                    
                case '*':
                    token.type = MULTIPLY;
                    token.value = "*";
                    advance();
                    return token;
                    
                case '/':
                    token.type = DIVIDE;
                    token.value = "/";
                    advance();
                    return token;
                    
                case '<':
                    advance();
                    if (currentChar == '=') {
                        token.type = LE;
                        token.value = "LE";
                        advance();
                    } else {
                        token.type = LT;
                        token.value = "LT";
                    }
                    return token;
                    
                case '>':
                    advance();
                    if (currentChar == '=') {
                        token.type = GE;
                        token.value = "GE";
                        advance();
                    } else {
                        token.type = GT;
                        token.value = "GT";
                    }
                    return token;
                    
                case '=':
                    advance();
                    if (currentChar == '=') {
                        token.type = EQ;
                        token.value = "EQ";
                        advance();
                    } else {
                        token.type = ASSIGN;
                        token.value = "=";
                    }
                    return token;
                    
                case '?':
                    token.type = QUESTION;
                    token.value = "?";
                    advance();
                    return token;
                    
                case ':':
                    token.type = COLON;
                    token.value = ":";
                    advance();
                    return token;
                    
                case '(':
                    token.type = LPAREN;
                    token.value = "(";
                    advance();
                    return token;
                    
                case ')':
                    token.type = RPAREN;
                    token.value = ")";
                    advance();
                    return token;
                    
                case '{':
                    token.type = LBRACE;
                    token.value = "{";
                    advance();
                    return token;
                    
                case '}':
                    token.type = RBRACE;
                    token.value = "}";
                    advance();
                    return token;
                    
                case ';':
                    token.type = SEMICOLON;
                    token.value = ";";
                    advance();
                    return token;
                    
                case ',':
                    token.type = COMMA;
                    token.value = ",";
                    advance();
                    return token;
                    
                default:
                    // 词法错误
                    token.type = ERROR;
                    token.value = string(1, currentChar);
                    cout << "词法错误：第" << line << "行出现非法字符 '" 
                         << currentChar << "'" << endl;
                    advance();
                    return token;
            }
        }
        
        // 文件结束
        Token token;
        token.type = END;
        token.value = "EOF";
        token.line = line;
        return token;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "用法: " << argv[0] << " <源文件>" << endl;
        return 1;
    }
    
    // 读取源文件
    ifstream file(argv[1]);
    if (!file.is_open()) {
        cerr << "无法打开文件: " << argv[1] << endl;
        return 1;
    }
    
    string source((istreambuf_iterator<char>(file)),
                   istreambuf_iterator<char>());
    file.close();
    
    // 词法分析
    cout << "=== C--语言词法分析结果（手动实现）===" << endl;
    cout << "格式: (种别码, 值)" << endl << endl;
    
    Lexer lexer(source);
    Token token;
    
    do {
        token = lexer.getNextToken();
        if (token.type != END && token.type != ERROR) {
            cout << "(" << token.type << ", " << token.value << ")" << endl;
        }
    } while (token.type != END);
    
    return 0;
}
