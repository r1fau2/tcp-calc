#include <stack>

#include "chat.hpp"

//using namespace std;

int Priority(char ch)
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

bool ChatSession::Calc(const char *opt, char *wmsg)
{
	std::stack<char> s_opt;
    std::stack<int> s_num;
    int i = 0, tmp = 0;

    while(opt[i] != '\0' || s_opt.empty() != true) {
		if (opt[i] == ' ' || opt[i] == '\t') {				// ignor spase symbol 
			i++;
			continue;
		}	
		else if(opt[i] >= '0' && opt[i] <= '9') {			// find a number
			tmp = tmp * 10 + opt[i] - '0';
			i++;
			if(opt[i] > '9' || opt[i] < '0') {
				s_num.push(tmp);
				tmp = 0;
			}    
		}
		else {												// find a operator
			if(opt[i] == ')' && s_opt.top() == '(') {		// inside the brackets it is calculated
				s_opt.pop();
				i++;
				continue;
            }
			
			else if(s_opt.empty() == true					||	// no ready operator
			Priority(opt[i]) > Priority(s_opt.top())	||	// higher priority operator
			opt[i] != ')' && s_opt.top() == '(') {			// the first operator after the left bracket
                s_opt.push( opt[i]);
                i++;
                continue;
            }
			else if(opt[i] == '\0' && s_opt.empty() != true	||	// end of the input and there is a ready operator
			opt[i] == ')' && s_opt.top() != '(' 		||	// right bracket and there is a ready operator
			Priority(opt[i]) <= Priority(s_opt.top())) {	// lower priority operator	
                
                char ch = s_opt.top();
                s_opt.pop();
                int r_num = s_num.top();
				s_num.pop();
				int l_num = s_num.top();
				s_num.pop();
                
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
					return false;	 //error
                }
            }    
        }
    }     
	if (s_num.empty() == false) {
		result = s_num.top();
		calc_success = true;
		balance--;
		return true;
	} 
	else {
		calc_success = false;
		return false;		
	}
}
