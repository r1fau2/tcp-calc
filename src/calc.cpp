#include <stdio.h>  // for sprintf
#include <stack>

#include "chat.hpp"

//using namespace std;

int ValidAndPrior(char ch)
{
    if (ch >= '0' && ch <= '9')
		return 1;
    switch(ch) {
        case '(':
            return 3;
        case '*': case '/':
            return 2;
        case '+': case '-': case ')':
        case '\t': case ' ': case '.':
        case '\0':
			return 1;
		default:
			return 0;
	}
}

bool ChatSession::Calc(const char *opt, char *wmsg)
{
	std::stack<char> s_opt;
    std::stack<int> s_num;
    int i = 0, tmp = 0;
    double l_num, r_num;

	while((opt[i] != '\0' || !s_opt.empty()) && ValidAndPrior(opt[i])) {
		if (opt[i] == ' ' || opt[i] == '\t')				// ignor spase symbol 
			i++;
		else if(opt[i] >= '0' && opt[i] <= '9') {			// find a number
			tmp = tmp * 10 + opt[i] - '0';
			i++;
			if(opt[i] > '9' || opt[i] < '0') {
				s_num.push(tmp);
				tmp = 0;
			}    
		}
		else {												// find a operator	
			// nothing do (parse stage)
			if (opt[i] == ')' && s_opt.empty()) {
					sprintf(wmsg, "unpaired ')' in %d-th position\ninput: <expr> or logout\n", i);
					calc_success = false;
					return false;
			}
			if(opt[i] == ')' && !s_opt.empty() && s_opt.top() == '(') {// inside the brackets it is calculated
				s_opt.pop();
				i++;
            }	
			// push operator (parse stage)
			else if(s_opt.empty()								||	// no ready operator or firsf operator
			// next s_opt-stack is not empty 						// and for safe execution s_opt.top()
			ValidAndPrior(opt[i]) > ValidAndPrior(s_opt.top())	||	// previous lower priority operator
			opt[i] != ')' && s_opt.top() == '(') {					// the first operator after the left bracket
				s_opt.push(opt[i]);
                i++;
            }
			// calculation
			else if(opt[i] == '\0'								||	// end of the input and there is a ready operator
			opt[i] == ')' && s_opt.top() != '(' 				||	// right bracket and there is a ready operator
			ValidAndPrior(opt[i]) <= ValidAndPrior(s_opt.top())) {	// previous higher priority operator	
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
    if (!ValidAndPrior(opt[i])) {
		sprintf(wmsg, "symbol '%c' in %d-th position is not allowed\ninput: <expr> or logout\n", opt[i], i);
		calc_success = false;
		return false;
	}
    if (!s_num.empty()) {
		result = s_num.top();
		calc_success = true;
		balance--;
		return true;
	} 
	else {
		calc_success = false;
		return false;			// empty input  
	}
}
