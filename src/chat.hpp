#ifndef CHAT_HPP_SENTRY
#define CHAT_HPP_SENTRY

#include "sockets.hpp"

enum {
    max_line_length = 1023,
    qlen_for_listen = 16
};

enum fsm_states {fsm_in, fsm_pasw, fsm_work};

class ChatServer;

class ChatSession : FdHandler {
    friend class ChatServer;

    char buffer[max_line_length+1];
    int buf_used;
    bool ignoring;
    
	enum fsm_states state;
    int result;

    ChatServer *the_master;

    ChatSession(ChatServer *a_master, int fd);

    void Send(const char *msg);

    virtual void Handle(bool r, bool w);

    void ReadAndIgnore();
    void ReadAndCheck();
    void CheckLines();
        
    bool Login(const char *str);
    bool Passwd(const char *str);
    void StateStep(const char *str);
        
    bool Balance();
    int Priority(char ch);
    void Calc(const char *opt);
};

class ChatServer : public FdHandler {
    EventSelector *the_selector;
    struct item {
        ChatSession *s;
        item *next;
    };
    item *first;

    ChatServer(EventSelector *sel, int fd);
public:
    ~ChatServer();

    static ChatServer *Start(EventSelector *sel, int port);

    void RemoveSession(ChatSession *s);
    void SendAll(const char *msg, ChatSession *except = 0);

private:
    virtual void Handle(bool r, bool w);
};

#endif
