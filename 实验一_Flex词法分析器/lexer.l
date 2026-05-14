%{
/*
 * C--语言词法分析器 - Flex实现
 * 功能：识别C--语言的所有单词符号，输出二元组(种别码, 值)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int line_num = 1;  // 行号计数器

// 种别码定义
#define WHILE 1
#define IF 2
#define ELSE 3
#define SWITCH 4
#define CASE 5
#define ID 6
#define NUM 7
#define PLUS 8
#define QUESTION 9
#define MULTIPLY 10
#define LE 10      // <=
#define LT 11      // <
#define EQ 12      // ==
#define ASSIGN 13  // =
#define COLON 13
#define MINUS 14   // -
#define DIVIDE 15  // /
#define GT 16      // >
#define GE 17      // >=
#define LPAREN 18  // (
#define RPAREN 19  // )
#define LBRACE 20  // {
#define RBRACE 21  // }
#define SEMICOLON 22  // ;
#define COMMA 23   // ,

%}

/* 正则表达式定义 */
DIGIT       [0-9]
LETTER      [a-zA-Z_]
ID          {LETTER}({LETTER}|{DIGIT})*
INTEGER     {DIGIT}+
FLOAT       {DIGIT}+\.{DIGIT}+
SCIENTIFIC  ({INTEGER}|{FLOAT})[eE][+-]?{DIGIT}+
WHITESPACE  [ \t]+

%%

    /* 关键字 优先级最高 */
"while"     { printf("(%d, while)\n", WHILE); }
"if"        { printf("(%d, if)\n", IF); }
"else"      { printf("(%d, else)\n", ELSE); }
"switch"    { printf("(%d, switch)\n", SWITCH); }
"case"      { printf("(%d, case)\n", CASE); }

    /* 标识符 */
{ID}        { printf("(%d, %s)\n", ID, yytext); }

    /* 数字常量 */
{SCIENTIFIC} { printf("(%d, %s)\n", NUM, yytext); }
{FLOAT}     { printf("(%d, %s)\n", NUM, yytext); }
{INTEGER}   { printf("(%d, %s)\n", NUM, yytext); }

    /* 双字符运算符 - 必须在单字符之前匹配 */
"<="        { printf("(%d, LE)\n", LE); }
">="        { printf("(%d, GE)\n", GE); }
"=="        { printf("(%d, EQ)\n", EQ); }

    /* 单字符运算符 */
"+"         { printf("(%d, +)\n", PLUS); }
"-"         { printf("(%d, -)\n", MINUS); }
"*"         { printf("(%d, *)\n", MULTIPLY); }
"/"         { printf("(%d, /)\n", DIVIDE); }
"<"         { printf("(%d, LT)\n", LT); }
">"         { printf("(%d, GT)\n", GT); }
"="         { printf("(%d, =)\n", ASSIGN); }
"?"         { printf("(%d, ?)\n", QUESTION); }
":"         { printf("(%d, :)\n", COLON); }

    /* 特殊符号 */
"("         { printf("(%d, ()\n", LPAREN); }
")"         { printf("(%d, ))\n", RPAREN); }
"{"         { printf("(%d, {)\n", LBRACE); }
"}"         { printf("(%d, })\n", RBRACE); }
";"         { printf("(%d, ;)\n", SEMICOLON); }
","         { printf("(%d, ,)\n", COMMA); }

    /* 空白符和换行 */
{WHITESPACE} { /* 忽略空白 */ }
"\n"        { line_num++; }
"\r\n"      { line_num++; }

    /* 注释处理 */
"//".*      { /* 单行注释 */ }
"/*"        { 
                int c;
                while ((c = input()) != 0) {
                    if (c == '\n') line_num++;
                    if (c == '*') {
                        if ((c = input()) == '/') break;
                        else unput(c);
                    }
                }
            }

    /* 错误处理 */
.           { printf("词法错误：第%d行出现非法字符 '%s'\n", line_num, yytext); }

%%

int yywrap() {
    return 1;
}

int main(int argc, char **argv) {
    if (argc > 1) {
        FILE *file = fopen(argv[1], "r");
        if (!file) {
            fprintf(stderr, "无法打开文件: %s\n", argv[1]);
            return 1;
        }
        yyin = file;
    }
    
    printf("=== C--语言词法分析结果 ===\n");
    printf("格式: (种别码, 值)\n\n");
    
    yylex();
    
    if (argc > 1) {
        fclose(yyin);
    }
    
    return 0;
}
