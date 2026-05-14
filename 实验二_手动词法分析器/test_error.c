/* 
 * 测试程序2：含有词法错误的C--程序
 * 错误类型：非法字符、错误的数字格式等
 */

int main() {
    int x;
    int y;
    
    x = 100;
    y = 200;
    
    // 错误1：非法字符 @
    int @value = 50;
    
    // 错误2：非法字符 #
    if (x # y) {
        x = x + 1;
    }
    
    // 错误3：非法字符 $
    int $money = 1000;
    
    // 错误4：非法字符 &
    while (x & y) {
        x = x - 1;
    }
    
    // 错误5：非法字符 %
    int result = x % y;
    
    // 错误6：非法字符 ^
    if (x ^ y) {
        y = y / 2;
    }
    
    // 错误7：非法字符 ~
    int neg = ~x;
    
    // 错误8：非法字符 `（反引号）
    int temp = 10;
    int value = temp ` 5;
    
    // 正常的语句
    if (x > y) {
        x = x - y;
    } else {
        y = y - x;
    }
    
    // 错误9：非法字符 |
    int flag = x | y;
    
    return 0;
}
