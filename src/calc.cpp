#include <stdio.h>  // for sprintf
// #include <iostream>
#include <regex>
#include <stack>

#include "chat.hpp"

using namespace std;

bool CheckValid(const char *opt, char *wmsg)
{
	string s = opt;
	string patern(R"([^\s\d\.\(\)\*/\+\-])");		// invalid characters,
	patern = patern						+ "|" +
		R"(\D\.\D)"						+ "|" +		// a free-standing dot,
		R"(^\.\D|\D\.$|^\.$)" 			+ "|" +		// start and end dot,
		R"((\)|\.|\d)\s+(\(|\.|\d))" 	+ "|" +		// a space between numbers,
		R"((\+|\-)\s*(\*|/))";						// two operators in a row
	regex r(patern);
	smatch m;
	if (regex_search(s, m, r)) {
		char *str = new char[m.position(0) + 1];
		int i;
		for (i = 0; i < m.position(0); i++)
			str[i] = ' ';
		str[i] = '^';
		str[i+1] = '\0';
		sprintf(wmsg, "\nerror:1:%ld: invalid syntax: \n%s\n%s\ninput: <expr> or logout\n", m.position(0), opt, str);
		delete[] str;
		return false;
	}
	return true;
}

int Priority(char ch)
{
    switch(ch) {
        case '(':
            return 3;
        case '*': case '/':
            return 2;
        case '+': case '-':
        case ')': case '\0':
			return 1;
		default:
			return 0;
	}
}

double GetDouble(const char *opt, int &i)
{
    int start = i;
    if (opt[i] == '.')
		i++;
    while (opt[i] != '\0' && isdigit(opt[i]))
		i++;
	if (opt[i] == '.' && opt[start] != '.') {
		i++;
		while (opt[i] != '\0' && isdigit(opt[i]))
			i++;
	}
	double num;
	sscanf(opt+start, "%lf", &num);
	return num;
}

bool Sign(const char *opt, int &i, int &sign)
{
	if (i > 0 && (isdigit(opt[i-1]) 					||
		(opt[i-1] == '.' && i > 1 && isdigit(opt[i-2]))	||
		opt[i-1] == ')'))
		return false;
	if (opt[i] == '-')
		sign *= -1;
	i++;
	return true;
}

bool ChatSession::Calc(const char *opt, char *wmsg)
{
	stack<char> s_opt;
	stack<int> s_sgn;
	stack<double> s_num;
    int i = 0, sign = 1;
    double l_num, r_num;

	if (!CheckValid(opt, wmsg)) {
		calc_success = false;
		return false;
	}
    string s = opt;
    regex r(R"(\s+)");
    s = regex_replace(s, r, "");					// remove the space characters
    opt = s.c_str();

    while(opt[i] != '\0' || !s_opt.empty()) {

		if((opt[i] == '+' || opt[i] == '-') && Sign(opt, i, sign))	// find the operand sign
			continue;

		if(isdigit(opt[i]) || opt[i] == '.') {		// find the operand
			s_num.push(sign * GetDouble(opt, i));
			sign = 1;
		}
		else {										// find the operator
			// nothing do (parse stage)
			if (opt[i] == ')' && s_opt.empty()) {
				sprintf(wmsg, "unpaired ')' in %d-th position\ninput: <expr> or logout\n", i);
				calc_success = false;
				return false;
			}
			if(opt[i] == ')' && !s_opt.empty() && s_opt.top() == '(') {// inside the brackets it is calculated
				s_opt.pop();
				if (s_num.empty()) {
					sprintf(wmsg, "\nerror: empty brackets\ninput: <expr> or logout\n");
					calc_success = false;
					return false;
				}
				else { 
					double tmp = s_num.top();
					s_num.pop();  
					s_num.push(s_sgn.top() * tmp);
					s_sgn.pop();
				} 
				i++;
            }	
			// push operator (parse stage)
			else if(s_opt.empty()								||	// no ready operator or firsf operator
			// next s_opt-stack is not empty 						// and for safe execution s_opt.top()
			Priority(opt[i]) > Priority(s_opt.top())			||	// previous lower priority operator
			opt[i] != ')' && s_opt.top() == '(') {					// the first operator after the left bracket
				s_opt.push(opt[i]);
				if (opt[i] == '(') {
					s_sgn.push(sign);
					sign = 1;
				} 
                i++;
            }
			// calculation
			else if(opt[i] == '\0'								||	// end of the input and there is a ready operator
			opt[i] == ')' && s_opt.top() != '(' 				||	// right bracket and there is a ready operator
			Priority(opt[i]) <= Priority(s_opt.top())) {			// previous higher priority operator	
               char ch = s_opt.top();
				if (ch == '(') {
					sprintf(wmsg, "unpaired '('\ninput: <expr> or logout\n");
					calc_success = false;
					return false;
				}
				s_opt.pop();
				if (!s_num.empty()) {
					r_num = s_num.top();
					s_num.pop();
				}
				else {
					sprintf(wmsg, "operand is missing\ninput: <expr> or logout\n");
					calc_success = false;
					return false;
				}	
				if (!s_num.empty()) {
					l_num = s_num.top();
					s_num.pop();
				}
				else {
					sprintf(wmsg, "operand is missing\ninput: <expr> or logout\n");
					calc_success = false;
					return false;
				}
                switch(ch) {
                case'+':
					s_num.push(l_num + r_num);
					break;
                case'-':
					s_num.push(l_num - r_num);
					break;    
                case'*':
					s_num.push(l_num * r_num);
					break;    
                case'/':
					if (r_num == 0) {
						sprintf(wmsg, "division by zero\ninput: <expr> or logout\n");
						calc_success = false;
						return false;
					}
					s_num.push(l_num / r_num);
					break;
				default:
					sprintf(wmsg, "your expression is not correct\ninput: <expr> or logout\n");
					calc_success = false;
					return false;
                }
			}    
        }
	}
	if (!s_num.empty()) {
		result = s_num.top();
		calc_success = true;
		balance--;
		return true;
	} 
	else {
		sprintf(wmsg, "input: <expr> or logout\n");
		calc_success = false;
		return false;			// empty input  
	}
}
