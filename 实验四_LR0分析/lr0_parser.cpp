/*
 * LR(0)语法分析器
 * 功能：自动构造LR(0)项目集族和分析表，并进行语法分析
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <iomanip>
#include <algorithm>
using namespace std;

// 产生式结构
struct Production {
    char left;              // 左部非终结符
    string right;           // 右部符号串
    
    bool operator<(const Production& other) const {
        if (left != other.left) return left < other.left;
        return right < other.right;
    }
};

// LR(0)项目
struct Item {
    int prodIndex;          // 产生式编号
    int dotPos;             // 点的位置
    
    bool operator<(const Item& other) const {
        if (prodIndex != other.prodIndex) return prodIndex < other.prodIndex;
        return dotPos < other.dotPos;
    }
    
    bool operator==(const Item& other) const {
        return prodIndex == other.prodIndex && dotPos == other.dotPos;
    }
};

// 动作类型
enum ActionType { SHIFT, REDUCE, ACCEPT, ERROR };

// 动作结构
struct Action {
    ActionType type;
    int number;  // 状态号或产生式号
};

class LR0Parser {
private:
    vector<Production> productions;              // 产生式集合
    set<char> terminals;                         // 终结符集合
    set<char> nonTerminals;                      // 非终结符集合
    char startSymbol;                            // 开始符号
    char augmentedStart;                         // 增广开始符号
    
    vector<set<Item>> itemSets;                  // 项目集族
    map<pair<int, char>, int> gotoTable;         // GOTO表
    map<pair<int, char>, Action> actionTable;    // ACTION表
    
    const char END_MARKER = '$';                 // 结束符号
    const char EPSILON = 'e';                    // 空串符号（用e表示ε）
    
    // 判断是否为终结符
    bool isTerminal(char symbol) {
        return terminals.find(symbol) != terminals.end();
    }
    
    // 判断是否为非终结符
    bool isNonTerminal(char symbol) {
        return nonTerminals.find(symbol) != nonTerminals.end();
    }
    
    // 获取项目的点后符号
    char getSymbolAfterDot(const Item& item) {
        const Production& prod = productions[item.prodIndex];
        if (item.dotPos < (int)prod.right.length()) {
            return prod.right[item.dotPos];
        }
        return '\0';  // 点在最后
    }
    
    // 计算项目集的闭包
    set<Item> closure(const set<Item>& items) {
        set<Item> result = items;
        bool changed = true;
        
        while (changed) {
            changed = false;
            set<Item> newItems;
            
            for (const Item& item : result) {
                char symbol = getSymbolAfterDot(item);
                
                // 如果点后是非终结符，添加该非终结符的所有产生式
                if (isNonTerminal(symbol)) {
                    for (int i = 0; i < (int)productions.size(); i++) {
                        if (productions[i].left == symbol) {
                            Item newItem = {i, 0};
                            if (result.find(newItem) == result.end() && 
                                newItems.find(newItem) == newItems.end()) {
                                newItems.insert(newItem);
                                changed = true;
                            }
                        }
                    }
                }
            }
            
            result.insert(newItems.begin(), newItems.end());
        }
        
        return result;
    }
    
    // 计算GOTO(I, X)
    set<Item> gotoSet(const set<Item>& items, char symbol) {
        set<Item> result;
        
        for (const Item& item : items) {
            char nextSymbol = getSymbolAfterDot(item);
            if (nextSymbol == symbol) {
                Item newItem = {item.prodIndex, item.dotPos + 1};
                result.insert(newItem);
            }
        }
        
        return closure(result);
    }
    
    // 构造LR(0)项目集族
    void constructItemSets() {
        // 创建初始项目集 I0
        set<Item> I0;
        I0.insert({0, 0});  // S' -> .S
        I0 = closure(I0);
        itemSets.push_back(I0);
        
        // 使用队列处理所有项目集
        vector<int> queue;
        queue.push_back(0);
        size_t queueIndex = 0;
        
        while (queueIndex < queue.size()) {
            int currentIndex = queue[queueIndex++];
            const set<Item>& currentSet = itemSets[currentIndex];
            
            // 收集所有可能的符号
            set<char> symbols;
            for (const Item& item : currentSet) {
                char symbol = getSymbolAfterDot(item);
                if (symbol != '\0') {
                    symbols.insert(symbol);
                }
            }
            
            // 对每个符号计算GOTO
            for (char symbol : symbols) {
                set<Item> newSet = gotoSet(currentSet, symbol);
                
                if (newSet.empty()) continue;
                
                // 查找是否已存在相同的项目集
                int targetIndex = -1;
                for (int i = 0; i < (int)itemSets.size(); i++) {
                    if (itemSets[i] == newSet) {
                        targetIndex = i;
                        break;
                    }
                }
                
                // 如果不存在，添加新项目集
                if (targetIndex == -1) {
                    targetIndex = itemSets.size();
                    itemSets.push_back(newSet);
                    queue.push_back(targetIndex);
                }
                
                // 记录GOTO转换
                gotoTable[{currentIndex, symbol}] = targetIndex;
            }
        }
    }
    
    // 构造LR(0)分析表
    bool constructParseTable() {
        bool hasConflict = false;
        
        for (int i = 0; i < (int)itemSets.size(); i++) {
            const set<Item>& itemSet = itemSets[i];
            
            for (const Item& item : itemSet) {
                const Production& prod = productions[item.prodIndex];
                char symbol = getSymbolAfterDot(item);
                
                // 移进项目：[A -> α.aβ]
                if (symbol != '\0' && isTerminal(symbol)) {
                    auto key = make_pair(i, symbol);
                    if (gotoTable.find({i, symbol}) != gotoTable.end()) {
                        int nextState = gotoTable[{i, symbol}];
                        
                        if (actionTable.find(key) != actionTable.end()) {
                            cout << "移进-规约冲突：状态" << i << "，符号" << symbol << endl;
                            hasConflict = true;
                        } else {
                            actionTable[key] = {SHIFT, nextState};
                        }
                    }
                }
                // 规约项目：[A -> α.]
                else if (symbol == '\0') {
                    // 接受项目：[S' -> S.]
                    if (item.prodIndex == 0) {
                        actionTable[{i, END_MARKER}] = {ACCEPT, 0};
                    }
                    // 普通规约项目
                    else {
                        for (char t : terminals) {
                            auto key = make_pair(i, t);
                            if (actionTable.find(key) != actionTable.end()) {
                                cout << "规约-规约冲突：状态" << i << "，符号" << t << endl;
                                hasConflict = true;
                            } else {
                                actionTable[key] = {REDUCE, item.prodIndex};
                            }
                        }
                    }
                }
            }
        }
        
        return !hasConflict;
    }
    
    // 打印项目
    void printItem(const Item& item) {
        const Production& prod = productions[item.prodIndex];
        cout << prod.left << " -> ";
        for (int i = 0; i < (int)prod.right.length(); i++) {
            if (i == item.dotPos) cout << "·";
            cout << prod.right[i];
        }
        if (item.dotPos == (int)prod.right.length()) cout << "·";
    }
    
public:
    // 读取文法
    bool loadGrammar(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "无法打开文件: " << filename << endl;
            return false;
        }
        
        string line;
        bool firstProduction = true;
        
        while (getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;
            
            // 解析产生式：A -> alpha
            size_t arrowPos = line.find("->");
            if (arrowPos == string::npos) continue;
            
            string leftStr = line.substr(0, arrowPos);
            string rightStr = line.substr(arrowPos + 2);
            
            // 去除空格
            leftStr.erase(remove(leftStr.begin(), leftStr.end(), ' '), leftStr.end());
            rightStr.erase(remove(rightStr.begin(), rightStr.end(), ' '), rightStr.end());
            
            if (leftStr.empty() || rightStr.empty()) continue;
            
            char left = leftStr[0];
            
            // 第一个产生式的左部是开始符号
            if (firstProduction) {
                startSymbol = left;
                firstProduction = false;
            }
            
            nonTerminals.insert(left);
            
            Production prod;
            prod.left = left;
            prod.right = rightStr;
            
            // 识别终结符
            for (char c : rightStr) {
                if (!isupper(c) && c != EPSILON) {
                    terminals.insert(c);
                }
            }
            
            productions.push_back(prod);
        }
        
        file.close();
        
        // 添加增广产生式 S' -> S
        augmentedStart = startSymbol + '\'';
        Production augProd;
        augProd.left = augmentedStart;
        augProd.right = string(1, startSymbol);
        productions.insert(productions.begin(), augProd);
        nonTerminals.insert(augmentedStart);
        
        terminals.insert(END_MARKER);
        
        return true;
    }
    
    // 分析文法
    bool analyze() {
        cout << "\n=== 文法分析 ===" << endl;
        cout << "开始符号: " << startSymbol << endl;
        cout << "增广开始符号: " << augmentedStart << endl;
        
        cout << "\n产生式:" << endl;
        for (int i = 0; i < (int)productions.size(); i++) {
            cout << i << ": " << productions[i].left << " -> " 
                 << productions[i].right << endl;
        }
        
        // 构造项目集族
        constructItemSets();
        cout << "\n=== LR(0)项目集族 ===" << endl;
        for (int i = 0; i < (int)itemSets.size(); i++) {
            cout << "\nI" << i << ":" << endl;
            for (const Item& item : itemSets[i]) {
                cout << "  ";
                printItem(item);
                cout << endl;
            }
        }
        
        // 构造分析表
        bool success = constructParseTable();
        if (!success) {
            cout << "\n该文法不是LR(0)文法！存在冲突。" << endl;
            return false;
        }
        
        cout << "\n=== LR(0)分析表 ===" << endl;
        printParseTable();
        
        return true;
    }
    
    // 打印分析表
    void printParseTable() {
        cout << "\nACTION表:" << endl;
        cout << setw(8) << "状态";
        for (char t : terminals) {
            cout << setw(10) << t;
        }
        cout << endl;
        cout << string(8 + terminals.size() * 10, '-') << endl;
        
        for (int i = 0; i < (int)itemSets.size(); i++) {
            cout << setw(8) << i;
            for (char t : terminals) {
                auto key = make_pair(i, t);
                if (actionTable.find(key) != actionTable.end()) {
                    Action action = actionTable[key];
                    stringstream ss;
                    switch (action.type) {
                        case SHIFT: ss << "s" << action.number; break;
                        case REDUCE: ss << "r" << action.number; break;
                        case ACCEPT: ss << "acc"; break;
                        default: break;
                    }
                    cout << setw(10) << ss.str();
                } else {
                    cout << setw(10) << " ";
                }
            }
            cout << endl;
        }
        
        cout << "\nGOTO表:" << endl;
        cout << setw(8) << "状态";
        for (char nt : nonTerminals) {
            if (nt != augmentedStart) {
                cout << setw(10) << nt;
            }
        }
        cout << endl;
        cout << string(8 + (nonTerminals.size() - 1) * 10, '-') << endl;
        
        for (int i = 0; i < (int)itemSets.size(); i++) {
            cout << setw(8) << i;
            for (char nt : nonTerminals) {
                if (nt != augmentedStart) {
                    auto key = make_pair(i, nt);
                    if (gotoTable.find(key) != gotoTable.end()) {
                        cout << setw(10) << gotoTable[key];
                    } else {
                        cout << setw(10) << " ";
                    }
                }
            }
            cout << endl;
        }
    }
    
    // 语法分析
    bool parse(const string& input) {
        cout << "\n=== 语法分析过程 ===" << endl;
        cout << "输入串: " << input << endl << endl;
        
        // 初始化栈和输入
        vector<int> stateStack;
        vector<char> symbolStack;
        stateStack.push_back(0);
        
        string inputStr = input + END_MARKER;
        size_t inputIndex = 0;
        
        // 打印表头
        cout << setw(5) << "步骤" << setw(20) << "状态栈" << setw(20) << "符号栈" 
             << setw(15) << "输入" << setw(25) << "动作" << endl;
        cout << string(85, '-') << endl;
        
        int step = 0;
        while (true) {
            // 打印当前状态
            cout << setw(5) << step++;
            
            // 打印状态栈
            stringstream stateStr;
            for (int s : stateStack) {
                stateStr << s << " ";
            }
            cout << setw(20) << stateStr.str();
            
            // 打印符号栈
            stringstream symbolStr;
            for (char c : symbolStack) {
                symbolStr << c << " ";
            }
            cout << setw(20) << symbolStr.str();
            
            // 打印输入
            cout << setw(15) << inputStr.substr(inputIndex);
            
            int currentState = stateStack.back();
            char currentInput = inputStr[inputIndex];
            
            auto key = make_pair(currentState, currentInput);
            if (actionTable.find(key) == actionTable.end()) {
                cout << setw(25) << "错误：无对应动作" << endl;
                return false;
            }
            
            Action action = actionTable[key];
            
            // 移进
            if (action.type == SHIFT) {
                cout << setw(25) << ("移进到状态" + to_string(action.number)) << endl;
                stateStack.push_back(action.number);
                symbolStack.push_back(currentInput);
                inputIndex++;
            }
            // 规约
            else if (action.type == REDUCE) {
                const Production& prod = productions[action.number];
                stringstream ss;
                ss << "用" << prod.left << "->" << prod.right << "规约";
                cout << setw(25) << ss.str() << endl;
                
                // 弹出产生式右部长度的状态和符号
                int popCount = prod.right.length();
                for (int i = 0; i < popCount; i++) {
                    stateStack.pop_back();
                    symbolStack.pop_back();
                }
                
                // 查GOTO表
                int gotoState = stateStack.back();
                auto gotoKey = make_pair(gotoState, prod.left);
                if (gotoTable.find(gotoKey) == gotoTable.end()) {
                    cout << "错误：GOTO表中无对应项" << endl;
                    return false;
                }
                
                stateStack.push_back(gotoTable[gotoKey]);
                symbolStack.push_back(prod.left);
            }
            // 接受
            else if (action.type == ACCEPT) {
                cout << setw(25) << "接受" << endl;
                return true;
            }
        }
        
        return false;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "用法: " << argv[0] << " <文法文件> [输入串]" << endl;
        return 1;
    }
    
    LR0Parser parser;
    
    // 读取文法
    if (!parser.loadGrammar(argv[1])) {
        return 1;
    }
    
    // 分析文法
    if (!parser.analyze()) {
        return 1;
    }
    
    // 如果提供了输入串，进行语法分析
    if (argc >= 3) {
        string input = argv[2];
        parser.parse(input);
    }
    
    return 0;
}
