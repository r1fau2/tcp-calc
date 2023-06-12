#include <stdio.h>  // for sprintf
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "chat.hpp"

ChatSession::ChatSession(ChatServer *a_master, int fd)
    : FdHandler(fd, true), buf_used(0), ignoring(false),
    the_master(a_master), state(fsm_in),
    db(0), name(0), balance(0), calc_success(false), result(0)
{
    Send("\nlogin: ");
}

ChatSession::~ChatSession()
{
    if(name)
        delete[] name;
}

void ChatSession::Send(const char *msg)
{
    write(GetFd(), msg, strlen(msg));
}

void ChatSession::Handle(bool r, bool w)
{
    if(!r)   // should never happen, we only request ready-to-read events
        return;
    if(buf_used >= (int)sizeof(buffer)) {
        buf_used = 0;
        ignoring = true;
    }
    if(ignoring)
        ReadAndIgnore(); // Send("The string is too long\n");
    else
        ReadAndCheck();
}

void ChatSession::ReadAndIgnore()
{
    int rc = read(GetFd(), buffer, sizeof(buffer));
    if(rc < 1) { 						// EOF if rc == 0
        the_master->RemoveSession(this);
        return;
    }
    int i;
    for(i = 0; i < rc; i++)
        if(buffer[i] == '\n') {   // stop ignoring!
            int rest = rc - i - 1;
            if(rest > 0)
                memmove(buffer, buffer + i + 1, rest);
            buf_used = rest;
            ignoring = 0;
            CheckLines();
        }
}

void ChatSession::ReadAndCheck()
{
    int rc = read(GetFd(), buffer+buf_used, sizeof(buffer)-buf_used);
    if(rc < 1) {				// EOF if rc == 0
        the_master->RemoveSession(this);
        return;
    }
    buf_used += rc;
    CheckLines();
}

void ChatSession::CheckLines()
{
    if(buf_used <= 0)
        return;
    int i;
    for(i = 0; i < buf_used; i++) {
        if(buffer[i] == '\n') {
            buffer[i] = 0;
            if(i > 0 && buffer[i-1] == '\r')
                buffer[i-1] = 0;
            StateStep(buffer);
            int rest = buf_used - i - 1;
            memmove(buffer, buffer + i + 1, rest);
            buf_used = rest;
            CheckLines();
            return;
        }
    }
}

void ChatSession::StateStep(const char *str)
{
	char *wmsg = new char[max_out_line_length];
	int len;
	switch(state) {
	case fsm_in:
		len = strlen(str);
        name = new char[len+1];
        strcpy(name, str);
		state = fsm_pasw;
		sprintf(wmsg, "\nРassword: ");
		break;
	case fsm_pasw:
		if (Authent(str)) {
			state = fsm_work;
			sprintf(wmsg, "input: <expr> or logout\n");
		}
		else {
			delete[] name;
			state = fsm_in;
			sprintf(wmsg, "\nLogin incorrect\nlogin: ");
		}
		break;
	case fsm_work:
		if (strstr(str, "logout") == str){
			state = fsm_in;
			sprintf(wmsg, "\nlogin: ");
		}
		else if (CheckBalance() && Calc(str, wmsg)) {	// lock db
			if (FixBalance())							// unlock db				
				Logged(str);
			sprintf(wmsg, "%lf\n%s", result, "input: <expr> or logout\n");
		}
		else {
			FixBalance();								// unlock db
			if (balance == 0)
				sprintf(wmsg, "your balance is spent\ninput: logout\n");
		}
	}
	Send(wmsg);
	delete[] wmsg;
}

//////////////////////////////////////////////////////////////////////

ChatServer::ChatServer(EventSelector *sel, int fd, const char *dbpt, const char *lgpt)
    : FdHandler(fd, true), the_selector(sel), first(0), dbpath(dbpt), logpath(lgpt)
{
    the_selector->Add(this);
}

ChatServer::~ChatServer()
{
    while(first) {
        item *tmp = first;
        first = first->next;
        the_selector->Remove(tmp->s);
        delete tmp->s;
        delete tmp;
    }
    the_selector->Remove(this);
}

ChatServer *ChatServer::Start(EventSelector *sel, int port, const char *dbpt, const char *lgpt)
{
    int ls, opt, res;
    struct sockaddr_in addr;

    ls = socket(AF_INET, SOCK_STREAM, 0);
    if(ls == -1)
        return 0;

    opt = 1;
    setsockopt(ls, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    res = bind(ls, (struct sockaddr*) &addr, sizeof(addr));
    if(res == -1)
        return 0;

    res = listen(ls, qlen_for_listen);
    if(res == -1)
        return 0;
    
    return new ChatServer(sel, ls, dbpt, lgpt);
}

void ChatServer::RemoveSession(ChatSession *s)
{
    the_selector->Remove(s);
    item **p;
    for(p = &first; *p; p = &((*p)->next)) {
        if((*p)->s == s) {
            item *tmp = *p;
            *p = tmp->next;
            delete tmp->s;
            delete tmp;
            return;
        }
    }
}

void ChatServer::SendAll(const char *msg, ChatSession *except)
{
    item *p;
    for(p = first; p; p = p->next)
        if(p->s != except)
            p->s->Send(msg);
}

void ChatServer::Handle(bool r, bool w)
{
    if(!r)       // must not happen, ever!
        return;

    int sd;
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    sd = accept(GetFd(), (struct sockaddr*) &addr, &len);
    if(sd == -1)
        return;

    item *p = new item;
    p->next = first;
    p->s = new ChatSession(this, sd);
    first = p;

    the_selector->Add(p->s);
}
