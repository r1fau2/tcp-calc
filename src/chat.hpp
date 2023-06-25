#ifndef CHAT_HPP_SENTRY
#define CHAT_HPP_SENTRY
#include <sqlite3.h>

#include "sockets.hpp"

enum {
    max_in_line_length = 256,
    max_out_line_length = 2 * max_in_line_length + 64,
    qlen_for_listen = 16
};

enum fsm_states {fsm_in, fsm_pasw, fsm_work};

class ChatServer;

class ChatSession : FdHandler {
    friend class ChatServer;

    char buffer[max_in_line_length+1];
    int buf_used;
    bool ignoring;
    
    sqlite3 *db;
    char *name;
    int balance;
    bool calc_success;
    double result;

	enum fsm_states state;
    
    ChatServer *the_master;

    ChatSession(ChatServer *a_master, int fd);
    ~ChatSession();

    void Send(const char *msg);

    virtual void Handle(bool r, bool w);

    void ReadAndIgnore();
    void ReadAndCheck();
    void CheckLines();
        
    bool Authent(const char *str);
    bool CheckBalance();
    bool FixBalance();
    void Logged(const char *str);

    bool Calc(const char *opt, char *strout);

    void StateStep(const char *str);
};

class ChatServer : public FdHandler {
    EventSelector *the_selector;
    struct item {
        ChatSession *s;
        item *next;
    };
    item *first;

    ChatServer(EventSelector *sel, int fd, const char *dbpt, const char *lgpt);
public:
    const char *dbpath;
    const char *logpath;
    
    ~ChatServer();

    static ChatServer *Start(EventSelector *sel, int port, const char *dbpt, const char *lgpt);
    int InitDB(const char *sqlpt);
    void RemoveSession(ChatSession *s);

private:
    virtual void Handle(bool r, bool w);
};

#endif
