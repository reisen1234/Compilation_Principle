#include <iostream>
#include <fstream>
#include<stdio.h>
#include <string>
#include <stack>
#include <map>
#include <unordered_set>
#include <set>
#include<unordered_map>
#include <vector>
#include<queue>
#include<string>
#include<algorithm>
using namespace std;
struct NODE {
	char begin;
	string next;
	NODE(char begin, string next) {
		this->begin = begin;
		this->next = next;
	}
	NODE() {
		this->begin = NULL;
		this->next = "";
	}
};
struct ITEM {
	string value;
	NODE node;
	char next;
	char next_char;
	set<char> successor;
	int Successor_Status;
	ITEM(string value, NODE node, char next, set<char> successor, int Successor_Status, char next_char) {
		this->next = next;
		this->node = node;
		this->successor = successor;
		this->value = value;
		this->Successor_Status = Successor_Status;
		this->next_char = next_char;
	}
	ITEM() {
		this->next = ' ';
		this->next_char = ' ';
		this->Successor_Status = -1;
		this->node = NODE();
		this->successor = set<char>{};
		this->value = "";
	};
};
struct SetOfItems
{
	set<string>closures;
	set<string>deduplication_set;
	vector<ITEM> items;
	int Prev_Status;
	char Prev_Char;
	int index;
	SetOfItems(set<string> closures, vector<ITEM> items, set<string>deduplication_set, int Prev_Status, char Prev_Char, int index) {
		this->closures = closures;
		this->items = items;
		this->deduplication_set = deduplication_set;
		this->Prev_Char = Prev_Char;
		this->Prev_Status = Prev_Status;
		this->index = index;
	}
	SetOfItems() {
		this->closures = set<string>{};
		this->deduplication_set = set<string>{};
		this->index = -1;
		this->items = vector<ITEM>{};
		this->Prev_Char = ' ';
		this->Prev_Status = -1;
	};
};
string text = "./Grammatical/Grammatical7.txt";
char Start;//开始符号
string S;
unordered_set<char> VT;//终结符
unordered_set<char>VN; //非终结符
vector<char>Vn;//输入顺序排序的终结符
unordered_map<char, vector<string>>Epsilon;//非终结符对应的产生式
vector<string>Origin_Grammatical;//保存输入的文法
unordered_map<int, string> Epsilon_Index;
unordered_map<string, int> Epsilon_index_map;//产生式对应条数
unordered_set<string> Deduplication_Set;//去重集合
vector<SetOfItems>Analyze_Table;//LR(1)项目集规范组
unordered_map<char, unordered_set<char>> First;//first集
unordered_map<int, unordered_map<char, string>>ACTION_TABLE;//ACTION表
unordered_map<int, unordered_map<char, int>>GOTO_TABLE;//GOTO表
unordered_map<int, unordered_set<int>>Replace_Table;//LALR替换表
map<int, pair<int, int>>Epsilon_Action;//语义动作 : first: type -> 1.Monocular 2.binary 3.i; if first=1 -> second :type -> 1.+ 2.- 3.* 4./
int Epsilon_index = 0;
int Turn_Number(string s) //将
{
	int ans = 0;
	int k = 1;
	for (int i = s.size() - 1; i >= 0; i--) {
		if (s[i] < '0' || s[i] > '9')continue;
		ans += (s[i] - '0') * k;
		k *= 10;
	}
	return ans;
}
//将数字转化为字符串
string Number_Turn_String(int number) {
	if (number == 0)return "0";
	string ans = "";
	int temp = number < 0 ? -number : number;
	while (temp) {
		ans += (temp % 10) + '0';
		temp /= 10;
	}
	if (number < 0)ans += "-";
	return string(ans.rbegin(), ans.rend());
}
//打印产生式集合
void Printf_Epsilon_Set() {
	cout << "生成的产生式是:" << endl;
	int i = 0;
	for (int i = 0; i < Epsilon_Index.size(); i++) {
		cout << "\t" << i << ":" << Epsilon_Index[i] << endl;
	}
}
void Init_Action() {
	for (auto i = Epsilon_Index.begin(); i != Epsilon_Index.end(); i++) {
		map<char, int> flag;
		char k = ' ';
		for (int j = 3; j < i->second.size(); j++) {
			if (VT.find(i->second[j]) != VT.end()) {
				flag[i->second[j]]++;
			}
			else flag['N']++;
		}
		int type = -1;
		int index = -1;
		if (flag['N'] == 0)type = 3;
		else if (flag['('] != '\0' && flag[')'] != '\0')type = 4;
		else type = flag['N'];
		if (flag['+'] != '\0')index = 1;
		else if (flag['-'] != '\0')index = 2;
		else if (flag['*'] != '\0')index = 3;
		else if (flag['/'] != '\0')index = 4;
		Epsilon_Action[i->first] = pair<int, int>(type, index);
	}
}
//符号分类，初始化
void Sort_Char() {
	//构建非终结符表
	for (int i = 0; i < Origin_Grammatical.size(); i++) {
		string value = Origin_Grammatical[i];
		char vn = value[0];
		if (i == 0)Start = vn;
		VN.insert(vn);
		Vn.push_back(vn);
	}
	VT.insert('$');
	//构建终结符表,产生式
	for (int i = 0; i < Origin_Grammatical.size(); i++) {
		string value = Origin_Grammatical[i];
		char Begin_VN = value[0];
		int j = 0;
		string token = "";
		char vt;
		for (j = 1; j < value.size() - 1; j++) {
			if (value[j - 1] == '-' && value[j] == '>')break;
		}
		string temp_value = "";
		for (int k = j + 1; k < value.size(); k++) {
			if (value[k] == '|') {
				Epsilon[Begin_VN].push_back(token);
				temp_value = string(1, Begin_VN);
				temp_value += ("->" + token);
				Epsilon_Index[Epsilon_index] = temp_value;
				Epsilon_index_map[temp_value] = Epsilon_index++;
				token = "";
				continue;
			}
			else if (value[k] != ' ') {
				vt = value[k];
				if (vt != '#' && VN.find(vt) == VN.end())
					VT.insert(vt);
			}
			if (value[k] != ' ')token += value[k];
		}
		if (token != "") {
			Epsilon[Begin_VN].push_back(token);
			temp_value = string(1, Begin_VN);
			temp_value += ("->" + token);
			Epsilon_Index[Epsilon_index] = temp_value;
			Epsilon_index_map[temp_value] = Epsilon_index++;
		}
	}
}
//初始化 
void init() {
	fstream In(text);
	string val = "";
	while (getline(In, val)) {
		if (val != "" && val != "\n")
			Origin_Grammatical.push_back(val);
		else break;
	}
	Sort_Char();
	Init_Action();
}
//集合转化为字符串
string SetToString(set<char> temp) {
	string ans = "";
	for (char c : temp)ans += c;
	return ans;
}
//给文法加点
string Create_Next_Dot(string s) {
	int dot_flag = -1;
	for (int i = 0; i < s.size(); i++)
		if (s[i] == '.')dot_flag = i;
	if (dot_flag == -1)
		return "." + s;
	else if (dot_flag == s.size() - 1)
		return "acc";
	else
		return s.substr(0, dot_flag) + s[dot_flag + 1] + "." + s.substr(dot_flag + 2, s.size() - dot_flag - 2);
}
//去掉文法的点
string Remove_Dot(string s) {
	int dot_flag = -1;
	for (int i = 0; i < s.size(); i++)
		if (s[i] == '.')dot_flag = i;
	if (dot_flag == -1)return s;
	else return s.substr(0, dot_flag) + s.substr(dot_flag + 1, s.size() - dot_flag - 1);
}
//获取文法点后的字符
int Get_Next_Char(string s) {
	for (int i = 0; i < s.size(); i++) {
		if (s[i] == '.') {
			if (i == s.size() - 1)return -1;
			return i + 1;
		}
	}
	return -1;
}
//找到非终结符对应的第一个终结符
void Find_First_Value(char origin, char c, string P) {
	stack<char>temp;
	temp.push(c);
	while (!temp.empty()) {
		char T = temp.top();
		temp.pop();
		vector<string> tempValue = Epsilon.find(T)->second;
		for (int i = 0; i < tempValue.size(); i++) {
			if (VN.find(tempValue[i][0]) != VN.end()) {
				temp.push(tempValue[i][0]);
				if (tempValue[i][0] == T)temp.pop();
			}
			else First[origin].insert(tempValue[i][0]);
		}
	}
}
//构造First集
void Get_First_Set() {
	for (auto it = VN.begin(); it != VN.end(); it++) {
		vector<string> value = Epsilon.find(*it)->second;
		for (int i = 0; i < value.size(); i++) {
			if (VN.find(value[i][0]) != VN.end()) {
				if (*it != value[i][0])Find_First_Value(*it, value[i][0], value[i]);
			}
			else First[*it].insert(value[i][0]);
		}
	}
}
//获取后继符号
set<char> Get_First_Set(char b, set<char> a) {
	set<char> c;
	if (b == '$')return a;
	if (VN.find(b) != VN.end()) {
		a.clear();
		for (char c : First.find(b)->second)if (c != '$')a.insert(c);
		return a;
	}
	else {
		c.insert(b);
		return c;
	}
}
//初始化集合
SetOfItems initClosure() {
	string epsilon = Epsilon.find(Start)->second[0];
	string Next_Dot_String = Create_Next_Dot(epsilon);
	int next_dot_index = Get_Next_Char(Next_Dot_String);
	char next_dot_char = next_dot_index == -1 ? '$' : Next_Dot_String[next_dot_index];
	set<char>head{ '$' };
	string value = string(1, Start) + "->" + Next_Dot_String;
	set<string>closures{ value };
	set<string>New_Deduplication_Set{ value + SetToString(head) };
	vector<ITEM> items{ ITEM(value, NODE(Start, Next_Dot_String), next_dot_char, head, -1, '$') };
	return SetOfItems(closures, items, New_Deduplication_Set, 0, ' ', 0);
}
bool Is_Same(set<string> set1, set<string> set2) {
	if (set1.size() != set2.size())return false;
	set<string>set3;
	set_intersection(set1.begin(), set1.end(), set2.begin(), set2.end(), inserter(set3, set3.begin()));
	if (set3.size() == set1.size() && set3.size() == set2.size())return true;
	return false;
}
int Is_Set_Same(set<string> set1) {
	for (int i = 0; i < Analyze_Table.size(); i++)
		if (Is_Same(Analyze_Table[i].deduplication_set, set1))return i;
	return -1;
}
//设置后继符
void setSuccessor(unordered_set<char> b, ITEM& Origin) {
	for (char c : b)Origin.successor.insert(c);
}
//求状态I闭包
SetOfItems* CLOSURE(SetOfItems I) {
	vector<ITEM> NEW_STATE;
	unordered_set<char>Deduplication_Char;
	set<string> NEW_ITEMS;
	set<string>New_Deduplication_Set;
	set<string>Deduplication_Set_;
	for (ITEM item : I.items) {
		NEW_STATE.push_back(item);
		NEW_ITEMS.insert(item.value);
		Deduplication_Set_.insert(item.value + SetToString(item.successor));
	}
	for (ITEM item : I.items) {
		stack<ITEM>ITEM_STACK;
		ITEM_STACK.push(item);
		while (!ITEM_STACK.empty()) {
			ITEM S = ITEM_STACK.top();
			ITEM_STACK.pop();
			if (VT.find(S.next) != VT.end())continue;
			vector<string>epsilon = Epsilon.find(S.next)->second;
			for (string s : epsilon) {
				string Next_Dot_String = Create_Next_Dot(s);
				int next_dot_index = Get_Next_Char(Next_Dot_String);
				char next_dot_char = next_dot_index == -1 ? '$' : Next_Dot_String[next_dot_index];
				char next_char;
				if (next_dot_index == -1) {
					next_dot_char = '$';
					next_char = '$';
				}
				else {
					if (Next_Dot_String.size() - 1 == next_dot_index)next_char = '$';
					else next_char = Next_Dot_String[next_dot_index + 1];
				}
				string value = string(1, S.next) + "->" + Next_Dot_String;
				set<char> New_successor = Get_First_Set(S.next_char, S.successor);
				if (NEW_ITEMS.find(value) != NEW_ITEMS.end()) {
					for (int i = 0; i < NEW_STATE.size(); i++)
						if (NEW_STATE[i].value == value) {
							if (Deduplication_Set_.find(value + SetToString(New_successor)) != Deduplication_Set_.end())break;
							for (char c : New_successor)NEW_STATE[i].successor.insert(c);
							Deduplication_Set_.insert(value + SetToString(New_successor));
							ITEM_STACK.push(ITEM(value, NODE(S.next, Next_Dot_String), next_dot_char, New_successor, -1, next_char));
							break;
						}
					continue;
				}
				string D = value + SetToString(New_successor);
				if (Deduplication_Set_.find(D) != Deduplication_Set_.end())continue;
				ITEM NEW_ITEM = ITEM(value, NODE(S.next, Next_Dot_String), next_dot_char, New_successor, -1, next_char);
				NEW_STATE.push_back(NEW_ITEM);
				NEW_ITEMS.insert(value);
				ITEM_STACK.push(NEW_ITEM);
				Deduplication_Set_.insert(D);
			}
		}
	}
	for (ITEM item : NEW_STATE) {
		string value = item.value + SetToString(item.successor);
		Deduplication_Set.insert(value);
		New_Deduplication_Set.insert(value);
	}
	SetOfItems* Ans = new SetOfItems(NEW_ITEMS, NEW_STATE, New_Deduplication_Set, I.Prev_Status, I.Prev_Char, I.index);
	return Ans;
}
//由状态I通过X条件转换为另一个状态
SetOfItems* GOTO(SetOfItems I, int i, char X) {
	set<string>closures;
	set<string>deduplication_set;
	vector<ITEM> items;
	for (ITEM item : I.items) {
		if (item.next != X)continue;
		string Next_Dot_String = Create_Next_Dot(item.node.next);
		int next_dot_index = Get_Next_Char(Next_Dot_String);
		char next_dot_char = next_dot_index == -1 ? '$' : Next_Dot_String[next_dot_index];
		char next_char;
		if (next_dot_index == -1) {
			next_dot_char = '$';
			next_char = '$';
		}
		else {
			if (Next_Dot_String.size() - 1 == next_dot_index)next_char = '$';
			else next_char = Next_Dot_String[next_dot_index + 1];
		}
		string value = string(1, item.node.begin) + "->" + Next_Dot_String;
		ITEM NEW_ITEM = ITEM(value, NODE(item.node.begin, Next_Dot_String), next_dot_char, item.successor, -1, next_char);
		closures.insert(value);
		deduplication_set.insert(value + SetToString(item.successor));
		items.push_back(NEW_ITEM);
	}
	SetOfItems* New_Item = CLOSURE(SetOfItems(closures, items, deduplication_set, i, X, (int)Analyze_Table.size()));
	int flag = Is_Set_Same(New_Item->deduplication_set);
	if (flag != -1) {
		for (ITEM& item : Analyze_Table[i].items)
			if (item.next == X)item.Successor_Status = flag;
		return NULL;
	}
	return New_Item;
}
//填写转换状态
void Fill_Out_Successor_Status(int i, int target, char s) {
	for (ITEM& item : Analyze_Table[i].items) {
		if (item.Successor_Status != -1)continue;
		if (item.next == s)item.Successor_Status = target;
	}
}
//求GOTO图
void items() {
	//构造GOTO图
	SetOfItems Begin = *CLOSURE(initClosure());
	Analyze_Table.push_back(Begin);
	queue<SetOfItems>LR_Queue;
	LR_Queue.push(Begin);
	while (!LR_Queue.empty()) {
		SetOfItems S = LR_Queue.front();
		LR_Queue.pop();
		for (ITEM item : S.items) {
			if (item.next == '$')continue;
			SetOfItems* New_LR_Item = GOTO(S, S.index, item.next);
			if (New_LR_Item == NULL)continue;
			Analyze_Table.push_back(*New_LR_Item);
			LR_Queue.push(*New_LR_Item);
		}
	}
	//填入GOTO图后继状态
	for (int i = 1; i < Analyze_Table.size(); i++) {
		Fill_Out_Successor_Status(Analyze_Table[i].Prev_Status, i, Analyze_Table[i].Prev_Char);
	}
	for (int i = 0; i < Analyze_Table.size(); i++)
		for (ITEM& item : Analyze_Table[i].items) {
			if (item.Successor_Status == -1) {
				item.Successor_Status = (int)Analyze_Table.size();
				if (item.node.begin == Start) {
					ACTION_TABLE[i]['$'] = "acc";
					continue;
				}
				for (char c : item.successor) {
					ACTION_TABLE[i][c] = "r" + Number_Turn_String(Epsilon_index_map[Remove_Dot(item.value)]);
				}
			}
			else {
				if (VN.find(item.next) != VN.end()) GOTO_TABLE[i][item.next] = item.Successor_Status;
				else ACTION_TABLE[i][item.next] = "S" + Number_Turn_String(item.Successor_Status);
			}

		}
}
//打印SLR(1)各种表格
void Printf_(int number) {
	for (int i = 0; i < number; i++)cout << "------";
}
string SetToString_(set<char> s) {
	string ans = "";
	for (char c : s) {
		ans += c;
		ans += '/';
	}
	ans.pop_back();
	return ans;
}
pair<int, string> getToken(int& index) {
	string token = "";
	bool flag = false;
	int type = 0;
	for (index; index < S.size(); index++) {
		if (S[index] >= '0' && S[index] <= '9') {
			token += S[index];
			flag = true;
			type = 1;
		}
		else {
			if (!flag) {
				if (S[index] == '+')type = INT_MIN;
				else if (S[index] == '-')type = INT_MIN + 1;
				else if (S[index] == '*')type = INT_MIN + 2;
				else if (S[index] == '/')type = INT_MIN + 3;
				else if (S[index] == '(')type = INT_MIN + 4;
				else if (S[index] == ')')type = INT_MIN + 5;
				else type = 0;
				token += S[index++];
				return pair<int, string>(type, token);
			}
			return pair<int, string>(type, token);
		}
	}
}
void Printf_Analyze_Item(int index, int step, stack<int> Stack, stack<char> Symbol, string action, int type) {
	vector<int>Stack_V;
	string Stack_S = "";
	while (!Stack.empty()) {
		Stack_V.push_back(Stack.top());
		Stack.pop();
	}
	for (int i = Stack_V.size() - 1; i >= 0; i--) {
		if (Stack_V[i] == INT_MIN)Stack_S += "+ ";
		else if (Stack_V[i] == INT_MIN + 1)Stack_S += "- ";
		else if (Stack_V[i] == INT_MIN + 2)Stack_S += "* ";
		else if (Stack_V[i] == INT_MIN + 3)Stack_S += "/ ";
		else if (Stack_V[i] == INT_MIN + 4) Stack_S += "(";
		else if (Stack_V[i] == INT_MIN + 5)Stack_S += ")";
		else Stack_S += (Number_Turn_String(Stack_V[i]) + " ");
	}
	vector<char>Symbol_V;
	string Symbol_S = "";
	while (!Symbol.empty()) {
		Symbol_V.push_back(Symbol.top());
		Symbol.pop();
	}
	for (int i = Symbol_V.size() - 1; i >= 0; i--)Symbol_S += Symbol_V[i];
	printf("%-4d%-16s%-16s%-16s%s\n", step, S.substr(index).c_str(), Symbol_S.c_str(), Stack_S.c_str(), action.c_str());
}
int MoveIn_Protocol(string s) {
	if (s == "")return -1;
	else if (s[0] == 'S')return 0;
	else if (s == "acc")return 2;
	return 1;
}
bool Analyze_Grammar() {
	S.push_back('$');
	printf("%-4s%-16s%-16s%-16s%s\n", "", "输入", "state", "val", "所用产生式");
	int step = 0;
	int index = 0;
	stack<char>Symbol;
	stack<string>Stack;
	stack<int>value;
	Stack.push("0");
	while (1) {
		string s = Stack.top();
		int temp = index;
		pair<int, string> token = getToken(temp);
		char c = ' ';
		if (token.first <= 0)c = token.second[0];
		else c = 'i';
		string item = ACTION_TABLE[Turn_Number(s)][c];
		int flag = MoveIn_Protocol(item);
		if (flag == 0) {
			//进行移入
			Printf_Analyze_Item(index, step++, value, Symbol, "", 0);
			if (c == 'i')value.push(Turn_Number(token.second));
			else if (token.first < 0)value.push(token.first);
			index = temp;
			Symbol.push(c);
			Stack.push(item.substr(1));
		}
		else if (flag == 1) {
			//进行规约
			string Grammar_ = Epsilon_Index[Turn_Number(item)];
			pair<int, int>action = Epsilon_Action[Turn_Number(item)];
			Printf_Analyze_Item(index, step++, value, Symbol, Grammar_, 1);
			if (action.first == 2 || action.first == 4) {
				int number1 = value.top();//stack[top]
				value.pop();
				int number3 = value.top();
				value.pop();
				int number2 = value.top();//stack[top-2]
				value.pop();
				if (action.second == 1)value.push(number2 + number1);
				else if (action.second == 2)value.push(number2 - number1);
				else if (action.second == 3)value.push(number2 * number1);
				else if (action.second == 4)value.push(number2 / number1);
				else value.push(number3);
			}
			for (int t = 0; t < Grammar_.size() - 3; t++) {
				Symbol.pop();
				Stack.pop();
			}
			Stack.push(Number_Turn_String(GOTO_TABLE[Turn_Number(Stack.top())][Grammar_[0]]));
			Symbol.push(Grammar_[0]);
		}
		else if (flag == 2) {
			//接受
			Printf_Analyze_Item(index, step++, value, Symbol, "acc", 2);
			return true;
		}
		else {
			//error
			return false;
		}
	}
	return true;
}
int main() {
	init();
	Printf_Epsilon_Set();
	Get_First_Set();
	items();
	while (1) {
		cout << "请输入要分析的字符串:";
		cin >> S;
		if (S == "exit")break;
		if (Analyze_Grammar())cout << "语法分析成功\n";
		else cout << "语法分析失败\n";
	}
	return 0;
}