/* 
 * 测试程序1：正确的C--程序
 * 功能：计算两个数的最大值和算术运算
 */

// 主函数
int main() {
    int a;
    int b;
    int max;
    int sum;
    int diff;
    int product;
    int quotient;
    
    a = 10;
    b = 20;
    
    // 比较运算测试
    if (a > b) {
        max = a;
    } else {
        max = b;
    }
    
    // 算术运算测试
    sum = a + b;        // 加法
    diff = b - a;       // 减法
    product = a * b;    // 乘法
    quotient = b / a;   // 除法
    
    // 关系运算测试
    while (a < b) {
        a = a + 1;
    }
    
    if (a >= b) {
        sum = sum + 1;
    }
    
    if (a <= b) {
        diff = diff - 1;
    }
    
    if (a == b) {
        product = product * 2;
    }
    
    // switch语句测试
    switch (max) {
        case 10:
            sum = sum + 10;
        case 20:
            sum = sum + 20;
    }
    
    // 浮点数和科学计数法测试
    float pi;
    float e;
    pi = 3.14159;
    e = 2.71828e10;
    
    return 0;
}
