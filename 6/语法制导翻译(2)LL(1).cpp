#include <iostream>
#include <string>
using namespace std;

int E(string token, int& i);
int T(string token, int& i);
int F(string token, int& i);

int main() {
    while (1) {
        cout << "请输入表达式: ";
        string expression;
        cin >> expression;
        if (expression == "exit")break;
        expression += "$"; // 添加结束符$

        int i = 0;
        int result = E(expression, i);
        if (result != INT_MIN && expression[i] == '$') {
            cout << "Result: " << result << endl;
        }
        else {
            cout << "非法表达式." << endl;
        }
    }

    return 0;
}

int E(string token, int& i) {
    int left = T(token, i);
    while (token[i] == '+' || token[i] == '-') {
        char op = token[i];
        i++;
        int right = T(token, i);
        if (op == '+') {
            left += right;
        }
        else {
            left -= right;
        }
    }
    return left;
}

int T(string token, int& i) {
    int left = F(token, i);
    while (token[i] == '*' || token[i] == '/') {
        char op = token[i];
        i++;
        int right = F(token, i);
        if (op == '*') {
            left *= right;
        }
        else {
            left /= right;
        }
    }
    return left;
}

int F(string token, int& i) {
    if (token[i] >= '0' && token[i] <= '9') {
        int value = 0;
        while (token[i] >= '0' && token[i] <= '9') {
            value = value * 10 + (token[i] - '0');
            i++;
        }
        return value;
    }
    else if (token[i] == '(') {
        i++;
        int result = E(token, i);
        if (token[i] == ')') {
            i++;
        }
        return result;
    }
    return INT_MIN;  // 处理错误情况
}
