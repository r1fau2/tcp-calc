#include <iostream>
#include <fstream>
#include <string>
#include <sqlite3.h>
#include <chrono>

#include "chat.hpp"

using namespace std;

static int callback(void *pString, int argc, char **argv, char **azColName){
    if (argc>0) {
        string* str = static_cast<string*>(pString);
        str->assign(argv[0]);
    }
    return 0;
}

int ChatServer::InitDB(const char *sqlpt)
{
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	string outStr, sql;

	rc = sqlite3_open(dbpath, &db);
	if( rc != SQLITE_OK ) {
		cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
		return 1;
	} 
		
	sql = string("SELECT count(*) FROM sqlite_master ") +
		"WHERE type = 'table' AND name = 'USERS';";
	
	rc = sqlite3_exec(db, sql.c_str(), callback, &outStr, &zErrMsg);
	if( rc != SQLITE_OK ) {
		cerr << "Error in SELECT function." << zErrMsg <<  endl;
		sqlite3_free(zErrMsg);
		return 1;
	} 
	
	if (stoi(outStr) == 0) {
			
		ifstream in(sqlpt, ios::in);
		if (!in) {
			cerr << "Error file: " << sqlpt << endl;
			exit(1);
		}

		sql = "";	
		string line;
		while (getline(in, line))
		{
			sql += line;
		}	
		
		rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
		if( rc != SQLITE_OK ){
			cerr << "SQL error: " << zErrMsg << endl;
			sqlite3_free(zErrMsg);
			return 1;
		} 
		else
			cout << "Table and records were created successfully" << endl;
	}
	sqlite3_close(db);
	return 0;
}

//////////////////////////////////////////////////////////////////////

bool ChatSession::Authent(const char *str)
{
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	string outStr, sql;

	rc = sqlite3_open(the_master->dbpath, &db);
	if( rc != SQLITE_OK ) {
		cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
		return false;
	} 	
	
	sql = string("SELECT count(*) FROM USERS WHERE LOGIN = '") +
		name + "' AND PASSWD = '" + str + "';";
		
	rc = sqlite3_exec(db, sql.c_str(), callback, &outStr, &zErrMsg);
	if (rc != SQLITE_OK) {
		cerr << "Error in SELECT function." << zErrMsg << endl;
		sqlite3_free(zErrMsg);
		return false;
	}
	
	if (stoi(outStr) == 0) {
		cout << "Login incorrect\n" << endl;
		return false;
	}	
	
	sqlite3_close(db);
	return true;
}
        
bool ChatSession::Balance()
{
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	string outStr, sql;

	rc = sqlite3_open(the_master->dbpath, &db);
	if( rc != SQLITE_OK ) {
		cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
		return false;
	} 	
	
	sql = string("BEGIN EXCLUSIVE;") +  
		"SELECT BALANCE FROM USERS WHERE LOGIN = '" +
		name + "';";
	
	rc = sqlite3_exec(db, sql.c_str(), callback, &outStr, &zErrMsg);
	if (rc != SQLITE_OK) {
		cerr << "Error in SELECT function." << zErrMsg << endl;
		sqlite3_free(zErrMsg);
		return false;
	}
	
	if ((balance = stoi(outStr)) == 0) {
		cout << name << "'s balance is spent" << endl;
		sql = "COMMIT;";
		
		rc = sqlite3_exec(db, sql.c_str(), callback, &outStr, &zErrMsg);
		if (rc != SQLITE_OK) {
			cerr << "SQL error: " << zErrMsg << endl;
			sqlite3_free(zErrMsg);
			return false;
		}
		return false;
	}
	cout << name << "'s balance = " << balance << endl;
	balance--;
		sql = "UPDATE USERS SET BALANCE = " +
		to_string(balance) +
		" WHERE LOGIN = '" +
		name + "';" +
		"COMMIT ;";
		
	rc = sqlite3_exec(db, sql.c_str(), callback, &outStr, &zErrMsg);
	if (rc != SQLITE_OK) {
		cerr << "SQL error: " << zErrMsg << endl;
		sqlite3_free(zErrMsg);
		return false;
	}
	
	sqlite3_close(db);
	return true;
}
    
void ChatSession::Logged(const char *str)
{
	ofstream out(the_master->logpath, ios::out | ios::app);
	if (!out) {
		cerr << "Error file: " << the_master->logpath << endl;
		exit(1);
	}
	
	auto now = chrono::system_clock::now();
    time_t end_time = chrono::system_clock::to_time_t(now);
	char *t = ctime(&end_time);
	if (t[strlen(t)-1] == '\n')
		t[strlen(t)-1] = '\0';
	
	out << t << setw(10) << name << "\t(balance = " << balance
		<< ")\t" << str << " = " << result << endl;
	return;
}
