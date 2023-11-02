#include <iostream>
#include<fstream>
#include<unordered_set>
#include <stack>
#include<map>
#include<queue>
#include<set>
#include<algorithm>
using namespace std;
map<int, int> ��_closure;//״̬ T �������� a ���Ե��������״̬�ļ���
map<char, map<char, char>>table;//����DFA��
map<char, char>replace_table;//�滻��
vector<set<char>> terminal;//���Ӽ�
queue<set<char>>non_terminal;//�����Ӽ�
unordered_set<char> maps[200][200];//ͼ�����ж������ת��
unordered_set<char> sets;//���ֵ���������a�ļ���
unordered_set<int> Repetition;//״̬��¼�����ظ�
vector<int>ans;//���д�״̬����
int F = INT_MAX, E = INT_MIN;//����״̬�Ŀ�ʼ�����
static int Begin, End;//����Ŀ�ʼ����״̬
//�������ַ���ת��Ϊ����
int Turn_Number(string s) //��
{
	int ans = 0;
	int k = 1;
	for (int i = s.size() - 1; i >= 0; i--) {
		ans += (s[i] - '0') * k;
		k *= 10;
	}
	return ans;
}
//�ҵ��������Ӧ���±꣬ת��Ϊ'A','B','C'....��״̬
int Find_Index_Of_Answer(int target) {
	for (int i = 0; i < ans.size(); i++) {
		if (ans[i] == target)return i;
	}
	return -1;
}
//������״̬T�����ִܵ������״̬����
void Create_Closure() {
	fstream In(".\\nfa.txt");
	string str = "";
	cout << "���ļ��ж�ȡNFA(&����մ�):" << endl;
	while (In >> str) {
		int begin = Turn_Number(str);
		In >> str;
		char middle = str[0];
		In >> str;
		int end = Turn_Number(str);
		maps[begin][end].insert(middle);
		F = min(begin, F);
		E = max(end, E);
		if (middle != '&')sets.insert(middle);
		cout << begin << " " << middle << " " << end << endl;
	}
	cout << "��ʼ״̬:";
	cin >> Begin;
	cout << "����״̬:";
	cin >> End;
}
//��ʼ������
void init() {
	Create_Closure();
	for (int i = F; i <= E; i++) {
		stack<int>S;
		S.push(i);
		int ans = 1 << i;
		while (!S.empty()) {
			int out = S.top();
			S.pop();
			for (int j = F; j <= E; j++) {
				if (maps[out][j].find('&') != maps[out][j].end()) {
					S.push(j);
					ans |= 1 << j;
				}
			}
		}
		��_closure[i] = ans;
	}
}
//������ת��Ϊ�ַ���
string Turn_String(int number) {
	if (number == 0)return "0 ";
	string ans = "";
	while (number) {
		ans += (number % 10) + '0';
		number /= 10;
	}
	return string(ans.rbegin(), ans.rend()) + " ";
}
//��������Ϣ������ת��Ϊ�����ƶ�Ӧ��λ��
string Binary_Turn_String(int num) {
	string result = "";
	int k = 0;
	while (num) {
		if (num % 2 != 0) result += Turn_String(k) + " ";
		++k;
		num = num >> 1;
	}
	return result;
}
//��T״̬����a����ת��Ϊһ����״̬
int Move_T_A(int T, char a) {
	int ans = 0;
	for (int i = F; i <= E; i++) {
		//i�Ƿ���T״̬������
		if ((T | 1 << i) == T) {
			for (int j = F; j <= E; j++) {
				//�Ƿ��о���a����·������a��������ת����
				if (maps[i][j].find(a) != maps[i][j].end())ans |= ��_closure.find(j)->second;
			}
		}
		else continue;
	}
	return ans;
}
//��ʼ��DFA
void Init_DFA() {
	set<char>temp_data_terminal;
	set<char>N;
	for (int i = 0; i < ans.size(); i++) {
		//״̬�а�����̬
		if ((ans[i] | 1 << End) == ans[i]) {
			temp_data_terminal.insert('A' + i);
		}
		//��������̬
		else N.insert('A' + i);
	}
	terminal.push_back(temp_data_terminal);
	non_terminal.push(N);
}
//�õ�����
pair<bool, set<char>> Is_Same(set<char> set1, set<char> set2) {
	set<char>set3;
	set_intersection(set1.begin(), set1.end(), set2.begin(), set2.end(), inserter(set3, set3.begin()));
	if (set3.size() != 0)return pair<bool, set<char>>(true, set3);
	return pair<bool, set<char>>(false, set3);
}
//���Ҽ����Ƿ��ܹ��ָ�
pair<bool, vector<set<char>>> check_s_split(set<char>s, char a) {
	set<char>newStatus;
	vector<set<char>>K;
	int flag = 0;
	for (char c : s) {
		newStatus.insert(table[c][a]);
	}
	for (set<char> c : terminal) {
		pair<bool, set<char>> ans = Is_Same(newStatus, c);
		if (ans.first) {
			flag++;
			K.push_back(ans.second);
		}
	}
	pair<bool, set<char>> ans = Is_Same(newStatus, s);
	if (ans.first) {
		flag++;
		K.push_back(ans.second);
	}
	return pair<bool, vector<set<char>>>((flag > 1 ? true : false), K);
}
//���Ϸָ�
vector<set<char>> split(set<char> s) {
	vector<set<char>>new_status;
	for (char c : sets) {
		new_status.clear();
		pair<bool, vector<set<char>>> ans = check_s_split(s, c);
		new_status.resize(ans.second.size());
		for (char t : s) {
			char target = table[t][c];
			for (int i = 0; i < ans.second.size(); i++)
				if (ans.second[i].find(target) != ans.second[i].end())
					new_status[i].insert(t);
		}
		if (ans.first)return new_status;
	}
	return new_status;
}
//����DFA
void Simplify_DFA() {
	Init_DFA();
	while (!non_terminal.empty()) {
		set<char> s = non_terminal.front();
		non_terminal.pop();
		vector<set<char>>ans = split(s);
		if (ans.size() > 1) {
			for (int i = 0; i < ans.size(); i++)
				non_terminal.push(ans[i]);
		}
		else if (ans.size() == 0 && ans[0].size() == 0)non_terminal.push(s);
		else terminal.push_back(ans[0]);
	}
	for (int i = 0; i < terminal.size(); i++) {
		char min_char = 127;
		for (char c : terminal[i])min_char = min(min_char, c);
		for (char c : terminal[i]) replace_table[c] = min_char;

	}
}
//���ֲ���
int Search(int number) {
	int index = 15;
	while (number) {
		if ((1 << index) > number) {
			index--;
		}
		else if ((1 << index) < number) {
			index++;
		}
		else return index;
	}
}
//ȥ������״̬
void Remove_Useless_Status()
{
	while (true) {
		int Base_Status = ��_closure.find(Begin)->second ^ (1 << Begin);
		if ((Base_Status & (Base_Status - 1)) == 0 && Base_Status != 0) {
			��_closure.erase(Begin);
			Begin = Search(Base_Status);
		}
		else break;
	}
}
void Print_Table() {
	cout << "DFA��״̬ת����:" << endl;
	if (replace_table.size() == 0 || terminal.size() == ans.size()) {
		cout << "��ǰ���Ѿ������" << endl;
		cout << '\t';
		for (auto it = sets.begin(); it != sets.end(); it++) {
			cout << *it << '\t';
		}
		cout << endl;
		for (int i = 0; i < ans.size(); i++) {
			cout << (char)(i + 'A') << '\t';
			for (auto it = sets.begin(); it != sets.end(); it++) {
				cout << table[i + 'A'][*it] << '\t';
			}
			cout << endl;
		}
	}
	else
	{
		cout << "�����ǻ����Ľ��:" << endl;
		cout << '\t';
		for (auto it = sets.begin(); it != sets.end(); it++) {
			cout << *it << '\t';
		}
		cout << endl;
		for (int i = 0; i < ans.size(); i++) {
			if (replace_table[(i + 'A')] == i + 'A')cout << (char)(i + 'A') << '\t';
			else continue;
			for (auto it = sets.begin(); it != sets.end(); it++) {
				cout << replace_table[table[i + 'A'][*it]] << '\t';
			}
			cout << endl;
		}
	}
}
void Print_Status() {
	for (int i = 0; i < ans.size(); i++) {
		if (ans[i] == 0)cout << (char)('A' + i) << " = �ռ�" << endl;
		else cout << (char)('A' + i) << " = " << Binary_Turn_String(ans[i]) << endl;
	}
}
int main() {
	init();
	stack<int> states;
	int Base_Status = ��_closure.find(Begin)->second;
	states.push(Base_Status);
	ans.push_back(Base_Status);
	Repetition.insert(Base_Status);
	//�Ӽ����취
	while (!states.empty()) {
		int temp = states.top();
		states.pop();
		for (auto it = sets.begin(); it != sets.end(); it++) {
			int New_State = Move_T_A(temp, *it);
			if (Repetition.find(New_State) == Repetition.end() && New_State != 0) {
				states.push(New_State);
				Repetition.insert(New_State);
				ans.push_back(New_State);
			}
			if (New_State != 0)table[Find_Index_Of_Answer(temp) + 'A'][*it] = Find_Index_Of_Answer(New_State) + 'A';
		}
	}
	//DFA����
	Simplify_DFA();
	Print_Status();
	Print_Table();
	return 0;
}