// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <openenclave/enclave.h>
#include "demo_t.h"

#include <stdio.h>
#include <string.h>

#include <sqlite3.h>

// https://www.tutorialspoint.com/sqlite/sqlite_c_cpp.htm

static int callback(void* NotUsed, int argc, char** argv, char** azColName)
{
    OE_UNUSED(NotUsed);
    int i;
    for (i = 0; i < argc; i++)
    {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

int enc_main(int argc, const char** argv)
{
    OE_UNUSED(argc);
    OE_UNUSED(argv);
    
    sqlite3* db = NULL;
    char* zErrMsg = 0;
    int rc;
    const char* sql;

    rc = sqlite3_open(":memory:", &db);
    if (rc)
    {
	printf("Can't open database: %s\n", sqlite3_errmsg(db));
	return -1;
    }
    printf("sqlite3: in-memory database opened\n");

    /* Create SQL statement */
    sql = "CREATE TABLE COMPANY("
	"ID INT PRIMARY KEY     NOT NULL,"
	"NAME           TEXT    NOT NULL,"
	"AGE            INT     NOT NULL,"
	"ADDRESS        CHAR(50),"
	"SALARY         REAL );";

    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

    if (rc != SQLITE_OK)
    {
	fprintf(stderr, "SQL error: %s\n", zErrMsg);
	sqlite3_free(zErrMsg);
    }
    else
    {
	fprintf(stdout, "Table created successfully\n");
    }

    /* Create SQL statement */
    sql = "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) "
	"VALUES (1, 'Paul', 32, 'California', 20000.00 ); "
	"INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) "
	"VALUES (2, 'Allen', 25, 'Texas', 15000.00 ); "
	"INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)"
	"VALUES (3, 'Teddy', 23, 'Norway', 20000.00 );"
	"INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)"
	"VALUES (4, 'Mark', 25, 'Rich-Mond ', 65000.00 );";

    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);

    if (rc != SQLITE_OK)
    {
	fprintf(stderr, "SQL error: %s\n", zErrMsg);
	sqlite3_free(zErrMsg);
    }
    else
    {
	fprintf(stdout, "Records created successfully\n");
    }

    /* Create SQL statement */
    sql = "SELECT * from COMPANY";

    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql, callback, NULL, &zErrMsg);

    if (rc != SQLITE_OK)
    {
	fprintf(stderr, "SQL error: %s\n", zErrMsg);
	sqlite3_free(zErrMsg);
    }
    else
    {
	fprintf(stdout, "Operation done successfully\n");
    }
    rc = sqlite3_close(db);
    if (rc)
    {
	printf("Can't close database: $s\n", sqlite3_errmsg(db));
	return -1;
    }
    printf("sqlite3: database closed.\n");

    return 0;
}

OE_SET_ENCLAVE_SGX(
    1,    /* ProductID */
    1,    /* SecurityVersion */
    true, /* Debug */
    1024, /* NumHeapPages */
    1024, /* NumStackPages */
    2);   /* NumTCS */
