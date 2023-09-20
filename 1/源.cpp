#include<iostream>
#include<string>
#include<map>
#include<fstream>
#include <stack>
using namespace std;
map<string, string> maps;
int i = 0;
string token = "";
string syn = "-1";
bool is_over = false;
stack<char> Parentheses;
stack<char> Brackets;
bool is_legal = true;
string error_message = "";
bool is_letter(char ch) {
	if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))return true;
	return false;
}
bool is_number(char ch) {
	if (ch >= '0' && ch <= '9')return true;
	return false;
}
void scan(string str) {
	if (str[i] == ' ') {
		syn = "31";
		while (str[i] == ' ')i++;
	}
	else if (str[i] == '\n') {
		syn = "31";
		while (str[i] == '\n')i++;
	}
	else {
		token = "";
		if (is_number(str[i])) {
			while (is_number(str[i])) {
				token += str[i++];
			}
			syn = "30";
		}
		else if (is_letter(str[i])) {
			token = "";
			while (is_number(str[i]) || is_letter(str[i])) token += str[i++];
			if (maps.find(token) == maps.end())syn = "32";
			else {
				auto it = maps.find(token);
				syn = it->second;
			}
		}
		else {
			token = "";
			switch (str[i]) {
			case '=':
				token = str[i];
				syn = maps.find("=")->second;
				i++;
				if (str[i] == '=')syn = maps.find("==")->second;
				break;
			case '!':
				syn = "-1";
				error_message = "非法标识符";
				i++;
				if (str[i] == '=')syn = maps.find("!=")->second;
				break;
			case '(':
				Parentheses.push('(');
				token = str[i++];
				syn = maps.find("(")->second;
				break;
			case ')':
				if (Parentheses.empty())is_legal = false;
				else Parentheses.pop();
				token = str[i++];
				syn = maps.find(")")->second;
				break;
			case '{':
				Brackets.push('{');
				token = str[i++];
				syn = maps.find("{")->second;
				break;
			case '}':
				if (Brackets.empty())is_legal = false;
				else Brackets.pop();
				token = str[i++];
				syn = maps.find("}")->second;
				break;
			case '#':
				is_over = true;
				syn = "over";
				break;
			case '\0':
				is_over = true;
				syn = "over";
				break;
			default:
				token = str[i++];
				if (maps.find(token) != maps.end()) {
					syn = maps.find(token)->second;
				}
				else {
					syn = "-1";
					error_message = "非法标识符";
				}
				break;
			}
		}
	}
}
int main() {
	fstream inFile(".\\CIFA.txt");
	string str;
	while (inFile >> str) {
		string key = str;
		inFile >> str;
		maps[key] = str;
	}
	fstream file(".\\data.txt");
	string tem = "";
	str = "";
	while (getline(file, tem)) {
		str += tem;
	}
	str.push_back('\0');
	cout << str << endl;
	do {
		scan(str);
		if (!is_legal) {
			cout << "编译失败！\nerror: 左右括号没有对齐" << endl;
			return 0;
		}
		if (is_over) {
			if (!Parentheses.empty() || !Brackets.empty()) {
				cout << "编译失败！\nerror: 左右括号没有对齐" << endl;
			}
			else cout << "compilation successful!" << endl;
			return 0;
		}
		if (syn == "-1") {
			cout << "编译失败！\nerror: " << error_message << endl;
			return 0;
		}
		else if (syn == "31")continue;
		else if (syn == "30") {
			cout << "<" << token << ", 常数>" << endl;
		}
		else if (syn == "32") {
			cout << "<" << token << ", 标识符>" << endl;
		}
		else {
			cout << "<" << token << ", " << syn << ">" << endl;
		}
	} while (!is_over);
	return 0;
}
