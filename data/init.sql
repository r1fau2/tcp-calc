CREATE TABLE USERS(ID INT PRIMARY KEY NOT NULL, LOGIN TEXT, PASSWD TEXT, BALANCE INT );
INSERT INTO USERS (ID, LOGIN, PASSWD, BALANCE)
	VALUES (1, 'bob', 'pswd-b', 50 );
INSERT INTO USERS (ID, LOGIN, PASSWD, BALANCE)
	VALUES (2, 'alice', 'pswd-a', 50 );
