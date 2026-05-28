/*
 * LL(1)语法分析器
 * 功能：自动计算First集、Follow集，生成LL(1)分析表，并进行语法分析
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <iomanip>
#include <cstdlib>
using namespace std;

// 产生式结构
struct Production {
    string left;              // 左部非终结符
    vector<string> right;     // 右部符号串
};

class LL1Parser {
private:
    vector<Production> productions;           // 产生式集合
    set<string> terminals;                    // 终结符集合
    set<string> nonTerminals;                 // 非终结符集合
    string startSymbol;                       // 开始符号
    map<string, set<string>> firstSets;       // First集
    map<string, set<string>> followSets;      // Follow集
    map<pair<string, string>, int> parseTable; // LL(1)分析表
    
    const string EPSILON = "ε";               // 空串符号
    const string END_MARKER = "$";            // 结束符号
    
    // 判断是否为终结符
    bool isTerminal(const string& symbol) {
        return terminals.find(symbol) != terminals.end() || symbol == EPSILON;
    }
    
    // 判断是否为非终结符
    bool isNonTerminal(const string& symbol) {
        return nonTerminals.find(symbol) != nonTerminals.end();
    }
    
    // 计算First集
    void computeFirstSets() {
        bool changed = true;
        
        // 初始化：终结符的First集是它自己
        for (const auto& t : terminals) {
            firstSets[t].insert(t);
        }
        
        // 迭代计算非终结符的First集
        while (changed) {
            changed = false;
            
            for (const auto& prod : productions) {
                string A = prod.left;
                const vector<string>& alpha = prod.right;
                
                // 如果产生式为 A -> ε
                if (alpha.size() == 1 && alpha[0] == EPSILON) {
                    if (firstSets[A].find(EPSILON) == firstSets[A].end()) {
                        firstSets[A].insert(EPSILON);
                        changed = true;
                    }
                    continue;
                }
                
                // 计算 First(alpha)
                bool allHaveEpsilon = true;
                for (const auto& symbol : alpha) {
                    // 将 First(symbol) - {ε} 加入 First(A)
                    for (const auto& s : firstSets[symbol]) {
                        if (s != EPSILON && firstSets[A].find(s) == firstSets[A].end()) {
                            firstSets[A].insert(s);
                            changed = true;
                        }
                    }
                    
                    // 如果symbol不能推导出ε，停止
                    if (firstSets[symbol].find(EPSILON) == firstSets[symbol].end()) {
                        allHaveEpsilon = false;
                        break;
                    }
                }
                
                // 如果所有符号都能推导出ε，将ε加入First(A)
                if (allHaveEpsilon && firstSets[A].find(EPSILON) == firstSets[A].end()) {
                    firstSets[A].insert(EPSILON);
                    changed = true;
                }
            }
        }
    }
    
    // 计算Follow集
    void computeFollowSets() {
        // 初始化：将$加入开始符号的Follow集
        followSets[startSymbol].insert(END_MARKER);
        
        bool changed = true;
        while (changed) {
            changed = false;
            
            for (const auto& prod : productions) {
                string A = prod.left;
                const vector<string>& alpha = prod.right;
                
                for (size_t i = 0; i < alpha.size(); i++) {
                    string B = alpha[i];
                    
                    // 只处理非终结符
                    if (!isNonTerminal(B)) continue;
                    
                    // 计算 First(beta)，其中 beta 是 B 后面的符号串
                    bool allHaveEpsilon = true;
                    for (size_t j = i + 1; j < alpha.size(); j++) {
                        string symbol = alpha[j];
                        
                        // 将 First(symbol) - {ε} 加入 Follow(B)
                        for (const auto& s : firstSets[symbol]) {
                            if (s != EPSILON && followSets[B].find(s) == followSets[B].end()) {
                                followSets[B].insert(s);
                                changed = true;
                            }
                        }
                        
                        // 如果symbol不能推导出ε，停止
                        if (firstSets[symbol].find(EPSILON) == firstSets[symbol].end()) {
                            allHaveEpsilon = false;
                            break;
                        }
                    }
                    
                    // 如果 beta 能推导出 ε（或B是最后一个符号），将Follow(A)加入Follow(B)
                    if (allHaveEpsilon) {
                        for (const auto& s : followSets[A]) {
                            if (followSets[B].find(s) == followSets[B].end()) {
                                followSets[B].insert(s);
                                changed = true;
                            }
                        }
                    }
                }
            }
        }
    }
    
    // 构造LL(1)分析表
    bool constructParseTable() {
        bool hasConflict = false;
        
        for (size_t i = 0; i < productions.size(); i++) {
            const Production& prod = productions[i];
            string A = prod.left;
            const vector<string>& alpha = prod.right;
            
            // 计算 First(alpha)
            set<string> firstAlpha;
            bool hasEpsilon = true;
            
            for (const auto& symbol : alpha) {
                for (const auto& s : firstSets[symbol]) {
                    if (s != EPSILON) {
                        firstAlpha.insert(s);
                    }
                }
                
                if (firstSets[symbol].find(EPSILON) == firstSets[symbol].end()) {
                    hasEpsilon = false;
                    break;
                }
            }
            
            if (alpha.size() == 1 && alpha[0] == EPSILON) {
                hasEpsilon = true;
            }
            
            // 对于 First(alpha) 中的每个终结符 a，将产生式加入 M[A, a]
            for (const auto& a : firstAlpha) {
                auto key = make_pair(A, a);
                if (parseTable.find(key) != parseTable.end()) {
                    cout << "冲突：M[" << A << ", " << a << "] 已有产生式" << endl;
                    hasConflict = true;
                }
                parseTable[key] = i;
            }
            
            // 如果 ε ∈ First(alpha)，对于 Follow(A) 中的每个符号 b，将产生式加入 M[A, b]
            if (hasEpsilon) {
                for (const auto& b : followSets[A]) {
                    auto key = make_pair(A, b);
                    if (parseTable.find(key) != parseTable.end()) {
                        cout << "冲突：M[" << A << ", " << b << "] 已有产生式" << endl;
                        hasConflict = true;
                    }
                    parseTable[key] = i;
                }
            }
        }
        
        return !hasConflict;
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
            
            // 解析产生式：A -> alpha | beta
            size_t arrowPos = line.find("->");
            if (arrowPos == string::npos) continue;
            
            string left = line.substr(0, arrowPos);
            string right = line.substr(arrowPos + 2);
            
            // 去除空格
            left.erase(0, left.find_first_not_of(" \t"));
            left.erase(left.find_last_not_of(" \t") + 1);
            right.erase(0, right.find_first_not_of(" \t"));
            
            // 第一个产生式的左部是开始符号
            if (firstProduction) {
                startSymbol = left;
                firstProduction = false;
            }
            
            nonTerminals.insert(left);
            
            // 处理多个候选式（用 | 分隔）
            stringstream ss(right);
            string alternative;
            
            while (getline(ss, alternative, '|')) {
                Production prod;
                prod.left = left;
                
                // 解析右部符号
                alternative.erase(0, alternative.find_first_not_of(" \t"));
                alternative.erase(alternative.find_last_not_of(" \t") + 1);
                
                stringstream ss2(alternative);
                string symbol;
                while (ss2 >> symbol) {
                    prod.right.push_back(symbol);
                    
                    // 识别终结符和非终结符
                    if (symbol != EPSILON && !isupper(symbol[0])) {
                        terminals.insert(symbol);
                    }
                }
                
                productions.push_back(prod);
            }
        }
        
        file.close();
        terminals.insert(END_MARKER);
        return true;
    }
    
    // 分析文法
    bool analyze() {
        cout << "\n=== 文法分析 ===" << endl;
        cout << "开始符号: " << startSymbol << endl;
        
        cout << "\n产生式:" << endl;
        for (size_t i = 0; i < productions.size(); i++) {
            cout << i << ": " << productions[i].left << " -> ";
            for (const auto& s : productions[i].right) {
                cout << s << " ";
            }
            cout << endl;
        }
        
        // 计算First集
        computeFirstSets();
        cout << "\n=== First集 ===" << endl;
        for (const auto& nt : nonTerminals) {
            cout << "First(" << nt << ") = { ";
            for (const auto& s : firstSets[nt]) {
                cout << s << " ";
            }
            cout << "}" << endl;
        }
        
        // 计算Follow集
        computeFollowSets();
        cout << "\n=== Follow集 ===" << endl;
        for (const auto& nt : nonTerminals) {
            cout << "Follow(" << nt << ") = { ";
            for (const auto& s : followSets[nt]) {
                cout << s << " ";
            }
            cout << "}" << endl;
        }
        
        // 构造分析表
        bool success = constructParseTable();
        if (!success) {
            cout << "\n该文法不是LL(1)文法！" << endl;
            return false;
        }
        
        cout << "\n=== LL(1)分析表 ===" << endl;
        printParseTable();
        
        return true;
    }
    
    // 打印分析表
    void printParseTable() {
        // 打印表头
        cout << setw(10) << " ";
        for (const auto& t : terminals) {
            cout << setw(15) << t;
        }
        cout << endl;
        
        cout << string(10 + terminals.size() * 15, '-') << endl;
        
        // 打印每一行
        for (const auto& nt : nonTerminals) {
            cout << setw(10) << nt;
            for (const auto& t : terminals) {
                auto key = make_pair(nt, t);
                if (parseTable.find(key) != parseTable.end()) {
                    int prodIndex = parseTable[key];
                    const Production& prod = productions[prodIndex];
                    
                    stringstream ss;
                    ss << nt << "->";
                    for (const auto& s : prod.right) {
                        ss << s;
                    }
                    cout << setw(15) << ss.str();
                } else {
                    cout << setw(15) << " ";
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
        vector<string> stack;
        stack.push_back(END_MARKER);
        stack.push_back(startSymbol);
        
        vector<string> inputSymbols;
        stringstream ss(input);
        string symbol;
        while (ss >> symbol) {
            inputSymbols.push_back(symbol);
        }
        inputSymbols.push_back(END_MARKER);
        
        size_t inputIndex = 0;
        
        // 打印表头
        cout << setw(5) << "步骤" << setw(20) << "栈" << setw(20) << "输入" << setw(30) << "动作" << endl;
        cout << string(75, '-') << endl;
        
        int step = 0;
        while (!stack.empty()) {
            // 打印当前状态
            cout << setw(5) << step++;
            
            // 打印栈
            stringstream stackStr;
            for (const auto& s : stack) {
                stackStr << s << " ";
            }
            cout << setw(20) << stackStr.str();
            
            // 打印输入
            stringstream inputStr;
            for (size_t i = inputIndex; i < inputSymbols.size(); i++) {
                inputStr << inputSymbols[i] << " ";
            }
            cout << setw(20) << inputStr.str();
            
            string top = stack.back();
            stack.pop_back();
            string current = inputSymbols[inputIndex];
            
            // 栈顶是结束符（必须在终结符判断之前）
            if (top == END_MARKER) {
                if (current == END_MARKER) {
                    cout << setw(30) << "接受" << endl;
                    return true;
                } else {
                    cout << setw(30) << "错误：输入未结束" << endl;
                    return false;
                }
            }
            // 栈顶是终结符
            else if (isTerminal(top)) {
                if (top == current) {
                    cout << setw(30) << ("匹配 " + top) << endl;
                    inputIndex++;
                } else {
                    cout << setw(30) << "错误：不匹配" << endl;
                    return false;
                }
            }
            // 栈顶是非终结符
            else {
                auto key = make_pair(top, current);
                if (parseTable.find(key) == parseTable.end()) {
                    cout << setw(30) << "错误：无匹配产生式" << endl;
                    return false;
                }
                
                int prodIndex = parseTable[key];
                const Production& prod = productions[prodIndex];
                
                stringstream action;
                action << top << " -> ";
                for (const auto& s : prod.right) {
                    action << s << " ";
                }
                cout << setw(30) << action.str() << endl;
                
                // 将产生式右部逆序压栈（跳过ε）
                if (!(prod.right.size() == 1 && prod.right[0] == EPSILON)) {
                    for (int i = prod.right.size() - 1; i >= 0; i--) {
                        stack.push_back(prod.right[i]);
                    }
                }
            }
        }
        
        return false;
    }
};

int main(int argc, char* argv[]) {
    // 设置Windows控制台为UTF-8编码，避免中文乱码
    #ifdef _WIN32
    system("chcp 65001 >nul");
    #endif

    if (argc < 2) {
        cerr << "用法: " << argv[0] << " <文法文件> [输入串]" << endl;
        return 1;
    }
    
    LL1Parser parser;
    
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
