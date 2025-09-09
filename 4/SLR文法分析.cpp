#include <iostream>
#include <fstream>
#include<stdio.h>
#include <string>
#include <stack>
#include <map>
#include <unordered_set>
#include <set>
#include <vector>
#include<queue>
#include<string>
#include<algorithm>
using namespace std;
//产生式
struct Grammatical_node {
	char begin;
	string next;
	Grammatical_node(char begin, string next) {
		this->begin = begin;
		this->next = next;
	}
	Grammatical_node() {}
};
//每个Sn详细状态条目
struct SLR_Node_Message {
	Grammatical_node Project_Set;
	string value;
	string Next_Char;
	int Successor_Status;
	SLR_Node_Message(Grammatical_node Project_Set, string value, string Next_Char, int Successor_Status) {
		this->Successor_Status = Successor_Status;
		this->Next_Char = Next_Char;
		this->value = value;
		this->Project_Set = Project_Set;
	}
	SLR_Node_Message() {}
};
//Sn状态表
struct SLR_Map {
	vector<SLR_Node_Message> Project_Sets;
	set<string> Set;
	string next_char;
	SLR_Map(vector<SLR_Node_Message> Project_Sets, set<string>Set, string next_char) {
		this->Project_Sets = Project_Sets;
		this->Set = Set;
		this->next_char = next_char;
	}
	SLR_Map() {}
};
string S;
string text = "./Grammatical/Grammatical7.txt";//打开文件
char Start;//开始符号
unordered_set<char> VT, VN;//终结符以及非终结符
vector<char>Vt;//输入顺序排序的终结符
map<char, unordered_set<char>>Follow, First;//follow集，first集
map<char, vector<string>>Epsilon;//非终结符对应的产生式
vector<string>Origin_Grammatical;//保存输入的文法
string Word_String;//分析的单词串
vector<SLR_Map>Analyze_Table;//分析表
unordered_set<string>Deduplication_Set;//去重集合
map<int, string> Epsilon_Index;
map<string, int> Epsilon_index_map;//产生式对应条数
map<string,map<char,string>>ACTION;//ACTION表
map<string, map<char, int>>GOTO;//GOTO表
bool is_slr = true;
int Epsilon_index = 0;
//将数字转化为字符串
string Turn_String(int number) {
	if (number == 0)return "0";
	string ans = "";
	while (number) {
		ans += (number % 10) + '0';
		number /= 10;
	}
	return string(ans.rbegin(), ans.rend());
}
//将数字字符串转换为数字
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
//符号归类
void Sort_Char() {
	//构建非终结符表
	for (int i = 0; i < Origin_Grammatical.size(); i++) {
		string value = Origin_Grammatical[i];
		char vt = value[0];
		if (i == 0)Start = vt;
		VT.insert(vt);
		Vt.push_back(vt);
	}
	VN.insert('$');
	//构建终结符表,产生式
	for (int i = 0; i < Origin_Grammatical.size(); i++) {
		string value = Origin_Grammatical[i];
		char Begin_VT = value[0];
		int j = 0;
		string token = "";
		char vn;
		for (j = 1; j < value.size() - 1; j++) {
			if (value[j - 1] == '-' && value[j] == '>')break;
		}
		string temp_value = "";
		for (int k = j + 1; k < value.size(); k++) {
			if (value[k] == '|') {
				Epsilon[Begin_VT].push_back(token);
				temp_value = string(1, Begin_VT);
				temp_value += ("->" + token);
				Epsilon_Index[Epsilon_index] = temp_value;
				Epsilon_index_map[temp_value] = Epsilon_index++;
				token = "";
				continue;
			}
			else if (value[k] != ' ') {
				vn = value[k];
				if (vn != '#' && VT.find(vn) == VT.end())
					VN.insert(vn);
			}
			if (value[k] != ' ')token += value[k];
		}
		if (token != "") {
			Epsilon[Begin_VT].push_back(token);
			temp_value = string(1, Begin_VT);
			temp_value += ("->" + token);
			Epsilon_Index[Epsilon_index] = temp_value;
			Epsilon_index_map[temp_value] = Epsilon_index++;

		}
	}
}
void init() {
	fstream In(text);
	string val = "";
	while (getline(In, val)) {
		if (val != "" && val != "\n")
			Origin_Grammatical.push_back(val);
		else break;
	}
	Sort_Char();

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
			if (VT.find(tempValue[i][0]) != VT.end()) {
				temp.push(tempValue[i][0]);
				if (tempValue[i][0] == T)temp.pop();
			}
			else {
				First[origin].insert(tempValue[i][0]);
			}
		}
	}
}
//构造First集
void Get_First_Set() {
	for (auto it = VT.begin(); it != VT.end(); it++) {
		vector<string> value = Epsilon.find(*it)->second;
		for (int i = 0; i < value.size(); i++) {
			if (VT.find(value[i][0]) != VT.end()) {
				if (*it != value[i][0])Find_First_Value(*it, value[i][0], value[i]);
			}
			else {
				First[*it].insert(value[i][0]);
			}
		}
	}
}
//c集合是否为存在空集
bool Is_Null_Set(char c) {
	vector<string>L = Epsilon.find(c)->second;
	for (int i = 0; i < L.size(); i++) {
		if (L[i][0] == '#')return true;
	}
	return false;
}
//把Follow(a)插入Follow(T)的集合
void Insert_Follow(char T, unordered_set<char>a) {
	for (auto it = a.begin(); it != a.end(); it++)
		if (*it != '#')Follow[T].insert(*it);
}
//找到 target右边的字符
char Find_Right_Char(char target, string s) {
	for (int i = 0; i < s.size(); i++) {
		if (s[i] == target) {
			return i == s.size() - 1 ? ' ' : s[i + 1];
		}
	}
	return ' ';
}
//在s字符串中是否存在 target
bool Find_Target(char target, string s) {
	for (int i = 0; i < s.size(); i++) {
		if (s[i] == target)return true;
	}
	return false;
}
//找到 target在Vt中的下标
int Find_Array_Index(char target) {
	for (int i = 0; i < Vt.size(); i++) {
		if (Vt[i] == target)return i;
	}
	return -1;
}
//构造Follow集
void Get_Follow_Set(int begin) {
	for (auto it = begin; it < Vt.size(); it++) {
		if (Vt[it] == Start)Follow[Vt[it]].insert('$');
		vector<string> epsilon = Epsilon.find(Vt[it])->second;
		for (int i = 0; i < epsilon.size(); i++) {
			string value = epsilon[i];
			for (int j = 0; j < value.size(); j++) {
				if (VN.find(value[j]) != VN.end() || value[j] == '#')continue;
				char right = Find_Right_Char(value[j], value);
				int origin_size = Follow[value[j]].size();
				if (VN.find(right) == VN.end()) {
					if (right != ' ') {
						Insert_Follow(value[j], First[right]);
					}
					if (right == ' ' || Is_Null_Set(right)) {
						Insert_Follow(value[j], Follow[Vt[it]]);
					}
				}
				else Follow[value[j]].insert(right);
				int now_size = Follow[value[j]].size();
				int index = Find_Array_Index(value[j]);
				if (now_size != origin_size && index < it) {
					Get_Follow_Set(index);
				}
			}
		}
	}
}
//打印产生式集合
void Printf_Epsilon_Set() {
	cout << "生成的产生式是:" << endl;
	int i = 0;
	for (int i = 0; i < Epsilon_Index.size(); i++) {
		cout << "\t" << i << ":" << Epsilon_Index[i] << endl;
	}
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
char Get_Next_Char(string s) {
	for (int i = 0; i < s.size(); i++) {
		if (s[i] == '.') {
			if (i == s.size() - 1)return NULL;
			return s[i + 1];
		}
	}
	return NULL;
}
void Printf_(int number) {
	for (int i = 0; i < number; i++)cout << "------";
}
//非终结符N能够抵达的所有闭包
void Create_N_Closure(char N, vector<SLR_Node_Message>& value, set<string>& Set, string& Next_Char_Set) {
	stack<char>VT_STACK;
	unordered_set<char> Deduplication_Vt_Set;//用来去重放入栈的符号
	VT_STACK.push(N);
	unordered_set<char>Deduplication_Char_Set;//用来去重后继符
	for (char c : Next_Char_Set)Deduplication_Char_Set.insert(c);
	while (!VT_STACK.empty()) {
		char top = VT_STACK.top();
		VT_STACK.pop();
		if (Deduplication_Vt_Set.find(top) == Deduplication_Vt_Set.end()) {
			Deduplication_Vt_Set.insert(top);
		}
		else continue;
		for (string s : Epsilon.find(top)->second) {
			string first = Create_Next_Dot(s);
			char next_char = Get_Next_Char(first);
			string V = string(1, top);
			V += ("->" + first);
			if (next_char == NULL) {
				if (Start == top)value.push_back(SLR_Node_Message(Grammatical_node(top, first), V, "$", 0));
				else value.push_back(SLR_Node_Message(Grammatical_node(top, first), V, Remove_Dot("$" + V), 0));
			}
			else {
				value.push_back(SLR_Node_Message(Grammatical_node(top, first), V, string(1, next_char), 0));
				if (VT.find(next_char) != VT.end()) {
					VT_STACK.push(next_char);
				}
				if (Deduplication_Char_Set.find(next_char) == Deduplication_Char_Set.end()) {
					Deduplication_Char_Set.insert(next_char);
					if (Deduplication_Set.find(V) == Deduplication_Set.end())Next_Char_Set.push_back(next_char);
				}
			}
			Set.insert(V);
			Deduplication_Set.insert(V);
		}
	}
}
//打印SLR(1)分析表
void Printf_SLR() {
	cout << "SLR分析表:\n\n";
	printf("%-8s%-16s%-16s%-8s", "状态", "项目集", "后继符号", "后继状态\n");
	Printf_(8);
	for (int i = 0; i < Analyze_Table.size(); i++) {
		printf("\n%s%-7d", "S", i);
		for (int j = 0; j < Analyze_Table[i].Project_Sets.size(); j++) {
			string value = Analyze_Table[i].Project_Sets[j].value;
			string Next_Char = Analyze_Table[i].Project_Sets[j].Next_Char;
			int Successor_Status = Analyze_Table[i].Project_Sets[j].Successor_Status;
			if (j == 0)printf("%-16s", value.c_str());
			else printf("%-8s%-16s", "", value.c_str());
			printf("%-16s%s%-8d\n", Next_Char.c_str(),"S", Successor_Status);
		}
		Printf_(8);
		cout << endl;
	}
}
//创建初始集合，初始化函数
void Init_Closure() {
	vector<SLR_Node_Message>value;
	set<string>Set;
	string next_char;
	Create_N_Closure(Start, value, Set, next_char);
	Analyze_Table.push_back(SLR_Map(value, Set, next_char));
}
//创建集合闭包
SLR_Map Create_Closure_Condition(SLR_Map slr_map, char condition) {
	vector<SLR_Node_Message>value = slr_map.Project_Sets;
	vector<SLR_Node_Message>New_State_Array;
	string next_char = "";
	set<string>Set;
	unordered_set<char>char_set;
	for (int i = 0; i < value.size(); i++) {
		if (value[i].Next_Char != string(1, condition))continue;
		string first = Create_Next_Dot(value[i].Project_Set.next);
		char next_dot = Get_Next_Char(first);
		string V = string(1, value[i].Project_Set.begin);
		V += ("->" + first);
		if (next_dot == NULL) {
			if (value[i].Project_Set.begin == Start)New_State_Array.push_back(SLR_Node_Message(Grammatical_node(value[i].Project_Set.begin, first), V, "$", 0));
			else New_State_Array.push_back(SLR_Node_Message(Grammatical_node(value[i].Project_Set.begin, first), V, Remove_Dot("$" + V), 0));
		}
		else {
			//若后继符为终结符
			if (Deduplication_Set.find(V) == Deduplication_Set.end()) {
				Deduplication_Set.insert(V);
				next_char.push_back(next_dot);
			}
			if (VT.find(next_dot) != VT.end()) {
				New_State_Array.push_back(SLR_Node_Message(Grammatical_node(value[i].Project_Set.begin, first), V, string(1, next_dot), 0));
				Create_N_Closure(next_dot, New_State_Array, Set, next_char);
			}
			else {
				New_State_Array.push_back(SLR_Node_Message(Grammatical_node(value[i].Project_Set.begin, first), V, string(1, next_dot), 0));
			}
		}
		Set.insert(V);
	}
	return SLR_Map(New_State_Array, Set, next_char);
}
//判断是否存在交集
bool Is_Intersect(set<string> set1, set<string> set2) {
	set<string>set3;
	set_intersection(set1.begin(), set1.end(), set2.begin(), set2.end(), inserter(set3, set3.begin()));
	if (set3.size() == set1.size())return true;
	return false;
}
//查找DFA对应状态
int Find_Status(set<string> set1) {
	for (int i = 0; i < Analyze_Table.size(); i++) {
		set<string>set2 = Analyze_Table[i].Set;
		if (Is_Intersect(set1, set2)) {
			return i;
		}
	}
}
//打印ACTION和GOTO
void Printf_ACTION_GOTO() {
	cout << "\nACTION表:\n";
	printf("%-6s\t\t\t%s\n", "状", "ACTION"); printf("%-6s", "态");
	for (char c : VN) {
		printf("%-6c", c);
	}
	cout << "|" << endl; Printf_(VN.size() + 1); cout << endl;
	for (int i = 0; i < Analyze_Table.size(); i++) {
		string In = "S" + Turn_String(i);
		printf("%-6s", In.c_str());
		for (char c : VN) {
			printf("%-6s", ACTION[In][c].c_str());
		}
		cout << "|" << endl; Printf_(VN.size() + 1); cout << endl;
	}
	cout << "\nGOTO表:\n";
	printf("%-6s\t\t%s\n", "状", "GOTO"); printf("%-6s", "态");
	for (char c : VT) if(c != Start)printf("%-6c", c);
	cout << "|" << endl; Printf_(VT.size()); cout << endl;
	for (int i = 0; i < Analyze_Table.size(); i++) {
		string In = "S" + Turn_String(i);
		printf("%-6s", In.c_str());
		for (char c : VT) {
			if (c == Start)continue;
			if (GOTO[In][c] == 0)printf("%-6s", "");
			else printf("%-6d", GOTO[In][c]);
		}
		cout << "|" << endl; Printf_(VT.size()); cout << endl;
	}
}
//形成拓广文法
void Create_Broadening_Grammar() {
	Init_Closure();
	queue<SLR_Map>SLR_STACK;
	SLR_STACK.push(Analyze_Table[0]);
	unordered_set<string>Deduplication_Status_Set;
	char next;
	string In = "";
	string N = "";
	while (!SLR_STACK.empty()) {
		SLR_Map temp_slr = SLR_STACK.front();
		SLR_STACK.pop();
		//状态去重
		for (char c : temp_slr.next_char) {
			SLR_Map New_Status = Create_Closure_Condition(temp_slr, c);
			SLR_STACK.push(New_Status);
			Analyze_Table.push_back(New_Status);
		}
	}
	for (int i = 0; i < Analyze_Table.size(); i++) {
		set<string>intersect_set;
		string begin = "";
		int index = 0;
		map<int, int>index_map;
		for (SLR_Node_Message &s : Analyze_Table[i].Project_Sets) {
			string next = Create_Next_Dot(s.value);
			if (s.Next_Char == begin) {
				s.Successor_Status = index;
			}
			else if (next == "acc") {
				s.Successor_Status = Analyze_Table.size();
				continue;
			}
			else {
				if (intersect_set.empty()) {
					s.Successor_Status = index;
					intersect_set.insert(next);
					begin = s.Next_Char;
					continue;
				}
				index_map[index] = Find_Status(intersect_set);
				++index;
				s.Successor_Status = index;
				intersect_set.clear();
				begin = s.Next_Char;
			}
			intersect_set.insert(next);
		}
		if (!intersect_set.empty()) {
			index_map[index] = Find_Status(intersect_set);
		}
		int k = 0; 
		bool Protocol_In = false;
		bool MoveIn = false;
		for (SLR_Node_Message &s : Analyze_Table[i].Project_Sets) {
			In = ("S" + Turn_String(i));
			next = s.Next_Char[0];
			if (s.Next_Char[0] == '$') {
				if (Start == s.Project_Set.begin)ACTION[In][next] = "acc";
				else{
					string k = Remove_Dot(s.value);
					for (char c : Follow.find(s.Project_Set.begin)->second) {
						if(c == '$')ACTION[In]['$'] = "r" + Turn_String(Epsilon_index_map[k]);
						else ACTION[In][c] = "r" + Turn_String(Epsilon_index_map[k]);
					}
					Protocol_In = true;
				}
				continue;
			}
			s.Successor_Status = index_map[s.Successor_Status];
			N = ("S" + Turn_String(s.Successor_Status));
			if (VT.find(s.Next_Char[0]) == VT.end())
			{
				ACTION[In][next] = N;
				MoveIn = true;
			}
			else GOTO[In][next] = s.Successor_Status;
		}
		if (MoveIn && Protocol_In)is_slr = false;
	}
}
int MoveIn_Protocol(string s) {
	if (s == "")return -1;
	else if (s[0] == 'S')return 0;
	else if (s == "acc")return 2;
	return 1;
}
void Printf_Analyze_Item(int index,int step, stack<string> Stack, stack<char> Symbol, string action, int type) {
	string Action_Value = "";
	vector<string>Stack_V;
	string Stack_S = "";
	while (!Stack.empty()) {
		Stack_V.push_back(Stack.top());
		Stack.pop();
	}
	for (int i = Stack_V.size() - 1; i >= 0; i--)Stack_S += (Stack_V[i] + " ");
	vector<char>Symbol_V;
	string Symbol_S = "";
	while (!Symbol.empty()) {
		Symbol_V.push_back(Symbol.top());
		Symbol.pop();
	}
	for (int i = Symbol_V.size() - 1; i >= 0; i--)Symbol_S += Symbol_V[i];
	if (type == 0 || type == 2)Action_Value = action;
	else Action_Value = "根据" + action + "规约";
	printf("%-4d%-16s%-16s%-16s%s\n", step, Stack_S.c_str(), Symbol_S.c_str(), S.substr(index).c_str(), Action_Value.c_str());
}
bool Analyze_Grammar() {
	S.push_back('$');
	printf("%-4s%-16s%-16s%-16s%s\n", "", "栈", "符号", "输入", "动作");
	int step = 0;
	int index = 0;
	stack<char>Symbol;
	stack<string>Stack;
	Stack.push("0");
	while (1) {
		string s = Stack.top();
		string item = ACTION["S" + s][S[index]];
		int flag = MoveIn_Protocol(item);
		if (flag == 0) {
			//进行移入
			Printf_Analyze_Item(index, step++, Stack, Symbol, "移入", 0);
			Symbol.push(S[index++]);
			Stack.push(item.substr(1));
		}
		else if (flag == 1) {
			//进行规约
			string Grammar_ = Epsilon_Index[Turn_Number(item)];
			Printf_Analyze_Item(index, step++, Stack, Symbol, Grammar_, 1);
			for (int t = 0; t < Grammar_.size() - 3; t++) {
				Symbol.pop();
				Stack.pop();
			}
			Stack.push(Turn_String(GOTO["S" + Stack.top()][Grammar_[0]]));
			Symbol.push(Grammar_[0]);
		}
		else if (flag == 2) {
			//接受
			Printf_Analyze_Item(index, step++, Stack, Symbol, "接受", 2);
			return true;
		}
		else {
			//error
			return false;
		}
	}
	return true;
}
//主函数
int main() {
	init();
	Printf_Epsilon_Set();
	Get_First_Set();
	Get_Follow_Set(0);
	Create_Broadening_Grammar();
	Printf_SLR();
	if (is_slr)cout << "该文法属于SLR文法\n";
	else cout << "该文法不属于SLR文法\n";
	Printf_ACTION_GOTO();
	cout << "请输入要分析的字符串:";
	cin >> S;
	if (Analyze_Grammar())cout << "语法分析成功\n";
	else cout << "语法分析失败\n";
	return 0;
}