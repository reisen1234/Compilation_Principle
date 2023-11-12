#include <iostream>
#include <fstream>
#include <string>
#include <stack>
#include <map>
#include <unordered_set>
#include <vector>
using namespace std;
string text = "./Grammatical/Grammatical4.txt";
char Start;//开始符号
unordered_set<char> VT, VN;//终结符以及非终结符
vector<char>Vt;//输入顺序排序的终结符
map<char, unordered_set<char>>Follow, First;//follow集，first集
map<char, map<char, string>>ForecastMap;//预测表
map<char, vector<string>>Epsilon;//非终结符对应的产生式
vector<string>Origin_Grammatical;//保存输入的文法
string Word_String;//分析的单词串
//符号分类，初始化
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
		
		for (int k = j + 1; k < value.size(); k++) {
			if (value[k] == '|') {
				Epsilon[Begin_VT].push_back(token);
				token = "";
				continue;
			}
			else if(value[k] != ' ') {
				vn = value[k];
				if (vn != '#' && VT.find(vn) == VT.end())
					VN.insert(vn);
			}
			if(value[k] != ' ')token += value[k];
		}	
		if (token != "")Epsilon[Begin_VT].push_back(token);
	}
}
//初始化
void init() {
	fstream In(text);
	string val = "";
	while (getline(In,val)) {
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
				if (tempValue[i][0] != '#') {
					string tempString = "";
					tempString += P;
					ForecastMap[origin][tempValue[i][0]] = tempString;
				}
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
				if(*it != value[i][0])Find_First_Value(*it, value[i][0],value[i]);
			}
			else {
				First[*it].insert(value[i][0]);
				if (value[i][0] != '#') {
					string tempString = "";
					tempString += value[i];
					ForecastMap[*it][value[i][0]] = tempString;
				}
			}
		}
	}
}
//集合是否为存在空集
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
		if(*it != '#')Follow[T].insert(*it);
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
//构建预测分析表
void Build_Forecast_Map() {
	for (auto i = First.begin(); i != First.end(); i++) {
		if (i->second.find('#') != i->second.end()) {
			unordered_set<char> FOLLOW_B = Follow.find(i->first)->second;
			for (auto it = FOLLOW_B.begin(); it != FOLLOW_B.end(); it++) {
				string tempValue = "";
				tempValue += "#";
				ForecastMap[i->first][*it] = tempValue;
			}
		}
	}
}
//打印产生式集合
void Printf_Epsilon_Set() {
	cout << "从文件读入的文法为:" << endl;
	for (auto it = Epsilon.begin(); it != Epsilon.end(); it++) {
		cout << it->first << "->";
		for (auto i = it->second.begin(); i != it->second.end(); i++) {
			cout << *i;
			if (i + 1 != it->second.end())cout << '|';
			else cout << endl;
		}
	}
}
 //打印First集合
void Printf_First_Set() {
	cout << "\nFirst集合:" << endl;
	for (int i = 0; i < Vt.size(); i++) {
		cout << "\tFirst(" << Vt[i] << ")={";
		for (auto it = First[Vt[i]].begin(); it != First[Vt[i]].end(); it++) {
			cout << *it;
			if (++it != First[Vt[i]].end()) {
				cout << ',';
				--it;
			}
			else {
				cout << "}\n";
				--it;
			}
		}
	}
}
 //打印Follow集合
void Printf_Follow_Set() {
	cout << "\nFollow集合:" << endl;
	for (int i = 0; i < Vt.size(); i++) {
		cout << "\tFollow(" << Vt[i] << ")={";
		for (auto it = Follow[Vt[i]].begin(); it != Follow[Vt[i]].end(); it++) {
			cout << *it;
			if (++it != Follow[Vt[i]].end()) {
				cout << ',';
				--it;
			}
			else {
				cout << "}\n";
				--it;
			}
		}
	}
}
//打印分析的结果
void Printf_Analysis_Answer(int SYM, stack<char>ans, string action, string epsilon) {
	string Ans = "";
	while (!ans.empty()) {
		Ans += ans.top();
		ans.pop();
	}
	cout << Word_String.substr(0, SYM) << "\t\t";
	cout << Ans << "\t\t";
	cout << Word_String.substr(SYM, Word_String.size() - SYM) << "\t";
	cout << action << "\t";
	cout << epsilon << endl;
}
//输出分析过程
bool Analysis() {
	cout << "请输入要分析的字符串:";
	cin >> Word_String;
	cout << Word_String << "的分析过程:";
	Word_String.push_back('$');
	stack<char>Stack;
	Stack.push('$');
	Stack.push(Start);
	int SYM = 0;
	cout << "\n已匹配\t\t" << "栈\t\t" << "输入\t" << "动作\t" << "产生式\t" << endl;
	 (SYM,Stack,"","");
	while (!Stack.empty()) {
		char out = Stack.top();
		Stack.pop();
		string forecast = ForecastMap[out][Word_String[SYM]];
		if (Word_String[SYM] == out) {
			string A = "";
			A += out;
			SYM++;
			if(out != '$')Printf_Analysis_Answer(SYM, Stack, "匹配", A);
		}
		else if (forecast == "") {
			return false;
		}
		else if(forecast != "") {
			string A = "";
			A += out;
			A += "->";
			A += forecast;
			if(forecast != "#")
				for (int i = forecast.size() - 1; i >= 0; i--)Stack.push(forecast[i]);
			Printf_Analysis_Answer(SYM, Stack, "输出", A);
		}
		else if (VT.find(out) != VT.end()) {
			return false;
		}
	}
	return true;
}
//打印预测分析表
void Printf_Forecast_Map() {
	cout << "该文法的预测分析表是:" << endl << "\t";
	for (auto it = VN.begin(); it != VN.end(); it++) {
		cout << *it << "\t";
	}
	cout << endl;
	for (int i = 0; i < Vt.size(); i++) {
		cout << Vt[i] << '\t';
		for (auto it = VN.begin(); it != VN.end(); it++) {
			if(ForecastMap[Vt[i]][*it] != "")
				cout << Vt[i] << "->" << ForecastMap[Vt[i]][*it] << "\t";
			else cout << "\t";
		}
		cout << endl;
	}
	cout << endl;
}
//主函数
int main() {
	init();
	Printf_Epsilon_Set();
	Get_First_Set();
	Printf_First_Set();
	Get_Follow_Set(0);
	Printf_Follow_Set();
 	Build_Forecast_Map();
	Printf_Forecast_Map();
	Analysis() == true ? cout << "\n语法分析成功" : cout << "\n语法分析失败";
	return 0;
}