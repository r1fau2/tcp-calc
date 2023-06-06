#include <stdio.h>
#include "chat.hpp"

static int port = 7777;
const char *dbpath = "../data/data.db";
const char *logpath = "../data/tcp-calc.log";
const char *sqlpath = "../data/init.sql";

int main()
{
    EventSelector *selector = new EventSelector;
    ChatServer *serv = ChatServer::Start(selector, port, dbpath, logpath);
    if(!serv) {				// serv == 0 - error
        perror("server");
        return 1;
    }
    int res = serv->InitDB(sqlpath);
    if(res) {				// res != 0 - error
        perror("database");
        return 1;
    }
    selector->Run();
    return 0;
}
