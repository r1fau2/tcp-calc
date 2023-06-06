#include <stack>

#include "chat.hpp"

using namespace std;

int ChatSession::Priority(char ch)
{
    switch(ch) {
        case '(':
            return 3;
        case '*':
        case '/':
            return 2;
        case '+':
        case '-':
            return 1;
        default:
            return 0;
    }
}

void ChatSession::Calc(const char *opt)
{
	stack<char> s_opt;
    stack<int> s_num;
    int i = 0, tmp = 0, num1 = 0, num2 = 0;

    while(opt[i] != '\0' || s_opt.empty() != true) {
    if(opt[i] >= '0' && opt[i] <= '9') {
        tmp = tmp * 10 + opt[i] - '0';
        i++;
        if(opt[i] > '9' || opt[i] < '0') {
            s_num.push(tmp);
            tmp = 0;
        }    
    }
	else {
        if((opt[i] == ')') && (s_opt.top() == '(') ) {
                s_opt.pop();
                i++;
                continue;
            }
		if((s_opt.empty()) == true || (Priority(opt[i]) > Priority(s_opt.top() )) ||
              (s_opt.top() == '(' && (opt[i] != ')'))) {
                s_opt.push( opt[i]);
                i++;
                continue;
            }
		if(((opt[i] == '\0') &&( s_opt.empty() != true)) || ( (opt[i] == ')') && (s_opt.top() != '(') )||
                (Priority(opt[i]) <= Priority(s_opt.top() ))) {
                char ch = s_opt.top();
                s_opt.pop();
                int num1,num2;
                switch(ch) {
                case'+':
					num1 = s_num.top();
					s_num.pop();
					num2 = s_num.top();
					s_num.pop();
					s_num.push(num1 + num2);
					break;
                case'-':
					num2 = s_num.top();
					s_num.pop();
					num1 = s_num.top();
					s_num.pop();
					s_num.push(num1 - num2);
					break;    
                case'*':
					num1 = s_num.top();
					s_num.pop();
					num2 = s_num.top();
					s_num.pop();
					s_num.push(num1 * num2);
					break;    
                case'/':
					num2 = s_num.top();
					s_num.pop();
					num1 = s_num.top();
					s_num.pop();
					s_num.push(num1 / num2);
					break;   
                }
            }    
        }
    }
	ChatSession::result = s_num.top();
}
