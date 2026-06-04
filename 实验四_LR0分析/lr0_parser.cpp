/*
 * LR(0)语法分析器
 * 实验四：实现LR(0)分析算法，自动构造项目集族和分析表，并给出语句的分析过程
 * 文法：E->aA|bB, A->cA|d, B->cB|d
 * 测试串：acccd
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
#include <cstdlib>
using namespace std;

// 产生式：left -> right[0] right[1] ... right[n-1]
struct Production {
    string left;           // 左部非终结符（用string，支持E'等符号）
    vector<string> right;  // 右部符号序列
};

// LR(0)项目：[left -> alpha . beta]
struct Item {
    int prodIndex;  // 产生式编号
    int dotPos;     // 点的位置（0 = 点在最前）

    bool operator<(const Item& o) const {
        if (prodIndex != o.prodIndex) return prodIndex < o.prodIndex;
        return dotPos < o.dotPos;
    }
    bool operator==(const Item& o) const {
        return prodIndex == o.prodIndex && dotPos == o.dotPos;
    }
};

// ACTION表条目类型
enum ActionType { SHIFT, REDUCE, ACCEPT };

struct Action {
    ActionType type;
    int number;  // SHIFT->目标状态号，REDUCE->产生式号
};

class LR0Parser {
private:
    vector<Production> productions;              // 所有产生式（0号为增广产生式）
    set<string> terminals;                       // 终结符集合
    set<string> nonTerminals;                    // 非终结符集合
    string startSymbol;                          // 原始开始符号
    string augStart;                             // 增广开始符号（如E'）

    vector<set<Item>> itemSets;                  // 项目集族
    map<pair<int,string>, int> gotoTable;        // GOTO表：(状态,符号)->状态
    map<pair<int,string>, Action> actionTable;   // ACTION表：(状态,终结符)->动作

    const string END_MARKER = "$";

    // 获取点后的符号，若点在末尾返回""
    string symbolAfterDot(const Item& item) const {
        const Production& p = productions[item.prodIndex];
        if (item.dotPos < (int)p.right.size())
            return p.right[item.dotPos];
        return "";
    }

    // 判断是否为非终结符
    bool isNonTerminal(const string& s) const {
        return nonTerminals.count(s) > 0;
    }

    // 计算项目集的CLOSURE
    set<Item> closure(set<Item> items) const {
        bool changed = true;
        while (changed) {
            changed = false;
            set<Item> toAdd;
            for (const Item& item : items) {
                string sym = symbolAfterDot(item);
                if (sym.empty() || !isNonTerminal(sym)) continue;
                // 对sym的每条产生式，加入初始项目
                for (int i = 0; i < (int)productions.size(); i++) {
                    if (productions[i].left == sym) {
                        Item ni{i, 0};
                        if (!items.count(ni) && !toAdd.count(ni)) {
                            toAdd.insert(ni);
                            changed = true;
                        }
                    }
                }
            }
            items.insert(toAdd.begin(), toAdd.end());
        }
        return items;
    }

    // 计算GOTO(I, X)
    set<Item> gotoSet(const set<Item>& I, const string& X) const {
        set<Item> moved;
        for (const Item& item : I) {
            if (symbolAfterDot(item) == X)
                moved.insert({item.prodIndex, item.dotPos + 1});
        }
        return moved.empty() ? moved : closure(moved);
    }

    // 构造LR(0)项目集族
    void buildItemSets() {
        // I0 = CLOSURE({E' -> .E})
        set<Item> I0 = closure({{0, 0}});
        itemSets.push_back(I0);

        for (int i = 0; i < (int)itemSets.size(); i++) {
            // 收集当前项目集中点后所有可能的符号
            set<string> symbols;
            for (const Item& item : itemSets[i]) {
                string s = symbolAfterDot(item);
                if (!s.empty()) symbols.insert(s);
            }

            for (const string& sym : symbols) {
                set<Item> newSet = gotoSet(itemSets[i], sym);
                if (newSet.empty()) continue;

                // 查找是否已存在
                int target = -1;
                for (int j = 0; j < (int)itemSets.size(); j++) {
                    if (itemSets[j] == newSet) { target = j; break; }
                }
                if (target == -1) {
                    target = (int)itemSets.size();
                    itemSets.push_back(newSet);
                }
                gotoTable[{i, sym}] = target;
            }
        }
    }

    // 构造LR(0)分析表（ACTION + GOTO）
    bool buildParseTable() {
        bool conflict = false;
        for (int i = 0; i < (int)itemSets.size(); i++) {
            for (const Item& item : itemSets[i]) {
                string sym = symbolAfterDot(item);

                if (!sym.empty()) {
                    // 移进项目 [A -> α.aβ]，a为终结符
                    if (terminals.count(sym)) {
                        auto key = make_pair(i, sym);
                        int next = gotoTable[{i, sym}];
                        if (actionTable.count(key)) {
                            cerr << "冲突：ACTION[" << i << "," << sym << "]" << endl;
                            conflict = true;
                        }
                        actionTable[key] = {SHIFT, next};
                    }
                    // GOTO表已在buildItemSets中构造，非终结符的转移不写ACTION
                } else {
                    // 规约或接受项目 [A -> α.]
                    if (item.prodIndex == 0) {
                        // 增广产生式规约 = 接受
                        auto key = make_pair(i, END_MARKER);
                        if (actionTable.count(key)) {
                            cerr << "冲突：ACTION[" << i << ",$]" << endl;
                            conflict = true;
                        }
                        actionTable[key] = {ACCEPT, 0};
                    } else {
                        // 对所有终结符（含$）填入规约
                        set<string> allTerms = terminals;
                        allTerms.insert(END_MARKER);
                        for (const string& t : allTerms) {
                            auto key = make_pair(i, t);
                            if (actionTable.count(key)) {
                                cerr << "冲突：ACTION[" << i << "," << t << "]" << endl;
                                conflict = true;
                            }
                            actionTable[key] = {REDUCE, item.prodIndex};
                        }
                    }
                }
            }
        }
        return !conflict;
    }

    // 打印一个项目
    void printItem(const Item& item) const {
        const Production& p = productions[item.prodIndex];
        cout << p.left << " -> ";
        for (int i = 0; i <= (int)p.right.size(); i++) {
            if (i == item.dotPos) cout << "·";
            if (i < (int)p.right.size()) cout << p.right[i];
        }
    }

public:
    // 从文件读取文法
    // 格式：每行 A->BC 或 A->a|b（不含空格，|分隔候选式）
    bool loadGrammar(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "无法打开文件: " << filename << endl;
            return false;
        }

        bool first = true;
        string line;
        while (getline(file, line)) {
            // 去掉行尾\r
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty() || line[0] == '#') continue;

            size_t pos = line.find("->");
            if (pos == string::npos) continue;

            string lhs = line.substr(0, pos);
            // 去除lhs两端空白
            while (!lhs.empty() && (lhs.front()==' '||lhs.front()=='\t')) lhs.erase(lhs.begin());
            while (!lhs.empty() && (lhs.back()==' '||lhs.back()=='\t')) lhs.pop_back();

            if (first) { startSymbol = lhs; first = false; }
            nonTerminals.insert(lhs);

            string rhs = line.substr(pos + 2);
            // 按|分割候选式
            stringstream ss(rhs);
            string alt;
            while (getline(ss, alt, '|')) {
                // 去除首尾空白
                while (!alt.empty() && (alt.front()==' '||alt.front()=='\t')) alt.erase(alt.begin());
                while (!alt.empty() && (alt.back()==' '||alt.back()=='\t')) alt.pop_back();
                if (alt.empty()) continue;

                Production prod;
                prod.left = lhs;
                // 将右部拆分为单字符符号（本实验文法每个符号均为单字符）
                for (char c : alt) {
                    string sym(1, c);
                    prod.right.push_back(sym);
                    if (!isupper(c)) terminals.insert(sym);
                }
                productions.push_back(prod);
            }
        }
        file.close();

        // 添加增广产生式 E' -> E（插入最前面，编号0）
        augStart = startSymbol + "'";
        Production aug;
        aug.left = augStart;
        aug.right = {startSymbol};
        productions.insert(productions.begin(), aug);
        nonTerminals.insert(augStart);

        return true;
    }

    // 分析文法，输出项目集族和分析表
    bool analyze() {
        // 设置Windows控制台UTF-8
        #ifdef _WIN32
        system("chcp 65001 >nul");
        #endif

        cout << "\n=== 文法分析 ===" << endl;
        cout << "开始符号: " << startSymbol << endl;
        cout << "增广开始符号: " << augStart << endl;

        cout << "\n产生式:" << endl;
        for (int i = 0; i < (int)productions.size(); i++) {
            cout << i << ": " << productions[i].left << " -> ";
            for (const string& s : productions[i].right) cout << s;
            cout << endl;
        }

        // 构造项目集族
        buildItemSets();

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
        bool ok = buildParseTable();
        if (!ok) {
            cout << "\n存在冲突，该文法不是严格的LR(0)文法！" << endl;
        }

        cout << "\n=== LR(0)分析表 ===" << endl;
        printTable();

        return ok;
    }

    // 打印ACTION表和GOTO表
    void printTable() {
        // 收集所有终结符（排序后输出）
        vector<string> terms(terminals.begin(), terminals.end());
        sort(terms.begin(), terms.end());
        terms.push_back(END_MARKER);

        // 收集非终结符（去掉增广符号）
        vector<string> nts;
        for (const string& nt : nonTerminals)
            if (nt != augStart) nts.push_back(nt);
        sort(nts.begin(), nts.end());

        int w = 8;  // 列宽

        cout << "\nACTION表:" << endl;
        cout << setw(w) << "状态";
        for (const string& t : terms) cout << setw(w) << t;
        cout << endl;
        cout << string(w + (int)terms.size() * w, '-') << endl;

        for (int i = 0; i < (int)itemSets.size(); i++) {
            cout << setw(w) << i;
            for (const string& t : terms) {
                auto it = actionTable.find({i, t});
                if (it != actionTable.end()) {
                    stringstream ss;
                    switch (it->second.type) {
                        case SHIFT:  ss << "s" << it->second.number; break;
                        case REDUCE: ss << "r" << it->second.number; break;
                        case ACCEPT: ss << "acc"; break;
                    }
                    cout << setw(w) << ss.str();
                } else {
                    cout << setw(w) << " ";
                }
            }
            cout << endl;
        }

        cout << "\nGOTO表:" << endl;
        cout << setw(w) << "状态";
        for (const string& nt : nts) cout << setw(w) << nt;
        cout << endl;
        cout << string(w + (int)nts.size() * w, '-') << endl;

        for (int i = 0; i < (int)itemSets.size(); i++) {
            cout << setw(w) << i;
            for (const string& nt : nts) {
                auto it = gotoTable.find({i, nt});
                if (it != gotoTable.end())
                    cout << setw(w) << it->second;
                else
                    cout << setw(w) << " ";
            }
            cout << endl;
        }
    }

    // LR(0)语法分析过程
    bool parse(const string& input) {
        cout << "\n=== 语法分析过程 ===" << endl;
        cout << "输入串: " << input << endl << endl;

        // 将输入拆成单字符符号序列，末尾加$
        vector<string> inputTokens;
        for (char c : input) inputTokens.push_back(string(1, c));
        inputTokens.push_back(END_MARKER);

        vector<int> stateStack = {0};    // 状态栈
        vector<string> symStack;         // 符号栈
        int idx = 0;

        // 表头
        cout << left
             << setw(5)  << "步骤"
             << setw(20) << "状态栈"
             << setw(20) << "符号栈"
             << setw(15) << "输入"
             << setw(25) << "动作"
             << endl;
        cout << string(85, '-') << endl;

        int step = 0;
        while (true) {
            // 打印当前状态
            stringstream ss_state, ss_sym, ss_input;
            for (int s : stateStack) ss_state << s << " ";
            for (const string& s : symStack) ss_sym << s;
            for (int i = idx; i < (int)inputTokens.size(); i++) ss_input << inputTokens[i];

            cout << left
                 << setw(5)  << step++
                 << setw(20) << ss_state.str()
                 << setw(20) << ss_sym.str()
                 << setw(15) << ss_input.str();

            int curState = stateStack.back();
            string curSym = inputTokens[idx];

            auto it = actionTable.find({curState, curSym});
            if (it == actionTable.end()) {
                cout << setw(25) << "错误：无对应动作" << endl;
                return false;
            }

            Action act = it->second;
            if (act.type == SHIFT) {
                cout << setw(25) << ("移进 " + curSym + " 到状态" + to_string(act.number)) << endl;
                stateStack.push_back(act.number);
                symStack.push_back(curSym);
                idx++;
            } else if (act.type == REDUCE) {
                const Production& prod = productions[act.number];
                stringstream action_str;
                action_str << "规约 " << prod.left << "->";
                for (const string& s : prod.right) action_str << s;
                cout << setw(25) << action_str.str() << endl;

                // 弹出 |right| 个符号和状态
                int len = (int)prod.right.size();
                for (int i = 0; i < len; i++) {
                    stateStack.pop_back();
                    symStack.pop_back();
                }
                // 查GOTO表
                auto gi = gotoTable.find({stateStack.back(), prod.left});
                if (gi == gotoTable.end()) {
                    cerr << "错误：GOTO表无对应项" << endl;
                    return false;
                }
                stateStack.push_back(gi->second);
                symStack.push_back(prod.left);
            } else { // ACCEPT
                cout << setw(25) << "接受" << endl;
                return true;
            }
        }
    }
};

int main(int argc, char* argv[]) {
    #ifdef _WIN32
    system("chcp 65001 >nul");
    #endif

    if (argc < 2) {
        cerr << "用法: " << argv[0] << " <文法文件> [输入串]" << endl;
        cerr << "示例: " << argv[0] << " grammar.txt acccd" << endl;
        return 1;
    }

    LR0Parser parser;

    if (!parser.loadGrammar(argv[1])) return 1;
    if (!parser.analyze()) return 1;

    if (argc >= 3) {
        parser.parse(argv[2]);
    }

    return 0;
}
